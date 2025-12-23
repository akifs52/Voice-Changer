#include "recorder.h"
#include <QFileDialog>
#include <QTimer>
#include <QElapsedTimer>
#include "ui_mainwindow.h"
#include "mainwindow.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}



AVFormatContext *formatContext = nullptr;
AVStream *audioStream = nullptr;
AVCodecContext *codecContext = nullptr;
AVFrame *frame = nullptr;
AVPacket *packet = nullptr;
SwrContext *swrCtx = nullptr; // Örnekleme dönüştürücü

int64_t pts = 0;

// Recording state variables
QTimer* recordingMixedAudioTimer = nullptr;
const AVCodec* recordingCodec = nullptr;

recorder::recorder(QWidget *parent)
    : QMainWindow{parent}
{

}

void MainWindow::on_startRecord_clicked()
{
    // Prevent multiple recordings
    if (isRecording) {
        qWarning() << "Recording already in progress!";
        return;
    }

    try {
        // Clear previous audio data
        data.clear();

        // Initialize circular buffer
        circularBuffer.clear();
        circularBuffer.resize(bufferSize * 2); // Stereo için 2 kanal
        readPos = 0;
        writePos = 0;
        availableSamples = 0;

        // Check if audio input is properly initialized
        if (!audioInput || !inputDevice) {
            qCritical() << "Audio input not initialized!";
            return;
        }

        // Get actual input format
        QAudioFormat inputFormat = audioInput->format();
        bool isInputMono = (inputFormat.channelCount() == 1);
        int inputSampleRate = inputFormat.sampleRate();

        qDebug() << "=== RECORDING START ===";
        qDebug() << "Input format - SampleRate:" << inputSampleRate
                 << "Channels:" << inputFormat.channelCount()
                 << "Mono:" << isInputMono
                 << "SampleFormat:" << inputFormat.sampleFormat();
        qDebug() << "Circular buffer size:" << circularBuffer.size()
                 << "samples (" << (circularBuffer.size() / 44100.0) << "seconds at 44.1kHz)";

        // Don't auto-enable test button - use Cable Input output instead
        testButtonWasActive = ui->testButton->isChecked();
        qDebug() << "Recording using Cable Input output without enabling test button";

        // Ensure audio input is active and resume if needed
        if (audioInput->state() == QAudio::SuspendedState || audioInput->state() == QAudio::StoppedState) {
            audioInput->resume();
            qDebug() << "Audio input resumed for recording. State:" << audioInput->state();
        }

        // Ensure virtual audio output is also active
        if (virtualAudioOutput && virtualAudioOutput->state() == QAudio::SuspendedState) {
            virtualAudioOutput->resume();
            qDebug() << "Virtual audio output resumed for recording";
        }

        // Default to MP3 (now supported with GPL FFmpeg)
        QString fileName = QFileDialog::getSaveFileName(this, "Save Audio File", "", "Audio Files (*.mp3 *.wav)");
        if (fileName.isEmpty()) {
            qCritical() << "No file selected.";
            if (!testButtonWasActive) {
                ui->testButton->setChecked(false);
            }
            return;
        }

        // Determine format and ensure proper extension
        bool useMP3 = false;
        if (fileName.endsWith(".mp3", Qt::CaseInsensitive)) {
            useMP3 = true;
        } else if (fileName.endsWith(".wav", Qt::CaseInsensitive)) {
            useMP3 = false;
        } else {
            // Default to MP3, add extension
            fileName += ".mp3";
            useMP3 = true;
        }

        QString formatName = useMP3 ? "mp3" : "wav";
        QString codecName = useMP3 ? "MP3" : "WAV";

        qDebug() << "Saving to file:" << fileName << "as" << formatName << "format";

        // Initialize FFmpeg
        avformat_network_init();

        if (avformat_alloc_output_context2(&formatContext, nullptr, formatName.toStdString().c_str(), fileName.toStdString().c_str()) < 0) {
            qCritical() << "Failed to allocate format context for" << formatName;
            return;
        }

        // Find appropriate codec - Use mp3_mf since libmp3lame is not available
        const AVCodec *codec = nullptr;
        
        if (useMP3) {
            qDebug() << "=== USING MP3_MF ENCODER ===";
            // Try mp3_mf first (Media Foundation - Windows built-in)
            codec = avcodec_find_encoder_by_name("mp3_mf");
            if (codec) {
                qDebug() << "SUCCESS: Using mp3_mf encoder:" << codec->name;
            } else {
                qWarning() << "FAILED: mp3_mf not available, falling back to WAV";
                useMP3 = false;
            }
        }
        
        if (!codec && useMP3) {
            // Fallback to generic MP3
            codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
            if (codec) {
                qDebug() << "SUCCESS: Using generic MP3 encoder:" << codec->name;
            } else {
                qWarning() << "FAILED: No MP3 encoder found, falling back to WAV";
                useMP3 = false;
            }
        }
        
        if (!useMP3) {
            // Use WAV as fallback
            codec = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);
            if (!codec) {
                qCritical() << "FAILED: Could not find even WAV encoder!";
                avformat_free_context(formatContext);
                return;
            }
            qDebug() << "SUCCESS: Using WAV encoder:" << codec->name;
            
            // Update format context for WAV
            avformat_free_context(formatContext);
            QString wavFileName = fileName;
            if (wavFileName.endsWith(".mp3", Qt::CaseInsensitive)) {
                wavFileName = wavFileName.left(wavFileName.length() - 4) + ".wav";
            }
            if (avformat_alloc_output_context2(&formatContext, nullptr, "wav", wavFileName.toStdString().c_str()) < 0) {
                qCritical() << "Failed to allocate WAV format context.";
                return;
            }
            fileName = wavFileName;
        }

        // Configure codec parameters
        codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            qCritical() << "Failed to allocate codec context.";
            return;
        }

        if (useMP3) {
            // MP3 specific settings - OPTIMIZED FOR mp3_mf
            if (QString(codec->name) == "mp3_mf") {
                qDebug() << "Using mp3_mf specific settings";
                // mp3_mf (Media Foundation) optimized settings
                codecContext->sample_fmt = AV_SAMPLE_FMT_FLT;  // Float works better with Media Foundation
                codecContext->bit_rate = 128000;
                codecContext->sample_rate = 48000;  // 48kHz standard for Media Foundation
                av_channel_layout_default(&codecContext->ch_layout, 2); // Stereo
                codecContext->frame_size = 0;  // Let mp3_mf decide frame size
                codecContext->strict_std_compliance = FF_COMPLIANCE_NORMAL;  // Standard compliance
                qDebug() << "mp3_mf configured with FLT format, 48kHz";
            } else if (QString(codec->name) == "libmp3lame") {
                // Standard libmp3lame settings
                codecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
                codecContext->bit_rate = 128000;
                codecContext->sample_rate = 44100;
                av_channel_layout_default(&codecContext->ch_layout, 2); // Stereo
                codecContext->frame_size = 1152;
                codecContext->compression_level = 5;
                av_opt_set_int(codecContext, "compression_level", 5, 0);
                qDebug() << "libmp3lame configured with FLTP format, 44.1kHz";
            } else {
                // Generic MP3 fallback settings
                codecContext->sample_fmt = AV_SAMPLE_FMT_S16;  // Most compatible
                codecContext->bit_rate = 128000;
                codecContext->sample_rate = 44100;
                av_channel_layout_default(&codecContext->ch_layout, 2); // Stereo
                codecContext->frame_size = 1152;
                qDebug() << "Generic MP3 configured with S16 format, 44.1kHz";
            }
        } else {
            // WAV specific settings
            codecContext->sample_fmt = AV_SAMPLE_FMT_S16;
            codecContext->sample_rate = 44100;
            av_channel_layout_default(&codecContext->ch_layout, 2); // Stereo
            // WAV doesn't need frame_size or compression settings
        }

        if (formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
            codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        qDebug() << "=== ATTEMPTING TO OPEN CODEC ===";
        qDebug() << "Codec name:" << codec->name;
        qDebug() << "Codec long name:" << (codec->long_name ? codec->long_name : "N/A");
        qDebug() << "Sample format:" << codecContext->sample_fmt;
        qDebug() << "Sample rate:" << codecContext->sample_rate;
        qDebug() << "Bit rate:" << codecContext->bit_rate;
        qDebug() << "Channels:" << codecContext->ch_layout.nb_channels;
        qDebug() << "Frame size:" << codecContext->frame_size;
        
        // Try opening codec with specific options for mp3_mf
        AVDictionary *opts = nullptr;
        if (useMP3 && QString(codec->name) == "mp3_mf") {
            // Media Foundation specific options - try minimal first
            qDebug() << "Setting mp3_mf options...";
            av_dict_set(&opts, "abr", "true", 0);  // Average bit rate
        }
        
        int openResult = avcodec_open2(codecContext, codec, &opts);
        if (opts) av_dict_free(&opts);
        
        qDebug() << "avcodec_open2 result:" << openResult;
        
        if (openResult < 0) {
            char errorBuf[256];
            av_strerror(openResult, errorBuf, sizeof(errorBuf));
            qCritical() << "Failed to open" << (useMP3 ? "MP3" : "WAV") << "codec.";
            qCritical() << "Error code:" << openResult << "-" << errorBuf;
            
            // For MP3 failure, try different approach before fallback
            if (useMP3) {
                qCritical() << "=== MP3 FAILED, TRYING ALTERNATIVE SETTINGS ===";
                
                // Try with different settings for mp3_mf
                if (QString(codec->name) == "mp3_mf") {
                    qDebug() << "Retrying mp3_mf with S16 format...";
                    
                    // Reset and try again with S16 format as fallback
                    avcodec_free_context(&codecContext);
                    codecContext = avcodec_alloc_context3(codec);
                    if (codecContext) {
                        codecContext->sample_fmt = AV_SAMPLE_FMT_S16;  // Try S16 as fallback
                        codecContext->bit_rate = 128000;
                        codecContext->sample_rate = 44100;  // Try 44.1kHz as fallback
                        av_channel_layout_default(&codecContext->ch_layout, 2);
                        codecContext->frame_size = 0;
                        codecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
                        
                        int retryResult = avcodec_open2(codecContext, codec, nullptr);
                        qDebug() << "mp3_mf retry result:" << retryResult;
                        
                        if (retryResult >= 0) {
                            qDebug() << "SUCCESS: mp3_mf opened with S16 fallback settings!";
                            goto codec_opened;  // Skip the fallback
                        }
                    }
                }
                
                qCritical() << "=== MP3 FAILED, FALLING BACK TO WAV ===";
                
                // Clean up current context
                avcodec_free_context(&codecContext);
                avformat_free_context(formatContext);
                
                // Switch to WAV
                useMP3 = false;
                QString wavFileName = fileName;
                if (wavFileName.endsWith(".mp3", Qt::CaseInsensitive)) {
                    wavFileName = wavFileName.left(wavFileName.length() - 4) + ".wav";
                }
                fileName = wavFileName;
                
                // Reinitialize format context for WAV
                if (avformat_alloc_output_context2(&formatContext, nullptr, "wav", fileName.toStdString().c_str()) < 0) {
                    qCritical() << "Failed to allocate WAV format context during fallback";
                    return;
                }
                
                // Get WAV codec
                codec = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);
                if (!codec) {
                    qCritical() << "Failed to find WAV codec during fallback";
                    return;
                }
                
                // Configure WAV codec
                codecContext = avcodec_alloc_context3(codec);
                if (!codecContext) {
                    qCritical() << "Failed to allocate WAV codec context during fallback";
                    return;
                }
                
                codecContext->sample_fmt = AV_SAMPLE_FMT_S16;
                codecContext->sample_rate = 44100;
                av_channel_layout_default(&codecContext->ch_layout, 2);
                
                // Try to open WAV codec
                int wavResult = avcodec_open2(codecContext, codec, nullptr);
                if (wavResult < 0) {
                    av_strerror(wavResult, errorBuf, sizeof(errorBuf));
                    qCritical() << "Even WAV codec failed:" << wavResult << "-" << errorBuf;
                    avcodec_free_context(&codecContext);
                    return;
                }
                
                qDebug() << "SUCCESS: Switched to WAV format:" << fileName;
                qDebug() << "WAV codec opened successfully";
            } else {
                avcodec_free_context(&codecContext);
                return;
            }
        }
        
codec_opened:
        qDebug() << "SUCCESS: Codec opened successfully!";

        audioStream = avformat_new_stream(formatContext, nullptr);
        if (!audioStream) {
            qCritical() << "Failed to create new stream.";
            avcodec_free_context(&codecContext);
            return;
        }

        if (avcodec_parameters_from_context(audioStream->codecpar, codecContext) < 0) {
            qCritical() << "Failed to copy codec parameters.";
            avcodec_free_context(&codecContext);
            return;
        }

        audioStream->time_base = {1, codecContext->sample_rate};

        qDebug() << "Codec configured - Frame size:" << codecContext->frame_size
                 << "Sample rate:" << codecContext->sample_rate
                 << "Channels:" << codecContext->ch_layout.nb_channels;

        // Open output file
        if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&formatContext->pb, fileName.toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
                qCritical() << "Failed to open output file.";
                avcodec_free_context(&codecContext);
                return;
            }
        }

        if (avformat_write_header(formatContext, nullptr) < 0) {
            qCritical() << "Failed to write header.";
            avcodec_free_context(&codecContext);
            return;
        }

        // Configure resampler only for MP3 (WAV doesn't need resampling)
        if (useMP3) {
            AVChannelLayout inputChannelLayoutStruct;
            AVChannelLayout outputChannelLayoutStruct;

            // Input channel layout
            if (isInputMono) {
                av_channel_layout_default(&inputChannelLayoutStruct, 1);
            } else {
                av_channel_layout_default(&inputChannelLayoutStruct, 2);
            }
            av_channel_layout_default(&outputChannelLayoutStruct, 2); // Stereo output

            // Determine input format
            AVSampleFormat ffmpegInputFormat = AV_SAMPLE_FMT_S16;
            if (inputFormat.sampleFormat() == QAudioFormat::Float) {
                ffmpegInputFormat = AV_SAMPLE_FMT_FLT;
            } else if (inputFormat.sampleFormat() == QAudioFormat::UInt8) {
                ffmpegInputFormat = AV_SAMPLE_FMT_U8;
            }

            // Choose target format based on codec
            AVSampleFormat targetFormat = AV_SAMPLE_FMT_FLTP;
            if (QString(codec->name) == "mp3_mf") {
                targetFormat = AV_SAMPLE_FMT_S16;  // mp3_mf uses S16
                qDebug() << "Using S16 format for mp3_mf resampling";
            }

            // Create and configure SwrContext
            swrCtx = swr_alloc();

            // Set resampler options with explicit settings
            if (swr_alloc_set_opts2(&swrCtx,
                                    &outputChannelLayoutStruct, targetFormat, codecContext->sample_rate,
                                    &inputChannelLayoutStruct, ffmpegInputFormat, inputSampleRate,
                                    0, nullptr) < 0) {
                qCritical() << "Failed to allocate SwrContext.";
                return;
            }

            // Set additional resampler options for better MP3 compatibility
            av_opt_set_int(swrCtx, "linear_interp", 1, 0);
            av_opt_set_double(swrCtx, "cutoff", 0.97, 0);
            av_opt_set_int(swrCtx, "async", 1, 0);

            if (swr_init(swrCtx) < 0) {
                qCritical() << "Failed to initialize SwrContext.";
                swr_free(&swrCtx);
                return;
            }

            // Calculate resampler delay
            int64_t swrDelay = swr_get_delay(swrCtx, codecContext->sample_rate);
            qDebug() << "SwrContext initialized. Target format:" << av_get_sample_fmt_name(targetFormat) << "Delay:" << swrDelay << "samples";
        } else {
            qDebug() << "WAV format - no resampling needed";
            swrCtx = nullptr;
        }

        // Allocate frame
        frame = av_frame_alloc();
        if (!frame) {
            qCritical() << "Failed to allocate frame.";
            return;
        }

        // mp3_mf doesn't require frame_size, use 1024 for it
        int frameSamples;
        if (codecContext->frame_size > 0) {
            frameSamples = codecContext->frame_size;
        } else {
            // For codecs like mp3_mf that don't specify frame_size
            if (QString(codec->name) == "mp3_mf") {
                frameSamples = 1024;  // Standard for Media Foundation
            } else {
                frameSamples = 1024;  // Default fallback
            }
        }
        
        frame->nb_samples = frameSamples;
        frame->format = codecContext->sample_fmt;
        av_channel_layout_default(&frame->ch_layout, codecContext->ch_layout.nb_channels);
        
        qDebug() << "Frame allocated - samples:" << frameSamples << "format:" << av_get_sample_fmt_name(codecContext->sample_fmt);

        if (av_frame_get_buffer(frame, 0) < 0) {
            qCritical() << "Failed to allocate audio frame buffer.";
            av_frame_free(&frame);
            return;
        }

        // Allocate packet
        packet = av_packet_alloc();
        if (!packet) {
            qCritical() << "Failed to allocate packet.";
            av_frame_free(&frame);
            return;
        }

        // Set up recording state
        isRecording = true;
        recordingTimer.start();
        pts = 0;

        qDebug() << "Recording state set to TRUE. Timer started.";
        qDebug() << "Using format:" << (useMP3 ? "MP3" : "WAV") << "with codec:" << codec->name;

        // Setup proper audio connection for recording with Cable Input AFTER all preparations
        if (inputDevice) {
            // Disconnect existing connections to avoid conflicts
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            // Create recording-specific connection that handles effects and Cable Input output
            connect(inputDevice, &QIODevice::readyRead, this, [=]() {
                data = inputDevice->readAll();
                progressBarOutput();
                
                // Apply effects if active
                if (!usingEffects) {
                    if (ui->robotButton->isChecked()) {
                        processToRobotVoice(data);
                    } else if (ui->bananaButton->isChecked()) {
                        processToBananaVoice(data);
                    } else if (ui->devilButton->isChecked()) {
                        processToDevilVoice(data);
                    } else if (ui->femaleButton->isChecked()) {
                        processToFemaleVoice(data);
                    } else if (ui->combineButton->isChecked()) {
                        processToCombineVoice(data);
                    } else if (ui->ekoButton->isChecked()) {
                        processToEkoVoice(data);
                    }
                }
                
                // Send to Cable Input (virtual output)
                if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                    // AudioPipeline kullanarak gönder
                    if (audioPipeline) {
                        // Write processed audio to appropriate pipeline buffer
                        if (!usingEffects) {
                            if (ui->robotButton->isChecked() || ui->bananaButton->isChecked() || 
                                ui->devilButton->isChecked() || ui->femaleButton->isChecked() || 
                                ui->combineButton->isChecked() || ui->ekoButton->isChecked()) {
                                // Send to effects buffer
                                audioPipeline->writeEffectsAudio(data);
                            } else {
                                // Send to clean input buffer
                                audioPipeline->writeInputAudio(data);
                            }
                        } else {
                            // Already processed audio, send to effects buffer
                            audioPipeline->writeEffectsAudio(data);
                        }
                    } else {
                        virtualOutputDevice->write(data);
                    }
                }
                
                // Fiziksel output'a gönderme (sadece test butonu aktifken)
                
                // Emit signal for recording
                if (isRecording) {
                    qDebug() << "RECORDING CONNECTION: Processed audio size:" << data.size() << "bytes";
                    emit audioDataReady(data);
                }
            });
            qDebug() << "Recording audio connection established AFTER file dialog closed";
        }

        // Update UI
        ui->startRecord->setText("Kaydediliyor...");
        ui->startRecord->setStyleSheet(
            "QPushButton {"
            "background-color: #ff4444;"
            "color: white;"
            "border: 2px solid #cc0000;"
            "border-radius: 5px;"
            "padding: 5px;"
            "}"
            "QPushButton:hover {"
            "background-color: #ff6666;"
            "}"
            );
        ui->startRecord->setEnabled(false);

        // Create processing timer (every 40ms for smooth recording)
        recordingProcessorTimer = new QTimer(this);
        connect(recordingProcessorTimer, &QTimer::timeout, this, &MainWindow::processRecordedFrames);
        recordingProcessorTimer->start(40); // 25 FPS processing

        // Connect audio data signal to circular buffer
        // First disconnect any existing recording-specific connections
        disconnect(this, &MainWindow::audioDataReady, this, nullptr);

        // Create a timer to capture mixed audio from AudioPipeline for recording
        QTimer* mixedAudioCaptureTimer = new QTimer(this);
        connect(mixedAudioCaptureTimer, &QTimer::timeout, this, [this, isInputMono, mixedAudioCaptureTimer]() {
            if (!isRecording || !audioPipeline) return;
            
            // Get mixed audio from AudioPipeline (this includes input + effects + soundpack)
            QByteArray mixedAudio = audioPipeline->getMixedAudioData(4096); // 4KB chunks
            
            if (!mixedAudio.isEmpty()) {
                qDebug() << "RECORDING: Mixed audio captured size:" << mixedAudio.size() << "bytes";
                // Write mixed audio to circular buffer for recording
                writeToCircularBuffer(mixedAudio, false); // Mixed audio is always stereo
            }
        });
        
        // Start capturing mixed audio every 20ms for high quality recording
        mixedAudioCaptureTimer->start(20);
        
        // Store timer reference for cleanup
        recordingMixedAudioTimer = mixedAudioCaptureTimer;

        // Store codec info for use in processRecordedFrames
        recordingCodec = codec;
        qDebug() << "Mono input:" << isInputMono
                 << "Target frame size:" << codecContext->frame_size
                 << "Processing interval: 40ms";

    } catch (const std::exception &e) {
        qCritical() << "Error during recording setup: " << e.what();
        cleanupRecording();
    }
}


void MainWindow::writeToCircularBuffer(const QByteArray& audioData, bool isMono)
{
    std::lock_guard<std::mutex> lock(bufferMutex);

    const int16_t* sourceData = reinterpret_cast<const int16_t*>(audioData.constData());
    size_t sourceSamples = audioData.size() / sizeof(int16_t);

    if (isMono) {
        // Mono to stereo conversion
        for (size_t i = 0; i < sourceSamples; i++) {
            int16_t sample = sourceData[i];

            // Left channel
            circularBuffer[writePos] = sample;
            writePos = (writePos + 1) % circularBuffer.size();

            // Right channel (same data)
            circularBuffer[writePos] = sample;
            writePos = (writePos + 1) % circularBuffer.size();

            availableSamples += 2;

            // Handle buffer overflow
            if (availableSamples > bufferSize * 2) {
                readPos = (readPos + 2) % circularBuffer.size();
                availableSamples -= 2;
            }
        }
    } else {
        // Already stereo
        for (size_t i = 0; i < sourceSamples; i++) {
            circularBuffer[writePos] = sourceData[i];
            writePos = (writePos + 1) % circularBuffer.size();
            availableSamples++;

            if (availableSamples > bufferSize * 2) {
                readPos = (readPos + 1) % circularBuffer.size();
                availableSamples--;
            }
        }
    }

    // Debug: Log buffer status occasionally
    static int writeCount = 0;
    if (++writeCount % 100 == 0) {
        qDebug() << "Buffer write - Available:" << availableSamples
                 << "samples, Write position:" << writePos;
    }
}

QByteArray MainWindow::readFromCircularBuffer(size_t samplesNeeded)
{
    std::lock_guard<std::mutex> lock(bufferMutex);

    if (availableSamples < samplesNeeded) {
        return QByteArray(); // Not enough data
    }

    QByteArray result;
    result.resize(samplesNeeded * sizeof(int16_t));
    int16_t* destData = reinterpret_cast<int16_t*>(result.data());

    for (size_t i = 0; i < samplesNeeded; i++) {
        destData[i] = circularBuffer[readPos];
        readPos = (readPos + 1) % circularBuffer.size();
    }

    availableSamples -= samplesNeeded;
    return result;
}

void MainWindow::processRecordedFrames()
{
    if (!isRecording || !codecContext || availableSamples == 0) {
        return;
    }
    
    // For MP3, swrCtx is required; for WAV, it's optional
    bool isMP3 = (codecContext->codec_id == AV_CODEC_ID_MP3);
    if (isMP3 && !swrCtx) {
        return;
    }

    try {
        // Determine frame size based on codec
        bool isMP3MF = recordingCodec && (QString(recordingCodec->name) == "mp3_mf");
        
        // Frame size: mp3_mf uses 1024, other MP3 uses 1152, WAV is flexible
        size_t SAMPLES_PER_CHANNEL;
        if (isMP3MF) {
            SAMPLES_PER_CHANNEL = 1024;
        } else if (isMP3) {
            SAMPLES_PER_CHANNEL = 1152;
        } else {
            SAMPLES_PER_CHANNEL = 1024; // WAV flexible
        }
        
        size_t SAMPLES_PER_FRAME = SAMPLES_PER_CHANNEL * 2; // Stereo
        size_t BYTES_PER_FRAME = SAMPLES_PER_FRAME * sizeof(int16_t);
        
        qDebug() << "Frame processing - isMP3:" << isMP3 << "isMP3MF:" << isMP3MF << "samplesPerChannel:" << SAMPLES_PER_CHANNEL;

        // Process as many frames as we have data for
        while (availableSamples >= SAMPLES_PER_FRAME) {
            // Read exactly one frame from circular buffer
            QByteArray frameData = readFromCircularBuffer(SAMPLES_PER_FRAME);
            if (frameData.size() != BYTES_PER_FRAME) {
                qWarning() << "Frame data size mismatch:" << frameData.size()
                << "expected:" << BYTES_PER_FRAME;
                break;
            }

            // Prepare input for resampler
            const uint8_t *inData[AV_NUM_DATA_POINTERS] = {
                reinterpret_cast<const uint8_t*>(frameData.constData())
        };

        // Allocate output buffer for exactly the right number of samples
        uint8_t *outData[AV_NUM_DATA_POINTERS] = { nullptr };
        int outLinesize;

        AVSampleFormat targetFormat;
        if (isMP3MF) {
            // Check what format mp3_mf actually opened with
            if (codecContext->sample_fmt == AV_SAMPLE_FMT_FLT) {
                targetFormat = AV_SAMPLE_FMT_FLT;  // mp3_mf with FLT
            } else {
                targetFormat = AV_SAMPLE_FMT_S16;  // mp3_mf with S16 fallback
            }
        } else if (isMP3) {
            targetFormat = AV_SAMPLE_FMT_FLTP;  // Other MP3 uses FLTP
        } else {
            targetFormat = AV_SAMPLE_FMT_S16;  // WAV uses S16
        }
        
        int allocateResult = av_samples_alloc(outData, &outLinesize,
                                              codecContext->ch_layout.nb_channels,
                                              SAMPLES_PER_CHANNEL,
                                              targetFormat, 0);

        if (allocateResult < 0) {
            qWarning() << "Failed to allocate output samples";
            continue;
        }

        // Calculate input samples for resampler
        int inputSamples = SAMPLES_PER_CHANNEL;

        int convertedSamples;
        if (isMP3MF) {
            // mp3_mf needs resampling but stays S16 (non-planar)
            convertedSamples = swr_convert(swrCtx, outData, SAMPLES_PER_CHANNEL,
                                          inData, inputSamples);
        } else if (isMP3) {
            // Other MP3 needs resampling to planar float
            convertedSamples = swr_convert(swrCtx, outData, SAMPLES_PER_CHANNEL,
                                          inData, inputSamples);
        } else {
            // WAV can use the data directly (already S16 format)
            // Copy directly to output buffer
            for (int ch = 0; ch < codecContext->ch_layout.nb_channels; ch++) {
                if (outData[ch]) {
                    int16_t* channelData = reinterpret_cast<int16_t*>(outData[ch]);
                    const int16_t* sourceData = reinterpret_cast<const int16_t*>(frameData.constData());

                    for (size_t i = 0; i < SAMPLES_PER_CHANNEL; i++) {
                        channelData[i] = sourceData[i * codecContext->ch_layout.nb_channels + ch];
                    }
                }
            }
            convertedSamples = SAMPLES_PER_CHANNEL;
        }

        if (convertedSamples != SAMPLES_PER_CHANNEL) {
            qWarning() << "Resampling produced" << convertedSamples
                       << "samples, expected" << SAMPLES_PER_CHANNEL;

            // Pad with zeros if necessary
            if (convertedSamples > 0 && convertedSamples < SAMPLES_PER_CHANNEL) {
                for (int ch = 0; ch < codecContext->ch_layout.nb_channels; ch++) {
                    if (outData[ch]) {
                        float* channelData = reinterpret_cast<float*>(outData[ch]);
                        for (int i = convertedSamples; i < SAMPLES_PER_CHANNEL; i++) {
                            channelData[i] = 0.0f;
                        }
                    }
                }
                convertedSamples = SAMPLES_PER_CHANNEL;
            } else {
                av_freep(&outData[0]);
                continue;
            }
        }

        // Copy to frame buffer
        for (int ch = 0; ch < codecContext->ch_layout.nb_channels; ch++) {
            if (frame->data[ch] && outData[ch]) {
                size_t bytesToCopy;
                if (isMP3MF) {
                    if (codecContext->sample_fmt == AV_SAMPLE_FMT_FLT) {
                        bytesToCopy = SAMPLES_PER_CHANNEL * sizeof(float);  // mp3_mf with FLT
                    } else {
                        bytesToCopy = SAMPLES_PER_CHANNEL * sizeof(int16_t);  // mp3_mf with S16
                    }
                } else if (isMP3) {
                    bytesToCopy = SAMPLES_PER_CHANNEL * sizeof(float);    // Other MP3 uses FLTP
                } else {
                    bytesToCopy = SAMPLES_PER_CHANNEL * sizeof(int16_t); // WAV uses S16
                }
                memcpy(frame->data[ch], outData[ch], bytesToCopy);
            }
        }

        // Set frame properties
        frame->nb_samples = SAMPLES_PER_CHANNEL;
        frame->pts = pts;
        pts += SAMPLES_PER_CHANNEL;

        // Send frame to encoder
        int sendResult = avcodec_send_frame(codecContext, frame);
        if (sendResult < 0) {
            char errorStr[256];
            av_strerror(sendResult, errorStr, sizeof(errorStr));
            qWarning() << "Failed to send frame:" << errorStr;
            av_freep(&outData[0]);
            continue;
        }

        // Receive encoded packets
        while (avcodec_receive_packet(codecContext, packet) == 0) {
            packet->stream_index = audioStream->index;
            av_packet_rescale_ts(packet, codecContext->time_base, audioStream->time_base);

            if (av_write_frame(formatContext, packet) < 0) {
                qWarning() << "Failed to write packet";
            }

            av_packet_unref(packet);
        }

        // Cleanup
        av_freep(&outData[0]);

        // Debug output
        static int totalFrames = 0;
        if (++totalFrames % 100 == 0) {
            qDebug() << "Processed" << totalFrames << "frames. Buffer:"
                     << availableSamples << "samples remaining";
        }
    }

} catch (const std::exception& e) {
    qWarning() << "Error in processRecordedFrames:" << e.what();
}
}

void MainWindow::cleanupRecording()
{
    if (recordingProcessorTimer) {
        recordingProcessorTimer->stop();
        recordingProcessorTimer->deleteLater();
        recordingProcessorTimer = nullptr;
    }

    if (recordingMixedAudioTimer) {
        recordingMixedAudioTimer->stop();
        recordingMixedAudioTimer->deleteLater();
        recordingMixedAudioTimer = nullptr;
    }

    // Clear circular buffer
    {
        std::lock_guard<std::mutex> lock(bufferMutex);
        circularBuffer.clear();
        readPos = writePos = availableSamples = 0;
    }

    isRecording = false;

    ui->startRecord->setText("Start Record");
    ui->startRecord->setStyleSheet("");
    ui->startRecord->setEnabled(true);

    // Don't restore test button state since we don't auto-enable it anymore
    Q_UNUSED(testButtonWasActive);

    qDebug() << "Recording cleanup completed";
}

void MainWindow::on_stopRecord_clicked()
{
    if (!isRecording) {
        qDebug() << "No active recording to stop";
        return;
    }

    qDebug() << "=== STOPPING RECORDING ===";
    qDebug() << "Recording duration:" << recordingTimer.elapsed() << "ms";
    qDebug() << "Available samples in buffer:" << availableSamples;

    // Stop processing timer
    if (recordingProcessorTimer) {
        recordingProcessorTimer->stop();
        recordingProcessorTimer->deleteLater();
        recordingProcessorTimer = nullptr;
    }

    // Stop mixed audio capture timer
    if (recordingMixedAudioTimer) {
        recordingMixedAudioTimer->stop();
        recordingMixedAudioTimer->deleteLater();
        recordingMixedAudioTimer = nullptr;
    }

    // Process any remaining frames in buffer
    if (availableSamples > 0) {
        qDebug() << "Processing remaining" << availableSamples << "samples...";
        processRecordedFrames();
    }

    // FFmpeg cleanup
    if (formatContext) {
        // Flush encoder
        if (codecContext) {
            avcodec_send_frame(codecContext, nullptr);

            AVPacket *flushPacket = av_packet_alloc();
            if (flushPacket) {
                while (avcodec_receive_packet(codecContext, flushPacket) == 0) {
                    flushPacket->stream_index = audioStream->index;
                    av_packet_rescale_ts(flushPacket, codecContext->time_base, audioStream->time_base);
                    av_write_frame(formatContext, flushPacket);
                    av_packet_unref(flushPacket);
                }
                av_packet_free(&flushPacket);
            }
        }

        // Write trailer
        av_write_trailer(formatContext);

        // Close file
        if (!(formatContext->oformat->flags & AVFMT_NOFILE) && formatContext->pb) {
            avio_closep(&formatContext->pb);
        }

        // Free resources
        avcodec_free_context(&codecContext);
        avformat_free_context(formatContext);
        formatContext = nullptr;

        if (frame) {
            av_frame_free(&frame);
            frame = nullptr;
        }
        if (packet) {
            av_packet_free(&packet);
            packet = nullptr;
        }
        if (swrCtx) {
            swr_free(&swrCtx);
            swrCtx = nullptr;
        }
    }

    // Disconnect recording signal
    disconnect(this, &MainWindow::audioDataReady, this, nullptr);

    // Restore original audio connection
    if (inputDevice) {
        disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);

        // Restore the initial connection (like in on_inputcombobox_currentIndexChanged)
        connect(inputDevice, &QIODevice::readyRead, this, [=](){
            data = inputDevice->readAll();
            progressBarOutput();

            // Emit signal for any future recordings
            if (isRecording) {
                qDebug() << "EMITTING SIGNAL (RESTORED): Audio size:" << data.size() << "bytes";
                emit audioDataReady(data);
            }

            // Send to virtual output (Cable Input) - use proper AudioPipeline routing
            if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                if (audioPipeline && !isRecording) {
                    // Check what type of audio we have and route appropriately
                    bool hasEffects = false;
                    bool hasSoundpack = false;
                    
                    // Check if any effect is active
                    if (ui->robotButton->isChecked() || ui->bananaButton->isChecked() || 
                        ui->devilButton->isChecked() || ui->femaleButton->isChecked() || 
                        ui->combineButton->isChecked() || ui->ekoButton->isChecked()) {
                        hasEffects = true;
                    }
                    
                    // Check if soundpack is playing (you might need to add this check)
                    // For now, assume soundpack is detected by currentPlayingButton
                    if (currentPlayingButton != nullptr) {
                        hasSoundpack = true;
                    }
                    
                    // Route audio based on what's active
                    if (hasEffects) {
                        // Effects audio - highest priority
                        audioPipeline->writeEffectsAudio(data);
                        qDebug() << "Routing to effects audio (effects active)";
                    } else if (hasSoundpack) {
                        // Soundpack audio
                        audioPipeline->writeSoundpackAudio(data);
                        qDebug() << "Routing to soundpack audio (soundpack active)";
                    } else {
                        // Clean input audio
                        audioPipeline->writeInputAudio(data);
                        qDebug() << "Routing to input audio (clean voice)";
                    }
                    
                    // Trigger audio mixing if needed
                    if (hasEffects || hasSoundpack) {
                        audioPipeline->processBuffers();
                    }
                } else if (!audioPipeline) {
                    // Fallback to direct write if AudioPipeline not available
                    virtualOutputDevice->write(data);
                }
            }

            // Fiziksel output'a gönderme (sadece test butonu aktifken)
        });
        qDebug() << "Original audio connection restored after recording";
    }

    // Cleanup and restore UI
    cleanupRecording();

    qDebug() << "=== RECORDING STOPPED SUCCESSFULLY ===";
    qDebug() << "Total recording time:" << recordingTimer.elapsed() << "ms";
}

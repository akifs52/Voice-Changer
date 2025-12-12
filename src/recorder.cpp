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

        // Auto-enable test button if not already active
        testButtonWasActive = ui->testButton->isChecked();
        if (!testButtonWasActive) {
            qDebug() << "Auto-enabling test button for recording";
            ui->testButton->setChecked(true);
            ui->testButton->setText("Stop");
        }

        // Resume audio input if suspended
        if (audioInput->state() == QAudio::SuspendedState) {
            audioInput->resume();
        }

        // Select file to save MP3
        QString mp3FileName = QFileDialog::getSaveFileName(this, "Save MP3 File", "", "MP3 Files (*.mp3)");
        if (mp3FileName.isEmpty()) {
            qCritical() << "No file selected.";
            if (!testButtonWasActive) {
                ui->testButton->setChecked(false);
            }
            return;
        }

        // Ensure .mp3 extension
        if (!mp3FileName.endsWith(".mp3", Qt::CaseInsensitive)) {
            mp3FileName += ".mp3";
        }

        qDebug() << "Saving to file:" << mp3FileName;

        // Initialize FFmpeg
        avformat_network_init();

        if (avformat_alloc_output_context2(&formatContext, nullptr, "mp3", mp3FileName.toStdString().c_str()) < 0) {
            qCritical() << "Failed to allocate format context.";
            return;
        }

        // Find MP3 codec
        const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
        if (!codec) {
            qCritical() << "MP3 codec not found. Ensure FFmpeg is built with libmp3lame.";
            return;
        }

        // Configure codec parameters
        codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            qCritical() << "Failed to allocate codec context.";
            return;
        }

        codecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
        codecContext->bit_rate = 128000;
        codecContext->sample_rate = 44100;
        av_channel_layout_default(&codecContext->ch_layout, 2); // Stereo
        codecContext->frame_size = 1152; // MP3 için sabit frame size

        // MP3 için optimize ayarlar
        codecContext->compression_level = 5;
        av_opt_set_int(codecContext, "compression_level", 5, 0);

        if (formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
            codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        if (avcodec_open2(codecContext, codec, nullptr) < 0) {
            qCritical() << "Failed to open MP3 codec.";
            avcodec_free_context(&codecContext);
            return;
        }

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
            if (avio_open(&formatContext->pb, mp3FileName.toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
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

        // Configure resampler
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

        // Create and configure SwrContext
        swrCtx = swr_alloc();

        // Set resampler options with explicit settings
        if (swr_alloc_set_opts2(&swrCtx,
                                &outputChannelLayoutStruct, AV_SAMPLE_FMT_FLTP, codecContext->sample_rate,
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
        qDebug() << "SwrContext initialized. Delay:" << swrDelay << "samples";

        // Allocate frame
        frame = av_frame_alloc();
        if (!frame) {
            qCritical() << "Failed to allocate frame.";
            return;
        }

        frame->nb_samples = codecContext->frame_size; // 1152 samples
        frame->format = codecContext->sample_fmt;     // AV_SAMPLE_FMT_FLTP
        av_channel_layout_default(&frame->ch_layout, codecContext->ch_layout.nb_channels);

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
        // First disconnect any existing connections
        disconnect(this, &MainWindow::audioDataReady, this, nullptr);

        // Connect to circular buffer writer
        connect(this, &MainWindow::audioDataReady, this, [this, isInputMono](const QByteArray& audioData) {
            if (!isRecording || audioData.isEmpty()) return;

            // Write to circular buffer
            writeToCircularBuffer(audioData, isInputMono);
        });

        qDebug() << "Recording system initialized successfully";
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
    if (!isRecording || !codecContext || !swrCtx || availableSamples == 0) {
        return;
    }

    try {
        // MP3 requires exactly 1152 samples per channel per frame
        // For stereo: 1152 * 2 = 2304 int16_t samples
        const size_t SAMPLES_PER_CHANNEL = 1152;
        const size_t SAMPLES_PER_FRAME = SAMPLES_PER_CHANNEL * 2; // Stereo
        const size_t BYTES_PER_FRAME = SAMPLES_PER_FRAME * sizeof(int16_t);

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

        // Allocate output buffer for exactly 1152 samples
        uint8_t *outData[AV_NUM_DATA_POINTERS] = { nullptr };
        int outLinesize;

        int allocateResult = av_samples_alloc(outData, &outLinesize,
                                              codecContext->ch_layout.nb_channels,
                                              SAMPLES_PER_CHANNEL,
                                              AV_SAMPLE_FMT_FLTP, 0);

        if (allocateResult < 0) {
            qWarning() << "Failed to allocate output samples";
            continue;
        }

        // Calculate input samples for resampler
        int inputSamples = SAMPLES_PER_CHANNEL; // 1152 samples per channel

        // Perform resampling - CRITICAL: Must produce exactly 1152 samples
        int convertedSamples = swr_convert(swrCtx, outData, SAMPLES_PER_CHANNEL,
                                           inData, inputSamples);

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

        // Copy to frame buffer (planar format)
        for (int ch = 0; ch < codecContext->ch_layout.nb_channels; ch++) {
            if (frame->data[ch] && outData[ch]) {
                size_t bytesToCopy = SAMPLES_PER_CHANNEL * sizeof(float);
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

    // Clear circular buffer
    {
        std::lock_guard<std::mutex> lock(bufferMutex);
        circularBuffer.clear();
        readPos = writePos = availableSamples = 0;
    }

    isRecording = false;

    ui->startRecord->setStyleSheet("");
    ui->startRecord->setEnabled(true);

    if (!testButtonWasActive && ui->testButton) {
        ui->testButton->setChecked(false);

    }

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

    // Cleanup and restore UI
    cleanupRecording();

    qDebug() << "=== RECORDING STOPPED SUCCESSFULLY ===";
    qDebug() << "Total recording time:" << recordingTimer.elapsed() << "ms";
}

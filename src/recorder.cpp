#include "recorder.h"
#include <QFileDialog>
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
    try {
        // Clear previous audio data
        data.clear();

        if (!audioInput) {
            audioInput->resume();
        }

        // Select file to save MP3
        QString mp3FileName = QFileDialog::getSaveFileName(this, "Save MP3 File", "", "MP3 Files (*.mp3)");
        if (mp3FileName.isEmpty()) {
            qCritical() << "No file selected.";
            return;
        }

        // Initialize FFmpeg
        avformat_network_init();

        if (avformat_alloc_output_context2(&formatContext, nullptr, "mp3", mp3FileName.toStdString().c_str()) < 0) {
            qCritical() << "Failed to allocate format context.";
            return;
        }

        // Find MP3 codec
        const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
        if (!codec) {
            qCritical() << "MP3 codec not found. Ensure FFmpeg is built with MP3 support.";
            return;
        }

        // Configure codec parameters
        codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            qCritical() << "Failed to allocate codec context.";
            return;
        }

        codecContext->sample_fmt = AV_SAMPLE_FMT_FLTP; // Use floating point format for compatibility
        codecContext->bit_rate = 192000; // MP3 bitrate
        codecContext->sample_rate = 48000; // Sampling rate
        av_channel_layout_default(&codecContext->ch_layout, 2); // Stereo



        if (formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
            codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        if (avcodec_open2(codecContext, codec, nullptr) < 0) {
            qCritical() << "Failed to open codec.";
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

        qDebug() << "Recording started.";

        AVChannelLayout inputChannelLayoutStruct;
        AVChannelLayout outputChannelLayoutStruct;

        av_channel_layout_default(&inputChannelLayoutStruct, 2); // Stereo
        av_channel_layout_default(&outputChannelLayoutStruct, 2); // Stereo

        // Get actual input sample rate
        int inputSampleRate = 48000;
        if (audioInput && audioInput->format().sampleRate() > 0) {
            inputSampleRate = audioInput->format().sampleRate();
        }

        swrCtx = swr_alloc();
        if (!swrCtx) {
            qCritical() << "Failed to allocate SwrContext.";
            return;
        }

        if (swr_alloc_set_opts2(&swrCtx, &outputChannelLayoutStruct, AV_SAMPLE_FMT_FLTP, codecContext->sample_rate,
                                &inputChannelLayoutStruct, AV_SAMPLE_FMT_S16, inputSampleRate, 0, nullptr) < 0) {
            qCritical() << "Failed to allocate and set options for SwrContext.";
            return;
        }

        if (swr_init(swrCtx) < 0) {
            qCritical() << "Failed to initialize SwrContext.";
            swr_free(&swrCtx);
            return;
        }

        frame = av_frame_alloc();
        if (!frame) {
            qCritical() << "Failed to allocate frame.";
            avcodec_free_context(&codecContext);
            return;
        }

        // Set frame properties before allocating buffer
        frame->nb_samples = codecContext->frame_size;
        frame->format = codecContext->sample_fmt;
        av_channel_layout_default(&frame->ch_layout, codecContext->ch_layout.nb_channels);

        if (av_frame_get_buffer(frame, 0) < 0) {
            qCritical() << "Failed to allocate audio frame buffer.";
            av_frame_free(&frame);
            avcodec_free_context(&codecContext);
            return;
        }

        packet = av_packet_alloc();
        if (!packet) {
            qCritical() << "Failed to allocate packet.";
            av_frame_free(&frame);
            avcodec_free_context(&codecContext);
            return;
        }

        audioInput->setBufferSize(4096);  // Reduce buffer size for real-time recording
        
        // Add recording status indicator
        ui->startRecord->setText("Kaydediliyor...");
        ui->startRecord->setStyleSheet("background-color: #ff4444; color: white;");

        // Buffer to accumulate audio data for proper frame size
        QByteArray audioBuffer;
        const int frameSize = codecContext->frame_size;
        const int bytesPerSample = 4; // 16-bit stereo = 4 bytes
        const int requiredBytes = frameSize * bytesPerSample;
        
        // Get actual input sample rate from audio input

        if (audioInput && audioInput->format().sampleRate() > 0) {
            inputSampleRate = audioInput->format().sampleRate();
        }

        connect(inputDevice, &QIODevice::readyRead, this, [=]() mutable {
            try {
                if (!formatContext || !codecContext) {
                    return;  // Recording not initialized
                }
                
                QByteArray newData = inputDevice->readAll();
                if (newData.isEmpty()) {
                    return;
                }

                // Append new data to buffer
                audioBuffer.append(newData);

                // Process complete frames
                while (audioBuffer.size() >= requiredBytes) {
                    QByteArray frameData = audioBuffer.left(requiredBytes);
                    audioBuffer = audioBuffer.mid(requiredBytes);

                    // Process frame data
                    for (int i = 0; i < frameData.size(); ++i) {
                        frameData[i] = static_cast<char>(qMin(qMax(static_cast<int>(frameData[i]), -32768), 32767));
                    }

                    // Calculate input samples correctly
                    int inputSamples = frameData.size() / bytesPerSample;
                    
                    const uint8_t *inData[AV_NUM_DATA_POINTERS] = { reinterpret_cast<const uint8_t *>(frameData.data()) };
                    uint8_t *outData[AV_NUM_DATA_POINTERS] = { nullptr };

                    int outLinesize;
                    int outSamples = av_samples_alloc(outData, &outLinesize, codecContext->ch_layout.nb_channels,
                                                      frameSize, codecContext->sample_fmt, 0);

                    if (outSamples < 0) {
                        continue;
                    }

                    // Use actual input sample rate for conversion
                    int convertedSamples = swr_convert(swrCtx, outData, frameSize, inData, inputSamples);

                    if (convertedSamples <= 0) {
                        av_freep(&outData[0]);
                        continue;
                    }

                    // Copy converted data to frame
                    if (av_samples_fill_arrays(frame->data, frame->linesize, outData[0], codecContext->ch_layout.nb_channels,
                                               convertedSamples, codecContext->sample_fmt, 0) < 0) {
                        av_freep(&outData[0]);
                        continue;
                    }

                    // Set correct PTS based on actual sample rate
                    frame->pts = pts;
                    pts += convertedSamples;

                    if (avcodec_send_frame(codecContext, frame) < 0) {
                        av_freep(&outData[0]);
                        continue;
                    }

                    while (avcodec_receive_packet(codecContext, packet) == 0) {
                        packet->stream_index = audioStream->index;
                        av_packet_rescale_ts(packet, codecContext->time_base, audioStream->time_base);
                        av_write_frame(formatContext, packet);
                        av_packet_unref(packet);
                    }

                    av_freep(&outData[0]);
                }
            } catch (const std::exception &e) {
                qCritical() << "Error during recording: " << e.what();
            }
        });
    } catch (const std::runtime_error &e) {
        qCritical() << "Error occurred: " << e.what();
        avcodec_free_context(&codecContext);
        return;
    }
}

void MainWindow::on_stopRecord_clicked()
{
    if (formatContext) {
        // Flush the encoder with null frame
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
        if (av_write_trailer(formatContext) < 0) {
            qCritical() << "Failed to write trailer";
        }

        // Close output file
        if (!(formatContext->oformat->flags & AVFMT_NOFILE) && formatContext->pb) {
            avio_closep(&formatContext->pb);
        }

        // Clean up FFmpeg resources
        if (codecContext) {
            avcodec_free_context(&codecContext);
        }
        if (formatContext) {
            avformat_free_context(formatContext);
            formatContext = nullptr;
        }
        if (frame) {
            av_frame_free(&frame);
        }
        if (packet) {
            av_packet_free(&packet);
        }
        if (swrCtx) {
            swr_free(&swrCtx);
        }

        qDebug() << "Kayıt durduruldu";
        
        // Restore button state
        ui->startRecord->setText("Kaydı Başlat");
        ui->startRecord->setStyleSheet("");  // Reset to default style
        
        // Show status message
        qDebug() << "Kayıt durdu - MP3 dosyası başarıyla kaydedildi";

        pts = 0;
        
        // Don't stop audio input - keep it running for voice changer
        if (audioInput && audioInput->state() == QAudio::SuspendedState) {
            audioInput->resume();
        }
    }
}


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

        av_channel_layout_default(&inputChannelLayoutStruct, 2); // Mono
        av_channel_layout_default(&outputChannelLayoutStruct, 2); // Stereo

        swrCtx = swr_alloc();
        if (!swrCtx) {
            qCritical() << "Failed to allocate SwrContext.";
            return;
        }

        if (swr_alloc_set_opts2(&swrCtx, &outputChannelLayoutStruct, AV_SAMPLE_FMT_FLTP, codecContext->sample_rate,
                                &inputChannelLayoutStruct, AV_SAMPLE_FMT_S16, codecContext->sample_rate, 0, nullptr) < 0) {
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

        audioInput->setBufferSize(16384);

        connect(inputDevice, &QIODevice::readyRead, this, [=]() {
            try {
                if (usingEffects) {
                    data = inputDevice->readAll();
                }

                if (data.isEmpty()) {
                    qWarning() << "No data received.";
                    return;
                }

                for (int i = 0; i < data.size(); ++i)
                {
                    data[i] = static_cast<char>(qMin(qMax(static_cast<int>(data[i]), -32768), 32767));
                }

                const uint8_t *inData[AV_NUM_DATA_POINTERS] = { reinterpret_cast<const uint8_t *>(data.data()) };
                uint8_t *outData[AV_NUM_DATA_POINTERS] = { nullptr };

                int outLinesize;
                int outSamples = av_samples_alloc(outData, &outLinesize, codecContext->ch_layout.nb_channels,
                                                  frame->nb_samples, codecContext->sample_fmt, 0);

                if (outSamples < 0) {
                    qCritical() << "Failed to allocate output samples.";
                    return;
                }

                int convertedSamples = swr_convert(swrCtx, outData, frame->nb_samples, inData, frame->nb_samples);

                if (convertedSamples < 0) {
                    qCritical() << "Failed to convert audio samples.";
                    av_freep(&outData[0]);
                    return;
                }

                if (av_samples_fill_arrays(frame->data, frame->linesize, outData[0], codecContext->ch_layout.nb_channels,
                                           convertedSamples, codecContext->sample_fmt, 0) < 0) {
                    qCritical() << "Failed to fill frame with converted data.";
                    av_freep(&outData[0]);
                    return;
                }

                frame->pts = pts;
                pts += frame->nb_samples;

                if (avcodec_send_frame(codecContext, frame) < 0) {
                    qWarning() << "Failed to send frame to encoder.";
                    av_freep(&outData[0]);
                    return;
                }

                while (avcodec_receive_packet(codecContext, packet) == 0) {
                    packet->stream_index = audioStream->index;
                    av_packet_rescale_ts(packet, codecContext->time_base, audioStream->time_base);
                    av_write_frame(formatContext, packet);
                    av_packet_unref(packet);
                }

                av_freep(&outData[0]);
            } catch (const std::exception &e) {
                qCritical() << "Error during processing input data: " << e.what();
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
        // Flush the encoder
        if (codecContext) {
            avcodec_send_frame(codecContext, nullptr);
            while (avcodec_receive_packet(codecContext, packet) == 0) {
                av_write_frame(formatContext, packet);
                av_packet_unref(packet);
            }
        }

        // Write trailer
        av_write_trailer(formatContext);

        if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&formatContext->pb);
        }

        avcodec_free_context(&codecContext);
        avformat_free_context(formatContext);
        av_frame_free(&frame);
        av_packet_free(&packet);

        qDebug() << " recording stop";

        pts = 0;

        disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
    }
}

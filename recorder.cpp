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

        // Check supported sample formats
        const enum AVSampleFormat *sample_fmts = codec->sample_fmts;
        if (!sample_fmts) {
            qCritical() << "No sample formats available for the codec.";
            return;
        }

        qDebug() << "Supported sample formats:";
        bool formatFound = false;
        AVSampleFormat selectedSampleFormat = AV_SAMPLE_FMT_NONE;
        int i = 0;
        while (sample_fmts[i] != AV_SAMPLE_FMT_NONE) {
            qDebug() << " - " << av_get_sample_fmt_name(sample_fmts[i]);
            if (!formatFound && (sample_fmts[i] == AV_SAMPLE_FMT_S32P)) {
                selectedSampleFormat = sample_fmts[i];
                formatFound = true;
            }
            i++;
        }

        if (!formatFound) {
            qCritical() << "No valid sample format found for the codec.";
            return;
        }

        qDebug() << "Selected sample format:" << av_get_sample_fmt_name(selectedSampleFormat);

        // Configure codec parameters
        try {
            codecContext = avcodec_alloc_context3(codec);
            if (!codecContext) {
                throw std::runtime_error("Failed to allocate codec context.");
            }

            codecContext->sample_fmt = selectedSampleFormat;
            codecContext->bit_rate = 192000; // MP3 bitrate
            codecContext->sample_rate = 48000; // Sampling rate
            av_channel_layout_default(&codecContext->ch_layout, 2); // Stereo

            if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
                codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            if (avcodec_open2(codecContext, codec, nullptr) < 0) {
                throw std::runtime_error("Failed to open codec.");
            }

            audioStream = avformat_new_stream(formatContext, nullptr);
            if (!audioStream) {
                throw std::runtime_error("Failed to create new stream.");
            }

            if (avcodec_parameters_from_context(audioStream->codecpar, codecContext) < 0) {
                throw std::runtime_error("Failed to copy codec parameters.");
            }
        } catch (const std::exception &e) {
            qCritical() << "Error occurred while configuring codec: " << e.what();
            avcodec_free_context(&codecContext);
            avformat_free_context(formatContext);
            return;
        }

        // Open output file
        if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&formatContext->pb, mp3FileName.toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
                qCritical() << "Failed to open output file.";
                avcodec_free_context(&codecContext);
                avformat_free_context(formatContext);
                return;
            }
        }

        if (avformat_write_header(formatContext, nullptr) < 0) {
            qCritical() << "Failed to write header.";
            avcodec_free_context(&codecContext);
            avformat_free_context(formatContext);
            return;
        }

        qDebug() << "Recording started.";

        // Initialize SwrContext for format conversion
        swrCtx = swr_alloc();
        if (!swrCtx) {
            qCritical() << "Failed to allocate SwrContext.";
            return;
        }

        int64_t inputChannelLayout = codecContext->ch_layout.u.mask; // Mono
        int64_t outputChannelLayout = AV_CH_LAYOUT_STEREO; // Stereo

        qDebug() << "Selected input channel layout: MONO";
        qDebug() << "Selected output channel layout: STEREO";



        // SwrContext ayarları
        av_opt_set_int(swrCtx, "in_channel_layout", inputChannelLayout, 0);
        av_opt_set_int(swrCtx, "out_channel_layout", outputChannelLayout, 0);
        av_opt_set_int(swrCtx, "in_sample_rate", codecContext->sample_rate, 0);
        av_opt_set_int(swrCtx, "out_sample_rate", codecContext->sample_rate, 0);
        av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", codecContext->sample_fmt, 0);
        av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", AV_SAMPLE_FMT_S32, 0);

        // SwrContext başlatma
        if (swr_init(swrCtx) < 0) {
            qCritical() << "Failed to initialize SwrContext.";
            qCritical() << "Details:";
            qCritical() << " - Input Channel Layout: " << inputChannelLayout;
            qCritical() << " - Output Channel Layout: " << outputChannelLayout;
            qCritical() << " - Input Sample Rate:" << codecContext->sample_rate;
            qCritical() << " - Output Sample Rate:" << codecContext->sample_rate;
            qCritical() << " - Input Sample Format:" << av_get_sample_fmt_name(codecContext->sample_fmt);
            qCritical() << " - Output Sample Format: s32";
            swr_free(&swrCtx);
            return;
        }

        qDebug() << "SwrContext initialized successfully.";

        qDebug() << "SwrContext initialized successfully.";

        // Prepare for data processing
        frame = av_frame_alloc();
        if (!frame) {
            qCritical() << "Failed to allocate frame.";
            avcodec_free_context(&codecContext);
            avformat_free_context(formatContext);
            return;
        }

        frame->nb_samples = codecContext->frame_size;
        frame->format = codecContext->sample_fmt;
        av_channel_layout_default(&frame->ch_layout, codecContext->ch_layout.nb_channels);

        if (av_frame_get_buffer(frame, 0) < 0) {
            qCritical() << "Failed to allocate audio frame buffer.";
            av_frame_free(&frame);
            avcodec_free_context(&codecContext);
            avformat_free_context(formatContext);
            return;
        }

        packet = av_packet_alloc();
        if (!packet) {
            qCritical() << "Failed to allocate packet.";
            av_frame_free(&frame);
            avcodec_free_context(&codecContext);
            avformat_free_context(formatContext);
            return;
        }

        connect(inputDevice, &QIODevice::readyRead, this, [=]() {
            try {
                data = inputDevice->readAll();
                if (data.isEmpty()) {
                    qWarning() << "No data received.";
                    return;
                }

                // Convert s32p to s32
                const uint8_t *inData[AV_NUM_DATA_POINTERS] = { reinterpret_cast<const uint8_t *>(data.data()) };
                uint8_t *outData[AV_NUM_DATA_POINTERS] = { nullptr };

                int outLinesize;
                int outSamples = av_samples_alloc(outData, &outLinesize, codecContext->ch_layout.nb_channels,
                                                  codecContext->frame_size, AV_SAMPLE_FMT_S32, 0);

                if (outSamples < 0) {
                    qCritical() << "Failed to allocate output samples.";
                    return;
                }

                int convertedSamples = swr_convert(swrCtx, outData, outSamples, inData, codecContext->frame_size);

                if (convertedSamples < 0) {
                    qCritical() << "Failed to convert audio samples.";
                    av_freep(&outData[0]);
                    return;
                }

                int dataSize = av_samples_fill_arrays(frame->data, frame->linesize,
                                                      outData[0], codecContext->ch_layout.nb_channels,
                                                      convertedSamples, AV_SAMPLE_FMT_S32, 0);

                if (dataSize < 0) {
                    qCritical() << "Failed to fill frame with converted data.";
                    av_freep(&outData[0]);
                    return;
                }

                if (avcodec_send_frame(codecContext, frame) < 0) {
                    qWarning() << "Failed to send frame to encoder.";
                    av_freep(&outData[0]);
                    return;
                }

                while (avcodec_receive_packet(codecContext, packet) == 0) {
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
        avformat_free_context(formatContext);
        return;
    }
}

void MainWindow::on_stopRecord_clicked()
{
    if (formatContext) {
        av_write_trailer(formatContext);

        if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&formatContext->pb);
        }

        avcodec_free_context(&codecContext);
        avformat_free_context(formatContext);
        av_frame_free(&frame);
        av_packet_free(&packet);

        qDebug() << "Recording stopped.";
    }

    data.clear();

    disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
}

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

}



AVFormatContext *formatContext = nullptr;
const AVOutputFormat *outputFormat = nullptr;
AVStream *audioStream = nullptr;
AVCodecContext *codecContext = nullptr;
const AVCodec *codec = nullptr;


recorder::recorder(QWidget *parent)
    : QMainWindow{parent}
{}



void MainWindow::on_startRecord_clicked()
{

    // Clean up previous audio data
    data.clear();

    if (!audioInput) {
        audioInput->resume();
    }

    // MP3 save file selection
    QString mp3FileName = QFileDialog::getSaveFileName(this, "Save MP3 File", "", "MP3 Files (*.mp3)");
    if (mp3FileName.isEmpty()) return;

    // FFmpeg configuration
    avformat_network_init();

    // Format allocation
    if (avformat_alloc_output_context2(&formatContext, nullptr, "mp3", mp3FileName.toStdString().c_str()) < 0) {
        qCritical() << "Failed to allocate output format context.";
        return;
    }
    outputFormat = formatContext->oformat;

    // Codec setup
    codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
    if (!codec) {
        qCritical() << "MP3 encoder not found.";
        return;
    }

    audioStream = avformat_new_stream(formatContext, nullptr);
    if (!audioStream) {
        qCritical() << "Failed to create new audio stream.";

        return;
    }

    codecContext = avcodec_alloc_context3(codec);
    codecContext->bit_rate = 128000;
    codecContext->sample_rate = 48000;
    av_channel_layout_default(&codecContext->ch_layout, 2); // Stereo

    // Set 16-bit sample format
    codecContext->sample_fmt = AV_SAMPLE_FMT_S16;

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    int ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        qCritical() << "Failed to open codec: " << errBuf;

        return;
    }

    if (avcodec_parameters_from_context(audioStream->codecpar, codecContext) < 0) {
        qCritical() << "Failed to copy codec parameters to stream.";

        return;
    }

    // Open output file
    if (!(outputFormat->flags & AVFMT_NOFILE)) {
        if (avio_open(&formatContext->pb, mp3FileName.toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
            qCritical() << "Failed to open output file.";

            return;
        }
    }

    if (avformat_write_header(formatContext, nullptr) < 0) {
        qCritical() << "Failed to write header.";

        return;
    }

    qDebug() << "Recording started.";

    // Record audio data
    connect(inputDevice, &QIODevice::readyRead, this, [=]() {
        data = inputDevice->readAll();
        if (data.isEmpty()) {
            qWarning() << "No data to save.";
            return;
        }

        AVFrame *frame = av_frame_alloc();
        frame->nb_samples = codecContext->frame_size;
        frame->format = codecContext->sample_fmt;
        av_channel_layout_default(&frame->ch_layout, codecContext->ch_layout.nb_channels);

        if (avcodec_fill_audio_frame(frame, codecContext->ch_layout.nb_channels, codecContext->sample_fmt,
                                     reinterpret_cast<const uint8_t*>(data.data()),
                                     data.size(), 0) < 0) {
            qWarning() << "Failed to fill audio frame.";
            av_frame_free(&frame);
            return;
        }

        if (avcodec_send_frame(codecContext, frame) < 0) {
            qWarning() << "Failed to send frame.";
            av_frame_free(&frame);
            return;
        }

        AVPacket *packet = av_packet_alloc();
        if (avcodec_receive_packet(codecContext, packet) == 0) {
            av_write_frame(formatContext, packet);
            av_packet_unref(packet);
        }
        av_packet_free(&packet);
        av_frame_free(&frame);
    });
}

void MainWindow::on_stopRecord_clicked()
{
    // FFmpeg işlemini sonlandırma
    if (formatContext) {
        av_write_trailer(formatContext);

        if (!(outputFormat->flags & AVFMT_NOFILE)) {
            avio_closep(&formatContext->pb);
        }

        avcodec_free_context(&codecContext);
        avformat_free_context(formatContext);
        qDebug() << "Recording stopped.";
    }

    data.clear();

    disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
}

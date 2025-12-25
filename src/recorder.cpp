#include "recorder.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <QFileDialog>
#include <QTimer>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

AVFormatContext* g_formatContext = nullptr;
AVCodecContext* g_codecContext = nullptr;
AVStream* g_audioStream = nullptr;
int64_t g_pts = 0;
qint64 g_recordStartTime = 0;

recorder::recorder(QWidget *parent)
    : QMainWindow{parent}
{

}

void MainWindow::on_startRecord_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save WAV File", "", "WAV Files (*.wav)");
    if (fileName.isEmpty()) return;

    if (!fileName.endsWith(".wav", Qt::CaseInsensitive)) {
        fileName += ".wav";
    }

    avformat_network_init();

    if (avformat_alloc_output_context2(&g_formatContext, nullptr, "wav", fileName.toStdString().c_str()) < 0) {
        qCritical() << "Failed to allocate format context";
        return;
    }

    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);
    if (!codec) {
        qCritical() << "Failed to find PCM S16LE codec";
        return;
    }

    g_codecContext = avcodec_alloc_context3(codec);
    if (!g_codecContext) {
        qCritical() << "Failed to allocate codec context";
        return;
    }

    g_codecContext->sample_fmt = AV_SAMPLE_FMT_S16;
    g_codecContext->sample_rate = 48000;
    av_channel_layout_default(&g_codecContext->ch_layout, 2);
    g_codecContext->bit_rate = 1536000;

    if (avcodec_open2(g_codecContext, codec, nullptr) < 0) {
        qCritical() << "Failed to open codec";
        return;
    }

    g_audioStream = avformat_new_stream(g_formatContext, nullptr);
    if (!g_audioStream) {
        qCritical() << "Failed to create audio stream";
        return;
    }

    if (avcodec_parameters_from_context(g_audioStream->codecpar, g_codecContext) < 0) {
        qCritical() << "Failed to copy codec parameters";
        return;
    }

    g_audioStream->time_base = {1, g_codecContext->sample_rate};

    if (!(g_formatContext->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&g_formatContext->pb, fileName.toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
            qCritical() << "Failed to open output file";
            return;
        }
    }

    if (avformat_write_header(g_formatContext, nullptr) < 0) {
        qCritical() << "Failed to write header";
        return;
    }

    g_pts = 0;
    g_recordStartTime = QDateTime::currentMSecsSinceEpoch();
    isRecording = true;

    // Timer to capture mixed audio from AudioPipeline
    QTimer* recordTimer = new QTimer(this);
    connect(recordTimer, &QTimer::timeout, this, [this]() {
        if (!isRecording) return;
        
        // Get mixed audio from AudioPipeline (contains input+soundpack or effects+soundpack)
        if (audioPipeline) {
            QByteArray mixedData = audioPipeline->getMixedAudioData(4096); // Get up to 4KB of mixed audio
            if (!mixedData.isEmpty()) {
                QByteArray data = mixedData;
                
                int samplesPerChannel = data.size() / sizeof(int16_t) / 2;
                int nb_samples = samplesPerChannel;

                AVFrame* frame = av_frame_alloc();
                frame->nb_samples = nb_samples;
                frame->format = AV_SAMPLE_FMT_S16;
                frame->sample_rate = 48000;
                av_channel_layout_default(&frame->ch_layout, 2);

                if (av_frame_get_buffer(frame, 0) < 0) {
                    qDebug() << "ERROR: Failed to get frame buffer";
                    av_frame_free(&frame);
                    return;
                }

                av_frame_make_writable(frame);
                memcpy(frame->data[0], data.constData(), data.size());

                frame->pts = g_pts;
                g_pts += nb_samples;

                if (avcodec_send_frame(g_codecContext, frame) < 0) {
                    qDebug() << "ERROR: Failed to send frame";
                    av_frame_free(&frame);
                    return;
                }

                AVPacket* packet = av_packet_alloc();
                while (avcodec_receive_packet(g_codecContext, packet) == 0) {
                    packet->stream_index = g_audioStream->index;
                    av_write_frame(g_formatContext, packet);
                    av_packet_unref(packet);
                }

                av_frame_free(&frame);
                av_packet_free(&packet);

                static int frameCount = 0;
                if (++frameCount % 50 == 0) {
                    qDebug() << "Recording frames:" << frameCount << "Time:" << (QDateTime::currentMSecsSinceEpoch() - g_recordStartTime) << "ms" << "Samples:" << nb_samples << "Size:" << data.size() << "bytes (MIXED AUDIO)";
                }
            }
        }
    });

    recordTimer->start(20); // Capture every 20ms

    qDebug() << "Recording started - capturing mixed audio from AudioPipeline (input+soundpack or effects+soundpack)";

    ui->startRecord->setText("Recording...");
    ui->startRecord->setEnabled(false);

    qDebug() << "Recording started to:" << fileName;
}

void MainWindow::on_stopRecord_clicked()
{
    if (!isRecording) return;

    // Find and stop the record timer
    QList<QTimer*> timers = findChildren<QTimer*>();
    for (QTimer* timer : timers) {
        if (timer->objectName().startsWith("recordTimer") || timer->interval() == 20) {
            timer->stop();
            timer->deleteLater();
            break;
        }
    }

    if (g_codecContext) {
        avcodec_send_frame(g_codecContext, nullptr);

        AVPacket* packet = av_packet_alloc();
        while (avcodec_receive_packet(g_codecContext, packet) == 0) {
            packet->stream_index = g_audioStream->index;
            av_write_frame(g_formatContext, packet);
            av_packet_unref(packet);
        }
        av_packet_free(&packet);
    }

    if (g_formatContext) {
        av_write_trailer(g_formatContext);

        if (!(g_formatContext->oformat->flags & AVFMT_NOFILE) && g_formatContext->pb) {
            avio_closep(&g_formatContext->pb);
        }
    }

    if (g_codecContext) {
        avcodec_free_context(&g_codecContext);
        g_codecContext = nullptr;
    }

    if (g_formatContext) {
        avformat_free_context(g_formatContext);
        g_formatContext = nullptr;
    }

    qint64 recordingDuration = QDateTime::currentMSecsSinceEpoch() - g_recordStartTime;
    isRecording = false;

    ui->startRecord->setText("Start Record");
    ui->startRecord->setEnabled(true);

    qDebug() << "Recording stopped. Duration:" << recordingDuration << "ms";
}

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
    
    // Recording buffer'ını temizle
    recordingBuffer.clear();
    
    // AudioPipeline recording state'ini güncelle
    if (audioPipeline) {
        audioPipeline->setRecordingState(true);
    }

    // AudioPipeline sinyalini doğrudan recording'e bağla - timer kullanma
    connect(this, &MainWindow::audioDataReady, this, [=](const QByteArray &audioData) {
        if (!isRecording || audioData.isEmpty()) return;
        
        // Recording buffer'ına biriktir - daha az işlem için
        recordingBuffer.append(audioData);
        
        // Buffer 8KB'a ulaşınca recording'e gönder
        if (recordingBuffer.size() >= 8192) {
            processAudioForRecording(recordingBuffer);
            recordingBuffer.clear();
        }
    });

    qDebug() << "Recording started - capturing mixed audio from AudioPipeline (input+soundpack or effects+soundpack)";

    ui->recordingButton->setText("Recording...");
    ui->recordingButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #e74c3c;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px;"
        "    border-radius: 4px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #c0392b;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #a93226;"
        "}"
    );
    ui->recordingButton->setEnabled(true);

    qDebug() << "Recording started to:" << fileName;
}

void MainWindow::on_stopRecord_clicked()
{
    if (!isRecording) return;

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
    
    // Kalan recording buffer'ını işle
    if (!recordingBuffer.isEmpty()) {
        processAudioForRecording(recordingBuffer);
        recordingBuffer.clear();
    }
    
    // AudioPipeline recording state'ini güncelle
    if (audioPipeline) {
        audioPipeline->setRecordingState(false);
    }

    ui->recordingButton->setText("Start Record");
    ui->recordingButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #27ae60;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px;"
        "    border-radius: 4px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #229954;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1e8449;"
        "}"
    );
    ui->recordingButton->setEnabled(true);

    qDebug() << "Recording stopped. Duration:" << recordingDuration << "ms";
}

void MainWindow::processAudioForRecording(const QByteArray &audioData)
{
    if (!isRecording || audioData.isEmpty()) return;
    
    int samplesPerChannel = audioData.size() / sizeof(int16_t) / 2;
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
    memcpy(frame->data[0], audioData.constData(), audioData.size());

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
        qDebug() << "Recording frames:" << frameCount << "Time:" 
                 << (QDateTime::currentMSecsSinceEpoch() - g_recordStartTime) 
                 << "ms" << "Samples:" << nb_samples << "Size:" 
                 << audioData.size() << "bytes (EFFECT AUDIO)";
    }
}

void MainWindow::setupEffectConnection(const QString &effectName, std::function<void(QByteArray&)> effectProcessor)
{
    if(!audioInput) return;
    
    // Always disconnect previous connection
    disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);

    connect(inputDevice, &QIODevice::readyRead, this, [=](){
        data = inputDevice->readAll();
        
        // Apply the effect
        effectProcessor(data);
        progressBarOutput();
        
        // Emit signal for recording when recording is active
        if (isRecording) {
            qDebug() << "EMITTING SIGNAL (" << effectName << " EFFECT): Audio size:" << data.size() << "bytes";
            emit audioDataReady(data);
        }

        // Send to virtual output for mixing
        if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
            if (audioPipeline) {
                audioPipeline->writeEffectsAudio(data);
                audioPipeline->processBuffers();
            } else {
                virtualOutputDevice->write(data);
            }
        }
        
        // Test modunda output routing AudioPipeline tarafından yönetiliyor
        // AudioPipeline test moduna göre normal output'a otomatik olarak yazar
    });
}

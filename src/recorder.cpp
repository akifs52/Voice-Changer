#include "recorder.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <QFileDialog>
#include <QTimer>
#include <QDateTime>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QEasingCurve>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

// Global recording variables
AVFormatContext* g_formatContext = nullptr;
AVCodecContext* g_codecContext = nullptr;
AVStream* g_audioStream = nullptr;
int64_t g_pts = 0;
qint64 g_recordStartTime = 0;

// Recording animation variables (moved from mainwindow.cpp)
QPropertyAnimation *recordingPulseAnimation = nullptr;
QGraphicsDropShadowEffect *recordingGlowEffect = nullptr;
QTimer *recordingPulseTimer = nullptr;

recorder::recorder(QWidget *parent)
    : QMainWindow{parent}
{
    // Constructor - recording functionality is now handled by MainWindow::on_recordingButton_clicked()
}

// Recording animation setup (moved from mainwindow.cpp)
void MainWindow::setupRecordingAnimations()
{
    // Create pulse animation for opacity
    recordingPulseAnimation = new QPropertyAnimation(this);
    recordingPulseAnimation->setDuration(800);
    recordingPulseAnimation->setStartValue(1.0);
    recordingPulseAnimation->setEndValue(0.3);
    recordingPulseAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    recordingPulseAnimation->setLoopCount(-1); // Infinite loop
    
    // Create red glow effect
    recordingGlowEffect = new QGraphicsDropShadowEffect(ui->recordingButton);
    recordingGlowEffect->setColor(QColor(231, 76, 60)); // Red color
    recordingGlowEffect->setBlurRadius(20);
    recordingGlowEffect->setOffset(0, 0);
    ui->recordingButton->setGraphicsEffect(recordingGlowEffect);
    
    // Initially hide the glow
    recordingGlowEffect->setEnabled(false);
    
    // Create pulse timer for glow effect
    recordingPulseTimer = new QTimer(this);
    connect(recordingPulseTimer, &QTimer::timeout, this, &MainWindow::updateRecordingPulse);
}

void MainWindow::startRecordingAnimation()
{
    if (recordingPulseAnimation && recordingGlowEffect) {
        // Set recording state property
        ui->recordingButton->setProperty("recording", true);
        ui->recordingButton->style()->unpolish(ui->recordingButton);
        ui->recordingButton->style()->polish(ui->recordingButton);
        
        // Start pulse animation
        recordingPulseAnimation->setTargetObject(ui->recordingButton);
        recordingPulseAnimation->setPropertyName("opacity");
        recordingPulseAnimation->start();
        
        // Enable and start glow pulse
        recordingGlowEffect->setEnabled(true);
        recordingPulseTimer->start(400); // Pulse every 400ms
    }
}

void MainWindow::stopRecordingAnimation()
{
    if (recordingPulseAnimation && recordingGlowEffect) {
        // Remove recording state property
        ui->recordingButton->setProperty("recording", false);
        ui->recordingButton->style()->unpolish(ui->recordingButton);
        ui->recordingButton->style()->polish(ui->recordingButton);
        
        // Stop pulse animation
        recordingPulseAnimation->stop();
        if (ui->recordingButton->graphicsEffect()) {
            ui->recordingButton->graphicsEffect()->setEnabled(false);
        }
        
        // Stop glow pulse
        recordingPulseTimer->stop();
        recordingGlowEffect->setEnabled(false);
    }
}

void MainWindow::updateRecordingPulse()
{
    if (recordingGlowEffect && recordingGlowEffect->isEnabled()) {
        // Create pulsing glow effect
        static bool increasing = true;
        static qreal currentRadius = 15;
        
        if (increasing) {
            currentRadius += 2;
            if (currentRadius >= 25) {
                increasing = false;
            }
        } else {
            currentRadius -= 2;
            if (currentRadius <= 15) {
                increasing = true;
            }
        }
        
        recordingGlowEffect->setBlurRadius(currentRadius);
    }
}

// Start recording function - called when recording button is toggled ON
void MainWindow::startRecording()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save WAV File", "", "WAV Files (*.wav)");
    if (fileName.isEmpty()) {
        // If user cancels, uncheck the button
        ui->recordingButton->setChecked(false);
        return;
    }

    if (!fileName.endsWith(".wav", Qt::CaseInsensitive)) {
        fileName += ".wav";
    }

    avformat_network_init();

    if (avformat_alloc_output_context2(&g_formatContext, nullptr, "wav", fileName.toStdString().c_str()) < 0) {
        qCritical() << "Failed to allocate format context";
        ui->recordingButton->setChecked(false);
        return;
    }

    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);
    if (!codec) {
        qCritical() << "Failed to find PCM S16LE codec";
        ui->recordingButton->setChecked(false);
        return;
    }

    g_codecContext = avcodec_alloc_context3(codec);
    if (!g_codecContext) {
        qCritical() << "Failed to allocate codec context";
        ui->recordingButton->setChecked(false);
        return;
    }

    g_codecContext->codec_type = AVMEDIA_TYPE_AUDIO;
    g_codecContext->sample_fmt = AV_SAMPLE_FMT_S16;
    g_codecContext->sample_rate = format->sampleRate();
    g_codecContext->ch_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
    g_codecContext->bit_rate = 192000;

    if (avcodec_open2(g_codecContext, codec, nullptr) < 0) {
        qCritical() << "Failed to open codec";
        ui->recordingButton->setChecked(false);
        return;
    }

    g_audioStream = avformat_new_stream(g_formatContext, codec);
    if (!g_audioStream) {
        qCritical() << "Failed to create audio stream";
        ui->recordingButton->setChecked(false);
        return;
    }

    avcodec_parameters_from_context(g_audioStream->codecpar, g_codecContext);

    if (!(g_formatContext->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&g_formatContext->pb, fileName.toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
            qCritical() << "Failed to open output file";
            ui->recordingButton->setChecked(false);
            return;
        }
    }

    if (avformat_write_header(g_formatContext, nullptr) < 0) {
        qCritical() << "Failed to write header";
        ui->recordingButton->setChecked(false);
        return;
    }

    isRecording = true;
    recordingStartTime = QDateTime::currentMSecsSinceEpoch();
    totalRecordedBytes = 0;
    recordingBuffer.clear();
    g_recordStartTime = QDateTime::currentMSecsSinceEpoch();

    // Update AudioPipeline recording state
    if (audioPipeline) {
        audioPipeline->setRecordingState(true);
    }
    
    // Start recording animation
    startRecordingAnimation();

    qDebug() << "Recording started to:" << fileName;
}

// Stop recording function - called when recording button is toggled OFF
void MainWindow::stopRecording()
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
    
    // Process remaining recording buffer
    if (!recordingBuffer.isEmpty()) {
        processAudioForRecording(recordingBuffer);
        recordingBuffer.clear();
    }
    
    // Update AudioPipeline recording state
    if (audioPipeline) {
        audioPipeline->setRecordingState(false);
    }
    
    // Stop recording animation
    stopRecordingAnimation();

    qDebug() << "Recording stopped. Duration:" << recordingDuration << "ms";
}

// Process audio data for recording
void MainWindow::processAudioForRecording(const QByteArray &audioData)
{
    if (!isRecording || audioData.isEmpty()) return;
    
    int samplesPerChannel = audioData.size() / sizeof(int16_t) / 2;
    int nb_samples = samplesPerChannel;

    AVFrame* frame = av_frame_alloc();
    frame->nb_samples = nb_samples;
    frame->format = AV_SAMPLE_FMT_S16;
    frame->ch_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
    frame->sample_rate = format->sampleRate();
    frame->pts = g_pts;

    if (av_frame_get_buffer(frame, 0) < 0) {
        qCritical() << "Failed to get frame buffer";
        av_frame_free(&frame);
        return;
    }

    // Copy audio data to frame
    int16_t* frameData = (int16_t*)frame->data[0];
    const int16_t* audioDataPtr = (const int16_t*)audioData.constData();
    
    for (int i = 0; i < nb_samples * 2; i++) {
        frameData[i] = audioDataPtr[i];
    }

    if (avcodec_send_frame(g_codecContext, frame) == 0) {
        AVPacket* packet = av_packet_alloc();
        while (avcodec_receive_packet(g_codecContext, packet) == 0) {
            av_packet_rescale_ts(packet, g_codecContext->time_base, g_audioStream->time_base);
            packet->stream_index = g_audioStream->index;
            av_write_frame(g_formatContext, packet);
            av_packet_unref(packet);
        }
        av_packet_free(&packet);
    }

    g_pts += nb_samples;
    av_frame_free(&frame);

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

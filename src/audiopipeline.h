#ifndef AUDIOPIPELINE_H
#define AUDIOPIPELINE_H

#include <QObject>
#include <QIODevice>
#include <QByteArray>
#include <QTimer>
#include "circularbuffer.h"

class AudioPipeline : public QObject
{
    Q_OBJECT

public:
    explicit AudioPipeline(QObject *parent = nullptr);
    ~AudioPipeline();

    // Set virtual output device
    void setVirtualOutputDevice(QIODevice *device);

    // Write audio to respective buffers
    void writeEffectsAudio(const QByteArray &data);
    void writeSoundpackAudio(const QByteArray &data);

    // Control pipeline
    void start();
    void stop();

    // Buffer management
    void clearBuffers();
    void clearSoundpackBuffer(); // SoundPack buffer temizliği için
    void printStatus() const;

    // Buffer status queries
    int getEffectsBufferBytesAvailable() const;
    int getSoundpackBufferBytesAvailable() const;

private slots:
    void processBuffers();

private:
    CircularBuffer *m_effectsBuffer;     // Efektli ses için buffer
    CircularBuffer *m_soundpackBuffer;  // Soundpack sesleri için buffer
    CircularBuffer *m_mixBuffer; // Özel mix buffer'ı
    QIODevice *m_virtualOutputDevice;    // Virtual output device
    QTimer *m_timer;                     // Buffer processing timer

    int m_chunkSize;                     // Buffer chunk size (normal streams)
    int m_mixChunkSize;                  // Mix işlemleri için özel chunk size
    bool m_isRunning;                    // Pipeline status

    // Audio mixing helper
    QByteArray mixAudioData(const QByteArray &data1, const QByteArray &data2);
};

#endif // AUDIOPIPELINE_H

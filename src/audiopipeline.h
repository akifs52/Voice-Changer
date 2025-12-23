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
    void writeInputAudio(const QByteArray &data);
    void writeSoundpackAudio(const QByteArray &data);
    void writeEffectsAudio(const QByteArray &data);  // Efektli ses için özel metod

    // Control pipeline
    void start();
    void stop();

    // Buffer management
    void clearBuffers();
    void clearSoundpackBuffer(); // SoundPack buffer temizliği için
    void printStatus() const;

    void processBuffers();

    // Buffer status queries
    int getInputBufferBytesAvailable() const;
    int getSoundpackBufferBytesAvailable() const;
    int getEffectsBufferBytesAvailable() const;  // Efektli ses buffer durumu
    int getMixBufferBytesAvailable() const;

    // Get mixed audio data for recording
    QByteArray getMixedAudioData(int maxSize);

private slots:


private:
// Separate buffers for different audio types
    CircularBuffer *m_inputAudioBuffer;  // Input sesleri için buffer
    CircularBuffer *m_soundpackBuffer;  // Soundpack sesleri için buffer
    CircularBuffer *m_effectsAudioBuffer; // Efektli sesler için özel buffer
    CircularBuffer *m_mixAudioBuffer;    // Mix edilmiş sesler için buffer
    QIODevice *m_virtualOutputDevice;     // Virtual output device
    QTimer *m_timer;                      // Buffer processing timer

    // Separate chunk sizes for each buffer type
    int m_inputChunkSize;                // Input audio için chunk size
    int m_soundpackChunkSize;            // Soundpack için chunk size
    int m_effectsChunkSize;              // Efektli sesler için özel chunk size
    int m_mixChunkSize;                  // Mix işlemleri için özel chunk size
    bool m_isRunning;                     // Pipeline status

    // Audio mixing helper
    QByteArray mixAudioData(const QByteArray &data1, const QByteArray &data2);
};

#endif // AUDIOPIPELINE_H

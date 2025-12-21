#include "audiopipeline.h"
#include "mainwindow.h"
#include "QDebug"
#include "QTimer"

AudioPipeline::AudioPipeline(QObject *parent)
    : QObject(parent)
    , m_inputAudioBuffer(nullptr)
    , m_soundpackBuffer(nullptr)
    , m_mixAudioBuffer(nullptr)
    , m_virtualOutputDevice(nullptr)
    , m_inputChunkSize(1024) // 256 samples at 48kHz stereo - optimal balance
    , m_soundpackChunkSize(1024) // 256 samples for soundpack - optimal balance
    , m_mixChunkSize(2048) // 512 samples for mix - daha hassas processing
    , m_isRunning(false)
{
    m_inputAudioBuffer = new CircularBuffer(65536);  // 64KB buffer for input audio
    m_soundpackBuffer = new CircularBuffer(65536); // 64KB buffer for soundpack
    m_mixAudioBuffer = new CircularBuffer(65536); // 64KB buffer for mixed audio
    
    // Timer for periodic buffer processing - 2ms for optimal balance
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &AudioPipeline::processBuffers);
    
    // Thread önceliğini artır - UI thread'i ile çakışmayı önle
    m_timer->setTimerType(Qt::PreciseTimer);
}

AudioPipeline::~AudioPipeline()
{
    stop();
    delete m_inputAudioBuffer;
    delete m_soundpackBuffer;
    delete m_mixAudioBuffer;
}

void AudioPipeline::setVirtualOutputDevice(QIODevice *device)
{
    m_virtualOutputDevice = device;
}

void AudioPipeline::writeInputAudio(const QByteArray &data)
{
    if (m_inputAudioBuffer) {
        // UI thread'i ile çakışmayı önlemek için lock-free yazım
        m_inputAudioBuffer->write(data);
        // Debug mesajlarını azalt - her 500 yazmada bir göster
        static int counter = 0;
        if (++counter % 500 == 0) { // Her 500 yazmada bir göster
            qDebug() << "Input Audio Buffer:" << m_inputAudioBuffer->bytesAvailable() << "bytes";
        }
    }
}

void AudioPipeline::writeSoundpackAudio(const QByteArray &data)
{
    if (m_soundpackBuffer) {
        // UI thread'i ile çakışmayı önlemek için lock-free yazım
        m_soundpackBuffer->write(data);
        // Debug mesajlarını azalt - her 500 yazmada bir göster
        static int counter = 0;
        if (++counter % 500 == 0) { // Her 500 yazmada bir göster
            qDebug() << "Soundpack Buffer:" << m_soundpackBuffer->bytesAvailable() << "bytes";
        }
    }
}

void AudioPipeline::start()
{
    if (!m_isRunning) {
        m_isRunning = true;
        // Process buffers every 2ms for optimal balance
        m_timer->start(2);
        qDebug() << "AudioPipeline started with 2ms interval (256 samples)";
        
        // Buffer durumunu göster
        printStatus();
    }
}

void AudioPipeline::stop()
{
    if (m_isRunning) {
        m_isRunning = false;
        m_timer->stop();
        qDebug() << "AudioPipeline stopped";
    }
}

void AudioPipeline::processBuffers()
{
    if (!m_virtualOutputDevice || !m_virtualOutputDevice->isOpen()) {
        return;
    }
    
    // ÖNCELİK 1: Mix buffer'ındaki verileri output'a gönder (BUFFER OVERFLOW ÖNLE)
    if (m_mixAudioBuffer->bytesAvailable() >= m_mixChunkSize) {
        QByteArray mixedData = m_mixAudioBuffer->read(m_mixChunkSize);
        m_virtualOutputDevice->write(mixedData);
        qDebug() << "OUTPUT: Sent" << mixedData.size() << "bytes from mix buffer";
        return; // Mix buffer'dan gönderdik, diğer buffer'ları bekle
    }
    
    // TEST: Mix buffer durumunu göster
    static int testCounter = 0;
    if (++testCounter % 1000 == 0) { // Her 1000 çağrıda bir göster
        qDebug() << "=== MIX BUFFER TEST ===";
        qDebug() << "Mix Buffer Available:" << (m_mixAudioBuffer ? m_mixAudioBuffer->bytesAvailable() : 0) << "bytes";
        qDebug() << "Input Audio Buffer Available:" << getInputBufferBytesAvailable() << "bytes";
        qDebug() << "Soundpack Buffer Available:" << getSoundpackBufferBytesAvailable() << "bytes";
        qDebug() << "Mix Chunk Size:" << m_mixChunkSize << "bytes";
        qDebug() << "Input Chunk Size:" << m_inputChunkSize << "bytes";
        qDebug() << "Soundpack Chunk Size:" << m_soundpackChunkSize << "bytes";
        
        // DEBUG: Mix koşullarını göster
        bool hasInput = m_inputAudioBuffer->bytesAvailable() >= m_mixChunkSize;
        bool hasSoundpack = m_soundpackBuffer->bytesAvailable() >= m_mixChunkSize;
        qDebug() << "Has Input (>=2048):" << hasInput;
        qDebug() << "Has Soundpack (>=2048):" << hasSoundpack;
        qDebug() << "Should Mix:" << (hasInput && hasSoundpack);
    }
    
    // ÖNCELİK 2: Buffer durumlarını kontrol et
    bool hasInput = m_inputAudioBuffer->bytesAvailable() >= m_mixChunkSize;
    bool hasSoundpack = m_soundpackBuffer->bytesAvailable() >= m_mixChunkSize;
    
    if (hasInput && hasSoundpack) {
        // İkisi de aynı andaysa mix yap ve mix buffer'ına yaz
        qDebug() << "MIXING: Both buffers have enough data!";
        QByteArray inputData = m_inputAudioBuffer->read(m_mixChunkSize);
        QByteArray soundpackData = m_soundpackBuffer->read(m_mixChunkSize);
        
        // Normalize buffer sizes (güvenlik için)
        if (inputData.size() != soundpackData.size()) {
            int minSize = std::min(inputData.size(), soundpackData.size());
            if (minSize > 0) {
                inputData.resize(minSize);
                soundpackData.resize(minSize);
            } else {
                return; // Geçersiz boyut
            }
        }
        
        // Mix yap ve özel buffer'a yaz
        QByteArray mixedData = mixAudioData(inputData, soundpackData);
        m_mixAudioBuffer->write(mixedData);
        qDebug() << "MIXED: Wrote" << mixedData.size() << "bytes to mix buffer";
        
    } else if (hasInput && !hasSoundpack) {
        // Sadece input varsa doğrudan output'a yaz
        QByteArray inputData = m_inputAudioBuffer->read(m_inputChunkSize);
        m_virtualOutputDevice->write(inputData);
        qDebug() << "OUTPUT: Sent" << inputData.size() << "bytes from input buffer";
        
    } else if (!hasInput && hasSoundpack) {
        // Sadece soundpack varsa doğrudan output'a yaz
        QByteArray soundpackData = m_soundpackBuffer->read(m_soundpackChunkSize);
        m_virtualOutputDevice->write(soundpackData);
        qDebug() << "OUTPUT: Sent" << soundpackData.size() << "bytes from soundpack buffer";
    }
    // Hiçbiri yoksa hiçbir şey yapma
}

void AudioPipeline::clearBuffers()
{
    if (m_inputAudioBuffer) {
        m_inputAudioBuffer->clear();
    }
    if (m_soundpackBuffer) {
        m_soundpackBuffer->clear();
    }
    if (m_mixAudioBuffer) {
        m_mixAudioBuffer->clear();
    }
    qDebug() << "AudioPipeline buffers cleared";
}

void AudioPipeline::clearSoundpackBuffer()
{
    if (m_soundpackBuffer) {
        m_soundpackBuffer->clear();
    }
    if (m_mixAudioBuffer) {
        m_mixAudioBuffer->clear(); // SoundPack değiştiğinde mix buffer'ını da temizle
    }
    qDebug() << "AudioPipeline soundpack buffer cleared";
}

int AudioPipeline::getInputBufferBytesAvailable() const
{
    if (m_inputAudioBuffer) {
        return m_inputAudioBuffer->bytesAvailable();
    }
    return 0;
}

int AudioPipeline::getSoundpackBufferBytesAvailable() const
{
    if (m_soundpackBuffer) {
        return m_soundpackBuffer->bytesAvailable();
    }
    return 0;
}

int AudioPipeline::getMixBufferBytesAvailable() const
{
    if (m_mixAudioBuffer) {
        return m_mixAudioBuffer->bytesAvailable();
    }
    return 0;
}

void AudioPipeline::printStatus() const
{
    qDebug() << "AudioPipeline Status:";
    qDebug() << "  Input Audio Buffer Available:" << getInputBufferBytesAvailable() << "bytes";
    qDebug() << "  Soundpack Buffer Available:" << getSoundpackBufferBytesAvailable() << "bytes";
    qDebug() << "  Mix Buffer Available:" << (m_mixAudioBuffer ? m_mixAudioBuffer->bytesAvailable() : 0) << "bytes";
    qDebug() << "  Input Chunk Size:" << m_inputChunkSize << "bytes (" << (m_inputChunkSize/4) << " samples)";
    qDebug() << "  Soundpack Chunk Size:" << m_soundpackChunkSize << "bytes (" << (m_soundpackChunkSize/4) << " samples)";
    qDebug() << "  Mix Chunk Size:" << m_mixChunkSize << "bytes (" << (m_mixChunkSize/4) << " samples)";
    qDebug() << "  Is Running:" << m_isRunning;
}

QByteArray AudioPipeline::mixAudioData(const QByteArray &data1, const QByteArray &data2)
{
    // 16-bit stereo PCM varsayımı (2 byte per sample, 2 channels = 4 bytes per frame)
    QByteArray mixedData;
    mixedData.resize(data1.size());
    
    const int16_t *samples1 = reinterpret_cast<const int16_t*>(data1.constData());
    const int16_t *samples2 = reinterpret_cast<const int16_t*>(data2.constData());
    int16_t *mixedSamples = reinterpret_cast<int16_t*>(mixedData.data());
    
    int sampleCount = data1.size() / sizeof(int16_t);
    
    // Hassas ses karıştırma parametreleri - kalite için optimize
    const float gain1 = 0.8f;    // Birinci sesin kazancı (%80)
    const float gain2 = 0.8f;    // İkinci sesin kazancı (%80)
    const float masterGain = 0.95f; // Master kazanç (%95)
    const int32_t maxSampleValue = 32767 * 0.98f; // %2 headroom bırak
    
    // Daha hassas gürültü kontrolü
    const int32_t noiseThreshold = 100; // Daha düşük gürültü eşiği
    
    for (int i = 0; i < sampleCount; ++i) {
        // Hassas gürültü kontrolü
        if (abs(samples1[i]) < noiseThreshold && abs(samples2[i]) < noiseThreshold) {
            mixedSamples[i] = 0;
            continue;
        }
        
        // 1. Her iki sinyali float'a çevir ve kazanç uygula
        float sample1_f = static_cast<float>(samples1[i]) * gain1;
        float sample2_f = static_cast<float>(samples2[i]) * gain2;
        
        // 2. Sesleri topla
        float mixed_f = sample1_f + sample2_f;
        
        // 3. Master kazancı uygula
        mixed_f *= masterGain;
        
        // 4. Hassas sınırlama - Soft clipping yerine daha yumuşak yaklaşım
        if (mixed_f > maxSampleValue) {
            // Daha yumuşak clipping - küçük azaltma
            mixed_f = maxSampleValue + (mixed_f - maxSampleValue) * 0.1f;
        } else if (mixed_f < -maxSampleValue) {
            // Daha yumuşak clipping - küçük azaltma
            mixed_f = -maxSampleValue - (-maxSampleValue - mixed_f) * 0.1f;
        }
        
        // 5. Geri int16'ya çevir - daha hassas rounding
        int32_t mixed_int = static_cast<int32_t>(mixed_f);
        
        // 6. Son sınırlama (güvenlik için)
        if (mixed_int > 32767) mixed_int = 32767;
        if (mixed_int < -32768) mixed_int = -32768;
        
        mixedSamples[i] = static_cast<int16_t>(mixed_int);
    }
    
    return mixedData;
}

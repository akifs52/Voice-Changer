#include "audiopipeline.h"
#include "mainwindow.h"
#include <QDebug>
#include <QTimer>

AudioPipeline::AudioPipeline(QObject *parent)
    : QObject(parent)
    , m_inputAudioBuffer(nullptr)
    , m_soundpackBuffer(nullptr)
    , m_effectsAudioBuffer(nullptr)
    , m_mixAudioBuffer(nullptr)
    , m_virtualOutputDevice(nullptr)
    , m_inputChunkSize(1024) // 256 samples at 48kHz stereo - optimal balance
    , m_soundpackChunkSize(1024) // 256 samples for soundpack - optimal balance
    , m_effectsChunkSize(1024) // 256 samples for effects - optimal balance
    , m_mixChunkSize(1024) // 512 samples for mix - daha hassas processing
    , m_isRunning(false)
{
m_inputAudioBuffer = new CircularBuffer(65536);  // 64KB buffer for input audio
    m_soundpackBuffer = new CircularBuffer(65536); // 64KB buffer for soundpack
    m_effectsAudioBuffer = new CircularBuffer(65536); // 64KB buffer for effects
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
    delete m_effectsAudioBuffer;
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

void AudioPipeline::writeEffectsAudio(const QByteArray &data)
{
    if (m_effectsAudioBuffer) {
        // UI thread'i ile çakışmayı önlemek için lock-free yazım
        m_effectsAudioBuffer->write(data);
        // Debug mesajlarını azalt - her 500 yazmada bir göster
        static int counter = 0;
        if (++counter % 100 == 0) { // Her 100'de bir göster (daha sık)
            qDebug() << "Effects Buffer:" << m_effectsAudioBuffer->bytesAvailable() << "bytes";
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
    
    // ÖNCELİKLİ: Efektli sesi işle (daha yüksek öncelik)
    if (m_effectsAudioBuffer->bytesAvailable() >= m_effectsChunkSize) {
        QByteArray effectsData = m_effectsAudioBuffer->read(m_effectsChunkSize);
        
        // Soundpack kontrolü - sadece ses varsa mix yap
        if (m_soundpackBuffer->bytesAvailable() >= m_soundpackChunkSize) {
            // Soundpack var - efektli ses ile karıştır
            QByteArray soundpackData = m_soundpackBuffer->read(m_soundpackChunkSize);
            
            // Boyutları normalize et
            if (effectsData.size() != soundpackData.size()) {
                soundpackData.resize(effectsData.size());
            }
            
            // Efektli ses + soundpack mix yap
            QByteArray mixedData = mixAudioData(effectsData, soundpackData);
            m_virtualOutputDevice->write(mixedData);
            
            // Debug mesajlarını azalt
            static int counter = 0;
            if (++counter % 100 == 0) { // Her 100'de bir göster (daha sık)
                qDebug() << "EFFECTS MIXED: Effects + Soundpack =" << effectsData.size() << "bytes";
            }
        } else {
            // Soundpack yok - efektli sesi doğrudan geçir
            m_virtualOutputDevice->write(effectsData);
            
            // Debug mesajlarını azalt
            static int counter = 0;
            if (++counter % 500 == 0) {
                qDebug() << "EFFECTS CLEAN PASS: No soundpack, passing through effects" << effectsData.size() << "bytes";
            }
        }
        return; // Efektli ses işlendiği için temiz sesi atla
    }
    
    // İKİNCİ ÖNCELİK: Temiz sesi işle (sadece efektli ses yoksa)
    if (m_inputAudioBuffer->bytesAvailable() >= m_inputChunkSize) {
        QByteArray inputData = m_inputAudioBuffer->read(m_inputChunkSize);
        
        // Soundpack kontrolü - sadece ses varsa mix yap
        if (m_soundpackBuffer->bytesAvailable() >= m_soundpackChunkSize) {
            // Soundpack var - temiz ses ile karıştır
            QByteArray soundpackData = m_soundpackBuffer->read(m_soundpackChunkSize);
            
            // Boyutları normalize et
            if (inputData.size() != soundpackData.size()) {
                soundpackData.resize(inputData.size());
            }
            
            // Temiz ses + soundpack mix yap
            QByteArray mixedData = mixAudioData(inputData, soundpackData);
            m_virtualOutputDevice->write(mixedData);
            
            // Debug mesajlarını azalt
            static int counter = 0;
            if (++counter % 500 == 0) {
                qDebug() << "CLEAN MIXED: Input + Soundpack =" << inputData.size() << "bytes";
            }
        } else {
            // Soundpack yok - temiz sesi doğrudan geçir
            m_virtualOutputDevice->write(inputData);
            
            // Debug mesajlarını azalt
            static int counter = 0;
            if (++counter % 500 == 0) {
                qDebug() << "CLEAN ONLY PASS: No soundpack, passing through clean" << inputData.size() << "bytes";
            }
        }
    }
}

void AudioPipeline::clearBuffers()
{
    if (m_inputAudioBuffer) {
        m_inputAudioBuffer->clear();
    }
    if (m_soundpackBuffer) {
        m_soundpackBuffer->clear();
    }
    if (m_effectsAudioBuffer) {
        m_effectsAudioBuffer->clear();
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

int AudioPipeline::getEffectsBufferBytesAvailable() const
{
    if (m_effectsAudioBuffer) {
        return m_effectsAudioBuffer->bytesAvailable();
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
    qDebug() << "  Effects Buffer Available:" << getEffectsBufferBytesAvailable() << "bytes";
    qDebug() << "  Soundpack Buffer Available:" << getSoundpackBufferBytesAvailable() << "bytes";
    qDebug() << "  Mix Buffer Available:" << (m_mixAudioBuffer ? m_mixAudioBuffer->bytesAvailable() : 0) << "bytes";
    qDebug() << "  Input Chunk Size:" << m_inputChunkSize << "bytes (" << (m_inputChunkSize/4) << " samples)";
    qDebug() << "  Effects Chunk Size:" << m_effectsChunkSize << "bytes (" << (m_effectsChunkSize/4) << " samples)";
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

QByteArray AudioPipeline::getMixedAudioData(int maxSize)
{
    // This method returns the current mixed audio output for recording
    // We'll capture what would normally go to virtualOutputDevice
    
    QByteArray mixedData;
    
    // Priority 1: Check if we have effects + soundpack mix
    if (m_effectsAudioBuffer->bytesAvailable() >= m_effectsChunkSize &&
        m_soundpackBuffer->bytesAvailable() >= m_soundpackChunkSize) {
        
        QByteArray effectsData = m_effectsAudioBuffer->read(m_effectsChunkSize);
        QByteArray soundpackData = m_soundpackBuffer->read(m_soundpackChunkSize);
        
        // Normalize sizes
        if (effectsData.size() != soundpackData.size()) {
            soundpackData.resize(effectsData.size());
        }
        
        mixedData = mixAudioData(effectsData, soundpackData);
    }
    // Priority 2: Check if we have input + soundpack mix
    else if (m_inputAudioBuffer->bytesAvailable() >= m_inputChunkSize &&
             m_soundpackBuffer->bytesAvailable() >= m_soundpackChunkSize) {
        
        QByteArray inputData = m_inputAudioBuffer->read(m_inputChunkSize);
        QByteArray soundpackData = m_soundpackBuffer->read(m_soundpackChunkSize);
        
        // Normalize sizes
        if (inputData.size() != soundpackData.size()) {
            soundpackData.resize(inputData.size());
        }
        
        mixedData = mixAudioData(inputData, soundpackData);
    }
    // Priority 3: Just effects (no soundpack)
    else if (m_effectsAudioBuffer->bytesAvailable() >= m_effectsChunkSize) {
        mixedData = m_effectsAudioBuffer->read(m_effectsChunkSize);
    }
    // Priority 4: Just clean input (no effects, no soundpack)
    else if (m_inputAudioBuffer->bytesAvailable() >= m_inputChunkSize) {
        mixedData = m_inputAudioBuffer->read(m_inputChunkSize);
    }
    
    // Limit the size to requested maxSize
    if (mixedData.size() > maxSize && maxSize > 0) {
        mixedData = mixedData.left(maxSize);
    }
    
    return mixedData;
}

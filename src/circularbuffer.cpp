#include "circularbuffer.h"
#include <QElapsedTimer>

CircularBuffer::CircularBuffer(int bufferSize)
    : m_head(0)
    , m_tail(0)
    , m_size(bufferSize)
    , m_count(0)
    , m_isClosed(false)
{
    m_buffer.resize(bufferSize);
}

CircularBuffer::~CircularBuffer()
{
    QMutexLocker locker(&m_mutex);
    m_isClosed = true;
    m_dataAvailable.wakeAll();
}

int CircularBuffer::write(const char *data, int maxSize)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isClosed) return 0;
    
    int bytesToWrite = qMin(maxSize, m_size - m_count);
    if (bytesToWrite == 0) return 0;
    
    // Buffer doluysa eski veriyi üzerine yaz (drop oldest)
    if (bytesToWrite < maxSize) {
        qDebug() << "Circular buffer full, dropping" << (maxSize - bytesToWrite) << "bytes";
    }
    
    // Veriyi buffer'a yaz - iki parça halinde yazabiliriz
    int firstPart = qMin(bytesToWrite, m_size - m_head);
    memcpy(m_buffer.data() + m_head, data, firstPart);
    
    if (firstPart < bytesToWrite) {
        // Buffer başına dönüp kalanı yaz
        memcpy(m_buffer.data(), data + firstPart, bytesToWrite - firstPart);
    }
    
    m_head = (m_head + bytesToWrite) % m_size;
    m_count = qMin(m_count + bytesToWrite, m_size);
    
    // Bekleyen consumer'ları uyandır
    if (m_count > 0) {
        m_dataAvailable.wakeAll();
    }
    
    return bytesToWrite;
}

int CircularBuffer::write(const QByteArray &data)
{
    return write(data.constData(), data.size());
}

int CircularBuffer::read(char *data, int maxSize)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_count == 0) return 0;
    
    int bytesToRead = qMin(maxSize, m_count);
    
    // Veriyi buffer'dan oku - iki parça halinde okuyabiliriz
    int firstPart = qMin(bytesToRead, m_size - m_tail);
    memcpy(data, m_buffer.constData() + m_tail, firstPart);
    
    if (firstPart < bytesToRead) {
        // Buffer başına dönüp kalanı oku
        memcpy(data + firstPart, m_buffer.constData(), bytesToRead - firstPart);
    }
    
    m_tail = (m_tail + bytesToRead) % m_size;
    m_count -= bytesToRead;
    
    return bytesToRead;
}

QByteArray CircularBuffer::read(int maxSize)
{
    QByteArray result;
    result.resize(maxSize);
    
    int bytesRead = read(result.data(), maxSize);
    result.resize(bytesRead);
    
    return result;
}

int CircularBuffer::bytesAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return m_count;
}

int CircularBuffer::bytesToWrite() const
{
    QMutexLocker locker(&m_mutex);
    return m_size - m_count;
}

bool CircularBuffer::isEmpty() const
{
    QMutexLocker locker(&m_mutex);
    return m_count == 0;
}

bool CircularBuffer::isFull() const
{
    QMutexLocker locker(&m_mutex);
    return m_count >= m_size;
}

void CircularBuffer::clear()
{
    QMutexLocker locker(&m_mutex);
    m_head = 0;
    m_tail = 0;
    m_count = 0;
}

void CircularBuffer::waitForData(int minBytes, int timeoutMs)
{
    QMutexLocker locker(&m_mutex);
    
    QElapsedTimer timer;
    timer.start();
    
    while (m_count < minBytes && timer.elapsed() < timeoutMs) {
        m_dataAvailable.wait(&m_mutex, 100); // 100ms timeout per wait
    }
}

void CircularBuffer::printStatus() const
{
    QMutexLocker locker(&m_mutex);
    qDebug() << "CircularBuffer Status:";
    qDebug() << "  Size:" << m_size << "bytes";
    qDebug() << "  Count:" << m_count << "bytes";
    qDebug() << "  Head:" << m_head;
    qDebug() << "  Tail:" << m_tail;
    qDebug() << "  Available:" << bytesAvailable() << "bytes";
    qDebug() << "  Space:" << bytesToWrite() << "bytes";
}

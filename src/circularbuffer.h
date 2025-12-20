#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>

class CircularBuffer
{
public:
    explicit CircularBuffer(int bufferSize = 8192);
    ~CircularBuffer();
    
    // Veri yazma - producer thread'ler için
    int write(const char *data, int maxSize);
    int write(const QByteArray &data);
    
    // Veri okuma - consumer thread'ler için  
    int read(char *data, int maxSize);
    QByteArray read(int maxSize);
    
    // Buffer durum kontrolü
    int bytesAvailable() const;
    int bytesToWrite() const;
    bool isEmpty() const;
    bool isFull() const;
    
    // Buffer yönetimi
    void clear();
    void waitForData(int minBytes = 1, int timeoutMs = 1000);
    
    // Debug
    void printStatus() const;

private:
    QByteArray m_buffer;
    mutable QMutex m_mutex;
    QWaitCondition m_dataAvailable;
    
    int m_head;      // Yazma pozisyonu
    int m_tail;      // Okuma pozisyonu
    int m_size;      // Buffer boyutu
    int m_count;     // Mevcut veri miktarı
    
    bool m_isClosed;
};

#endif // CIRCULARBUFFER_H

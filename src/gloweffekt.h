#ifndef GLOWEFFEKT_H
#define GLOWEFFEKT_H

#include "ui_mainwindow.h"
#include <QObject>
#include <QWidget>
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include <QEvent>
#include <QMap>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSet>
#include <QTimer>
#include <QVector>
#include <QDateTime>

template<typename T>
class GlowFrameBuffer
{
public:
    explicit GlowFrameBuffer(int size) : m_size(size), m_head(0), m_tail(0), m_count(0) {
        m_buffer.resize(size);
    }
    
    void push(const T &item) {
        m_buffer[m_head] = item;
        m_head = (m_head + 1) % m_size;
        if (m_count < m_size) {
            m_count++;
        } else {
            m_tail = (m_tail + 1) % m_size;
        }
    }
    
    bool peek(int index, T &item) const {
        if (index >= m_count) return false;
        int actualIndex = (m_tail + index) % m_size;
        item = m_buffer[actualIndex];
        return true;
    }
    
    bool isEmpty() const { return m_count == 0; }
    int size() const { return m_count; }
    void clear() { m_head = m_tail = m_count = 0; }
    
private:
    QVector<T> m_buffer;
    int m_size;
    int m_head;
    int m_tail;
    int m_count;
};

struct GlowFrame {
    int blurRadius;
    QColor color;
    qint64 timestamp;
};

class Ui_VoiceChangerMainWindow;

class GlowEffect : public QObject
{
    Q_OBJECT

public:
    explicit GlowEffect(Ui::VoiceChangerMainWindow *ui, QObject *parent = nullptr);
    virtual ~GlowEffect();

    // Setup glow effects for all widgets
    void setupGlowEffects();

    // Control individual widget glow
    void setGlowEnabled(QWidget *widget, bool enabled);
    void setGlowColor(QWidget *widget, const QColor &color);
    void setGlowBlurRadius(QWidget *widget, int radius);
    
    // Audio-safe performance mode
    void setLowPerformanceMode(bool enabled);
    
    // Circular buffer controls
    void setBufferSize(int size);
    void setFrameRate(int fps);

protected:
    // Event filter for hover effects
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onAnimationFinished();
    void processBufferedFrames();

private:
    void setupAnimatedGlow(QWidget *widget, const QColor &normalColor, int normalRadius);
    void startHoverAnimation(QWidget *widget);
    void startLeaveAnimation(QWidget *widget);
    void addToBuffer(QWidget *widget, int blurRadius, const QColor &color);

    Ui::VoiceChangerMainWindow *m_ui;
    QMap<QWidget*, QColor> m_originalGlowColors;
    QMap<QWidget*, int> m_originalGlowRadius;
    QMap<QWidget*, QPropertyAnimation*> m_colorAnimations; // Only color animations
    QMap<QWidget*, QParallelAnimationGroup*> m_animationGroups;
    QSet<QWidget*> m_hoveredWidgets; // Hover state tracking
    bool m_lowPerformanceMode;
    
    // Circular buffer system
    QMap<QWidget*, GlowFrameBuffer<GlowFrame>*> m_glowBuffers;
    QTimer *m_bufferTimer;
    int m_bufferSize;
    int m_frameRate;
};

#endif // GLOWEFFEKT_H

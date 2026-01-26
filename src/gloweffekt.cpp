#include "gloweffekt.h"
#include "ui_mainwindow.h"
#include <QGraphicsDropShadowEffect>

GlowEffect::GlowEffect(Ui::VoiceChangerMainWindow *ui, QObject *parent)
    : QObject(parent), m_ui(ui), m_lowPerformanceMode(false), m_bufferSize(12), m_frameRate(60)
{
    // Setup buffer timer for smooth frame processing
    m_bufferTimer = new QTimer(this);
    connect(m_bufferTimer, &QTimer::timeout, this, &GlowEffect::processBufferedFrames);
    m_bufferTimer->setInterval(1000 / m_frameRate); // 60 FPS by default
    m_bufferTimer->start();
    
    setupGlowEffects();
}

GlowEffect::~GlowEffect()
{
    // Cleanup circular buffers
    for (auto *buffer : m_glowBuffers) {
        delete buffer;
    }
    m_glowBuffers.clear();
}

void GlowEffect::setBufferSize(int size)
{
    m_bufferSize = qMax(8, qMin(24, size)); // Enhanced: 8-24 frames for smoother animations
}

void GlowEffect::setFrameRate(int fps)
{
    m_frameRate = qMax(30, qMin(144, fps)); // Enhanced: 30-144 FPS for high-refresh displays
    m_bufferTimer->setInterval(1000 / m_frameRate);
}

void GlowEffect::setupGlowEffects()
{
    // Setup animated glow for all widgets with optimized parameters
    setupAnimatedGlow(m_ui->caveWidget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->childWidget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->combineWidget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->devilWidget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->femaleWidget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->flangerWidget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->phaseWidget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->robotWidget, QColor(180, 60, 255, 180), 40);
    
    // Setup animated glow for preset widgets
    setupAnimatedGlow(m_ui->preset1Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->preset2Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->preset3Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->preset4Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->preset5Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->preset6Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->preset7Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->preset8Widget, QColor(180, 60, 255, 180), 40);
    
    // Setup animated glow for slot widgets
    setupAnimatedGlow(m_ui->slot1Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot2Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot3Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot4Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot5Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot6Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot7Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot8Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot9Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot10Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot11Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot12Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot13Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot14Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot15Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot16Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot17Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot18Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot19Widget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->slot20Widget, QColor(180, 60, 255, 180), 40);
    
    // Setup animated glow for sidebar widgets
    setupAnimatedGlow(m_ui->miniSidebarWidget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->settingsSidebarWidget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->EffectsSideBarWidget, QColor(180, 60, 255, 180), 40);
}

void GlowEffect::setupAnimatedGlow(QWidget *widget, const QColor &normalColor, int normalRadius)
{
    if (!widget) return;
    
    // Create glow effect with STATIC blur for performance
    auto *glowEffect = new QGraphicsDropShadowEffect();
    glowEffect->setBlurRadius(48); // Static optimal blur (between normal and hover)
    glowEffect->setColor(normalColor);
    glowEffect->setOffset(0, 0);
    widget->setGraphicsEffect(glowEffect);
    
    // Store original properties
    m_originalGlowColors[widget] = normalColor;
    m_originalGlowRadius[widget] = normalRadius;
    
    // Create circular buffer for this widget
    m_glowBuffers[widget] = new GlowFrameBuffer<GlowFrame>(m_bufferSize);
    
    // Create ONLY color animation (no blur animation for performance)
    auto *colorAnimation = new QPropertyAnimation(glowEffect, "color", this);
    colorAnimation->setDuration(m_lowPerformanceMode ? 120 : 180); // Adaptive duration
    colorAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_colorAnimations[widget] = colorAnimation;
    
    // Create animation group (only contains color animation)
    auto *animationGroup = new QParallelAnimationGroup(this);
    animationGroup->addAnimation(colorAnimation);
    m_animationGroups[widget] = animationGroup;
    
    // Install event filter
    widget->installEventFilter(this);
}

void GlowEffect::addToBuffer(QWidget *widget, int blurRadius, const QColor &color)
{
    if (!m_glowBuffers.contains(widget)) return;
    
    GlowFrame frame;
    frame.blurRadius = blurRadius;
    frame.color = color;
    frame.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    auto *buffer = m_glowBuffers[widget];
    
    // Add interpolation frames for smoother transitions
    if (buffer->size() > 0) {
        GlowFrame lastFrame;
        if (buffer->peek(0, lastFrame)) {
            // Calculate interpolation steps based on color difference
            int colorDiff = qAbs(lastFrame.color.red() - color.red()) + 
                           qAbs(lastFrame.color.green() - color.green()) + 
                           qAbs(lastFrame.color.blue() - color.blue()) +
                           qAbs(lastFrame.color.alpha() - color.alpha());
            
            int interpolationSteps = qMin(4, qMax(1, colorDiff / 30)); // 1-4 steps
            
            for (int i = 1; i < interpolationSteps; ++i) {
                float progress = (float)i / (float)interpolationSteps;
                
                GlowFrame interpFrame;
                interpFrame.blurRadius = blurRadius; // Static blur
                interpFrame.color = QColor(
                    lastFrame.color.red() + (color.red() - lastFrame.color.red()) * progress,
                    lastFrame.color.green() + (color.green() - lastFrame.color.green()) * progress,
                    lastFrame.color.blue() + (color.blue() - lastFrame.color.blue()) * progress,
                    lastFrame.color.alpha() + (color.alpha() - lastFrame.color.alpha()) * progress
                );
                interpFrame.timestamp = frame.timestamp - (interpolationSteps - i) * 4; // 4ms intervals
                
                buffer->push(interpFrame);
            }
        }
    }
    
    // Add the target frame
    buffer->push(frame);
}

void GlowEffect::processBufferedFrames()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    int frameWindow = 1000 / m_frameRate; // Dynamic frame window based on FPS
    
    for (auto it = m_glowBuffers.begin(); it != m_glowBuffers.end(); ++it) {
        QWidget *widget = it.key();
        GlowFrameBuffer<GlowFrame> *buffer = it.value();
        
        if (buffer->isEmpty()) continue;
        
        // Get the most appropriate frame for current time
        GlowFrame bestFrame;
        bool foundFrame = false;
        qint64 smallestTimeDiff = LLONG_MAX;
        
        // Look for the frame closest to current time within the frame window
        for (int i = 0; i < buffer->size(); ++i) {
            GlowFrame candidateFrame;
            if (buffer->peek(i, candidateFrame)) {
                qint64 timeDiff = currentTime - candidateFrame.timestamp;
                
                // Only consider frames within reasonable time window
                if (timeDiff >= 0 && timeDiff <= frameWindow * 2) {
                    if (timeDiff < smallestTimeDiff) {
                        smallestTimeDiff = timeDiff;
                        bestFrame = candidateFrame;
                        foundFrame = true;
                    }
                }
            }
        }
        
        // If no frame found in time window, use the newest frame
        if (!foundFrame && buffer->peek(0, bestFrame)) {
            foundFrame = true;
        }
        
        if (foundFrame) {
            auto *glowEffect = qobject_cast<QGraphicsDropShadowEffect*>(widget->graphicsEffect());
            if (glowEffect) {
                glowEffect->setBlurRadius(bestFrame.blurRadius);
                glowEffect->setColor(bestFrame.color);
            }
        }
    }
}

void GlowEffect::startHoverAnimation(QWidget *widget)
{
    if (!m_animationGroups.contains(widget)) return;
    
    // Instead of direct animation, add target frame to buffer
    addToBuffer(widget, 48, QColor(255, 120, 255, 220));
}

void GlowEffect::startLeaveAnimation(QWidget *widget)
{
    if (!m_animationGroups.contains(widget)) return;
    
    // Instead of direct animation, add target frame to buffer
    addToBuffer(widget, 48, m_originalGlowColors[widget]);
}

bool GlowEffect::eventFilter(QObject *obj, QEvent *event)
{
    QWidget *widget = qobject_cast<QWidget*>(obj);
    if (!widget) return QObject::eventFilter(obj, event);
    
    if (!m_animationGroups.contains(widget)) return QObject::eventFilter(obj, event);
    
    switch (event->type()) {
    case QEvent::Enter:
        // CRITICAL: Prevent duplicate animations
        if (!m_hoveredWidgets.contains(widget)) {
            m_hoveredWidgets.insert(widget);
            startHoverAnimation(widget);
        }
        break;
        
    case QEvent::Leave:
        // CRITICAL: Prevent duplicate animations
        if (m_hoveredWidgets.contains(widget)) {
            m_hoveredWidgets.remove(widget);
            startLeaveAnimation(widget);
        }
        break;
        
    default:
        break;
    }
    
    return QObject::eventFilter(obj, event);
}

void GlowEffect::onAnimationFinished()
{
    // Animation finished - Qt handles cleanup automatically
}

void GlowEffect::setLowPerformanceMode(bool enabled)
{
    m_lowPerformanceMode = enabled;
    
    // Update all animation durations for audio safety
    for (auto *colorAnim : m_colorAnimations) {
        colorAnim->setDuration(enabled ? 100 : 180); // Faster in low performance mode
    }
    
    // Adjust frame rate and buffer size for performance
    if (enabled) {
        setFrameRate(45); // Reduced but still smooth
        setBufferSize(8); // Smaller buffer for less memory
    } else {
        setFrameRate(60); // Normal smooth rate
        setBufferSize(12); // Default buffer size
    }
}

void GlowEffect::setGlowEnabled(QWidget *widget, bool enabled)
{
    if (!widget) return;
    
    QGraphicsEffect *effect = widget->graphicsEffect();
    if (effect) {
        effect->setEnabled(enabled);
    }
}

void GlowEffect::setGlowColor(QWidget *widget, const QColor &color)
{
    if (!widget) return;
    
    auto *glowEffect = qobject_cast<QGraphicsDropShadowEffect*>(widget->graphicsEffect());
    if (glowEffect) {
        glowEffect->setColor(color);
        m_originalGlowColors[widget] = color;
    }
}

void GlowEffect::setGlowBlurRadius(QWidget *widget, int radius)
{
    if (!widget) return;
    
    auto *glowEffect = qobject_cast<QGraphicsDropShadowEffect*>(widget->graphicsEffect());
    if (glowEffect) {
        glowEffect->setBlurRadius(radius);
        m_originalGlowRadius[widget] = radius;
    }
}

#include "gloweffekt.h"
#include "ui_mainwindow.h"
#include <QGraphicsDropShadowEffect>

// GlowEffectWorker implementation
GlowEffectWorker::GlowEffectWorker(QObject *parent)
    : QObject(parent)
{
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
        // Process all active animations
        for (auto it = m_animationSteps.begin(); it != m_animationSteps.end(); ++it) {
            QWidget *widget = it.key();
            if (m_currentStep[widget] < m_totalSteps[widget]) {
                // Calculate current values using linear interpolation
                float progress = (float)m_currentStep[widget] / (float)m_totalSteps[widget];
                
                // Interpolate blur radius
                int currentRadius = m_startRadius[widget] + 
                    (m_targetRadius[widget] - m_startRadius[widget]) * progress;
                
                // Interpolate color
                int currentRed = m_startColor[widget].red() + 
                    (m_targetColor[widget].red() - m_startColor[widget].red()) * progress;
                int currentGreen = m_startColor[widget].green() + 
                    (m_targetColor[widget].green() - m_startColor[widget].green()) * progress;
                int currentBlue = m_startColor[widget].blue() + 
                    (m_targetColor[widget].blue() - m_startColor[widget].blue()) * progress;
                int currentAlpha = m_startColor[widget].alpha() + 
                    (m_targetColor[widget].alpha() - m_startColor[widget].alpha()) * progress;
                
                QColor currentColor(currentRed, currentGreen, currentBlue, currentAlpha);
                
                // Emit update signal to main thread
                emit updateGlowEffect(widget, currentRadius, currentColor);
                
                m_currentStep[widget]++;
            } else {
                // Animation finished, remove from active animations
                m_animationSteps.remove(widget);
                m_currentStep.remove(widget);
                m_totalSteps.remove(widget);
                m_startRadius.remove(widget);
                m_targetRadius.remove(widget);
                m_startColor.remove(widget);
                m_targetColor.remove(widget);
            }
        }
        
        // Stop timer if no active animations
        if (m_animationSteps.isEmpty()) {
            m_animationTimer->stop();
        }
    });
    
    m_animationTimer->setInterval(16); // ~60 FPS
}

void GlowEffectWorker::processHoverAnimation(QWidget *widget, int startRadius, int endRadius, 
                                           const QColor &startColor, const QColor &endColor)
{
    if (!widget) return;
    
    // Setup animation parameters
    m_startRadius[widget] = startRadius;
    m_targetRadius[widget] = endRadius;
    m_startColor[widget] = startColor;
    m_targetColor[widget] = endColor;
    m_currentStep[widget] = 0;
    m_totalSteps[widget] = 18; // 300ms / 16ms = ~18 steps
    m_animationSteps[widget] = 1;
    
    // Start timer if not running
    if (!m_animationTimer->isActive()) {
        m_animationTimer->start();
    }
}

void GlowEffectWorker::processLeaveAnimation(QWidget *widget, int startRadius, int endRadius,
                                           const QColor &startColor, const QColor &endColor)
{
    if (!widget) return;
    
    // Setup animation parameters
    m_startRadius[widget] = startRadius;
    m_targetRadius[widget] = endRadius;
    m_startColor[widget] = startColor;
    m_targetColor[widget] = endColor;
    m_currentStep[widget] = 0;
    m_totalSteps[widget] = 18; // 300ms / 16ms = ~18 steps
    m_animationSteps[widget] = 1;
    
    // Start timer if not running
    if (!m_animationTimer->isActive()) {
        m_animationTimer->start();
    }
}

// GlowEffect implementation
GlowEffect::GlowEffect(Ui::VoiceChangerMainWindow *ui, QObject *parent)
    : QObject(parent), m_ui(ui)
{
    // Setup worker thread
    m_workerThread = new QThread(this);
    m_worker = new GlowEffectWorker();
    m_worker->moveToThread(m_workerThread);
    
    // Connect signals
    connect(m_worker, &GlowEffectWorker::updateGlowEffect, 
            this, &GlowEffect::onUpdateGlowEffect, Qt::QueuedConnection);
    
    // Start worker thread
    m_workerThread->start();
    
    setupGlowEffects();
}

GlowEffect::~GlowEffect()
{
    // Cleanup worker thread
    if (m_workerThread && m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
    
    if (m_worker) {
        m_worker->deleteLater();
    }
}

void GlowEffect::setupGlowEffects()
{
    // Setup animated glow for all widgets
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

    setupAnimatedGlow(m_ui->miniSidebarWidget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->settingsSidebarWidget, QColor(180, 60, 255, 180), 40);
    setupAnimatedGlow(m_ui->EffectsSideBarWidget, QColor(180, 60, 255, 180), 40);

}


void GlowEffect::setupAnimatedGlow(QWidget *widget, const QColor &normalColor, int normalRadius)
{
    if (!widget) return;
    
    // Create glow effect
    auto *glowEffect = new QGraphicsDropShadowEffect();
    glowEffect->setBlurRadius(normalRadius);
    glowEffect->setColor(normalColor);
    glowEffect->setOffset(0, 0);
    widget->setGraphicsEffect(glowEffect);
    
    // Store original properties
    m_originalGlowColors[widget] = normalColor;
    m_originalGlowRadius[widget] = normalRadius;
    
    // Install event filter
    widget->installEventFilter(this);
}

void GlowEffect::startHoverAnimation(QWidget *widget)
{
    if (!widget || !m_originalGlowColors.contains(widget)) return;
    
    // Send animation request to worker thread
    QMetaObject::invokeMethod(m_worker, "processHoverAnimation", Qt::QueuedConnection,
        Q_ARG(QWidget*, widget),
        Q_ARG(int, m_originalGlowRadius[widget]),
        Q_ARG(int, 65), // Hover blur radius
        Q_ARG(QColor, m_originalGlowColors[widget]),
        Q_ARG(QColor, QColor(255, 120, 255, 240))); // Hover color
}

void GlowEffect::startLeaveAnimation(QWidget *widget)
{
    if (!widget || !m_originalGlowColors.contains(widget)) return;
    
    // Send animation request to worker thread
    QMetaObject::invokeMethod(m_worker, "processLeaveAnimation", Qt::QueuedConnection,
        Q_ARG(QWidget*, widget),
        Q_ARG(int, 65), // From hover blur radius
        Q_ARG(int, m_originalGlowRadius[widget]),
        Q_ARG(QColor, QColor(255, 120, 255, 240)), // From hover color
        Q_ARG(QColor, m_originalGlowColors[widget]));
}

bool GlowEffect::eventFilter(QObject *obj, QEvent *event)
{
    QWidget *widget = qobject_cast<QWidget*>(obj);
    if (!widget) return QObject::eventFilter(obj, event);
    
    if (!m_originalGlowColors.contains(widget)) return QObject::eventFilter(obj, event);
    
    switch (event->type()) {
    case QEvent::Enter:
        startHoverAnimation(widget);
        break;
        
    case QEvent::Leave:
        startLeaveAnimation(widget);
        break;
        
    default:
        break;
    }
    
    return QObject::eventFilter(obj, event);
}

void GlowEffect::onUpdateGlowEffect(QWidget *widget, int blurRadius, const QColor &color)
{
    if (!widget) return;
    
    auto *glowEffect = qobject_cast<QGraphicsDropShadowEffect*>(widget->graphicsEffect());
    if (glowEffect) {
        glowEffect->setBlurRadius(blurRadius);
        glowEffect->setColor(color);
    }
}

void GlowEffect::onAnimationFinished()
{
    // Animation finished - can add additional logic here if needed
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

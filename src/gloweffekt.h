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
#include <QThread>
#include <QTimer>

class Ui_VoiceChangerMainWindow;

class GlowEffectWorker : public QObject
{
    Q_OBJECT

public:
    explicit GlowEffectWorker(QObject *parent = nullptr);

public slots:
    void processHoverAnimation(QWidget *widget, int startRadius, int endRadius, 
                              const QColor &startColor, const QColor &endColor);
    void processLeaveAnimation(QWidget *widget, int startRadius, int endRadius,
                              const QColor &startColor, const QColor &endColor);

signals:
    void updateGlowEffect(QWidget *widget, int blurRadius, const QColor &color);

private:
    QTimer *m_animationTimer;
    QMap<QWidget*, int> m_currentRadius;
    QMap<QWidget*, QColor> m_currentColor;
    QMap<QWidget*, int> m_targetRadius;
    QMap<QWidget*, QColor> m_targetColor;
    QMap<QWidget*, int> m_startRadius;
    QMap<QWidget*, QColor> m_startColor;
    QMap<QWidget*, int> m_animationSteps;
    QMap<QWidget*, int> m_currentStep;
    QMap<QWidget*, int> m_totalSteps;
};

class GlowEffect : public QObject
{
    Q_OBJECT

public:
    explicit GlowEffect(Ui::VoiceChangerMainWindow *ui, QObject *parent = nullptr);
    ~GlowEffect();

    // Setup glow effects for all widgets
    void setupGlowEffects();

    // Control individual widget glow
    void setGlowEnabled(QWidget *widget, bool enabled);
    void setGlowColor(QWidget *widget, const QColor &color);
    void setGlowBlurRadius(QWidget *widget, int radius);

protected:
    // Event filter for hover effects
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onAnimationFinished();
    void onUpdateGlowEffect(QWidget *widget, int blurRadius, const QColor &color);

private:
    void setupAnimatedGlow(QWidget *widget, const QColor &normalColor, int normalRadius);
    void startHoverAnimation(QWidget *widget);
    void startLeaveAnimation(QWidget *widget);

    Ui::VoiceChangerMainWindow *m_ui;
    QMap<QWidget*, QColor> m_originalGlowColors;
    QMap<QWidget*, int> m_originalGlowRadius;
    QMap<QWidget*, QPropertyAnimation*> m_blurAnimations;
    QMap<QWidget*, QPropertyAnimation*> m_colorAnimations;
    QMap<QWidget*, QParallelAnimationGroup*> m_animationGroups;
    
    // Threading support
    QThread *m_workerThread;
    GlowEffectWorker *m_worker;
};

#endif // GLOWEFFEKT_H

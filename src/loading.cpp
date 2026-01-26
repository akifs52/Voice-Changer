#include "loading.h"
#include "ui_loading.h"
#include <QScreen>
#include <QApplication>
#include <QEasingCurve>

LoadingWidget::LoadingWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoadingWidget)
    , progressTimer(new QTimer(this))
    , fadeInAnimation(nullptr)
    , fadeOutAnimation(nullptr)
    , opacityEffect(nullptr)
{
    ui->setupUi(this);
    
    // Set transparent background attributes
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    
    // Setup animations
    setupAnimations();
    
    // Connect progress timer for smooth animations
    connect(progressTimer, &QTimer::timeout, this, &LoadingWidget::animateProgress);
    
    // Initially hidden
    hide();
}

LoadingWidget::~LoadingWidget()
{
    delete ui;
}

void LoadingWidget::setupAnimations()
{
    // Create opacity effect for fade animations
    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    
    // Setup fade in animation
    fadeInAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeInAnimation->setDuration(500);
    fadeInAnimation->setStartValue(0.0);
    fadeInAnimation->setEndValue(1.0);
    fadeInAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    
    // Setup fade out animation
    fadeOutAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeOutAnimation->setDuration(300);
    fadeOutAnimation->setStartValue(1.0);
    fadeOutAnimation->setEndValue(0.0);
    fadeOutAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    
    connect(fadeOutAnimation, &QPropertyAnimation::finished, this, [this]() {
        hide();
        opacityEffect->setOpacity(1.0); // Reset for next use
    });
}

void LoadingWidget::centerOnScreen()
{
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void LoadingWidget::showLoading()
{
    centerOnScreen();
    show();
    raise();
    
    // Fade in
    opacityEffect->setOpacity(0.0);
    fadeInAnimation->start();
}

void LoadingWidget::hideLoading()
{
    fadeOutAnimation->start();
}

void LoadingWidget::updateProgress(int current, int total)
{
    if (total > 0) {
        int progress = (current * 100) / total;
        ui->progressBar->setValue(progress);
    }
}

void LoadingWidget::updateStatus(const QString &status)
{
    ui->loadingLabel->setText(status);
}

void LoadingWidget::animateProgress()
{
    // This can be used for smooth progress animations if needed
    // Currently progress is updated directly via updateProgress()
}

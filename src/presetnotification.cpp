#include "presetnotification.h"
#include "ui_presetnotification.h"
#include <QScreen>
#include <QApplication>
#include <QEasingCurve>

PresetNotification::PresetNotification(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PresetNotification)
    , hideTimer(new QTimer(this))
    , fadeOutAnimation(nullptr)
    , opacityEffect(nullptr)
{
    ui->setupUi(this);
    
    // Set window attributes for floating notification
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    
    // Setup animations
    setupAnimations();
    
    // Connect timer
    connect(hideTimer, &QTimer::timeout, this, &PresetNotification::hideNotification);
    
    // Connect buttons
    connectButtons();
    
    // Initially hidden
    hide();
}

PresetNotification::~PresetNotification()
{
    delete ui;
}

void PresetNotification::connectButtons()
{
    connect(ui->yesButton, &QPushButton::clicked, this, &PresetNotification::onYesClicked);
    connect(ui->noButton, &QPushButton::clicked, this, &PresetNotification::onNoClicked);
}

void PresetNotification::setupAnimations()
{
    // Create opacity effect for fade animations
    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    
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

void PresetNotification::showConfirmationDialog(const QString &message)
{
    // Set message
    ui->messageLabel->setText(message);
    

    
    // Change background to blue (confirmation color)
    ui->notificationFrame->setStyleSheet(
        "QFrame {"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 #3498db, stop:0.5 #2980b9, stop:1 #1f618d);"
        "border-radius: 12px;"
        "border: 2px solid rgba(255, 255, 255, 0.2);"
        "}"
    );
    
    // Show buttons
    ui->yesButton->show();
    ui->noButton->show();
    
    // Center notification on screen
    centerNotification();
    
    // Show with fade in effect
    show();
    raise();
    
    // Fade in
    opacityEffect->setOpacity(0.0);
    QPropertyAnimation *fadeInAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeInAnimation->setDuration(200);
    fadeInAnimation->setStartValue(0.0);
    fadeInAnimation->setEndValue(1.0);
    fadeInAnimation->setEasingCurve(QEasingCurve::OutQuad);
    fadeInAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void PresetNotification::showSuccessNotification(const QString &message, int duration)
{
    // Set message
    ui->messageLabel->setText(message);
      
    // Change background to green (success color)
    ui->notificationFrame->setStyleSheet(
        "QFrame {"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 #2ecc71, stop:0.5 #27ae60, stop:1 #229954);"
        "border-radius: 12px;"
        "border: 2px solid rgba(255, 255, 255, 0.2);"
        "}"
    );
    
    // Hide buttons for notification
    ui->yesButton->hide();
    ui->noButton->hide();
    
    // Center notification on screen
    centerNotification();
    
    // Show with fade in effect
    show();
    raise();
    
    // Fade in
    opacityEffect->setOpacity(0.0);
    QPropertyAnimation *fadeInAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeInAnimation->setDuration(200);
    fadeInAnimation->setStartValue(0.0);
    fadeInAnimation->setEndValue(1.0);
    fadeInAnimation->setEasingCurve(QEasingCurve::OutQuad);
    fadeInAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Start hide timer
    hideTimer->start(duration);
}

void PresetNotification::hideNotification()
{
    hideTimer->stop();
    fadeOutAnimation->start();
}

void PresetNotification::onYesClicked()
{
    emit confirmed();
    hideNotification();
}

void PresetNotification::onNoClicked()
{
    emit cancelled();
    hideNotification();
}

void PresetNotification::centerNotification()
{
    if (!parentWidget()) {
        // Center on screen if no parent
        QScreen *screen = QApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    } else {
        // Center on parent widget
        QWidget *parent = parentWidget();
        int x = parent->x() + (parent->width() - width()) / 2;
        int y = parent->y() + (parent->height() - height()) / 2;
        move(x, y);
    }
}

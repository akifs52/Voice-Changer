#ifndef PRESETNOTIFICATION_H
#define PRESETNOTIFICATION_H

#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

namespace Ui {
class PresetNotification;
}

class PresetNotification : public QWidget
{
    Q_OBJECT

public:
    explicit PresetNotification(QWidget *parent = nullptr);
    ~PresetNotification();

    void showConfirmationDialog(const QString &message);
    void showSuccessNotification(const QString &message, int duration = 2000);

signals:
    void confirmed();
    void cancelled();

private slots:
    void hideNotification();
    void onYesClicked();
    void onNoClicked();

private:
    Ui::PresetNotification *ui;
    QTimer *hideTimer;
    QPropertyAnimation *fadeOutAnimation;
    QGraphicsOpacityEffect *opacityEffect;
    
    void setupAnimations();
    void centerNotification();
    void connectButtons();
};

#endif // PRESETNOTIFICATION_H

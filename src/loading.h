#ifndef LOADING_H
#define LOADING_H

#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

namespace Ui {
class LoadingWidget;
}

class LoadingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoadingWidget(QWidget *parent = nullptr);
    ~LoadingWidget();

    void updateProgress(int current, int total);
    void updateStatus(const QString &status);
    void showLoading();
    void hideLoading();

private slots:
    void animateProgress();

private:
    Ui::LoadingWidget *ui;
    QTimer *progressTimer;
    QPropertyAnimation *fadeInAnimation;
    QPropertyAnimation *fadeOutAnimation;
    QGraphicsOpacityEffect *opacityEffect;
    
    void setupAnimations();
    void centerOnScreen();
};

#endif // LOADING_H

#ifndef ANIMATIONWIDGET_H
#define ANIMATIONWIDGET_H

#include <QObject>
#include <QWidget>
#include <QPropertyAnimation>
#include <QAbstractAnimation>

class AnimationWidget : public QObject
{
    Q_OBJECT

public:
    explicit AnimationWidget(QWidget *parent = nullptr);

    // Main animation functions
    void animateSidebarTransition(QWidget *widgetToShow, QWidget *widgetToHide, 
                                 const QString &direction = "left", int duration = 300);
    void animateFadeTransition(QWidget *widgetToShow, QWidget *widgetToHide, int duration = 300);
    void animateSlideTransition(QWidget *widget, const QString &direction, int duration = 300);

private:
    QAbstractAnimation* createHideAnimation(QWidget *widget, const QString &direction, int duration);
    QAbstractAnimation* createShowAnimation(QWidget *widget, const QString &direction, int duration);
    int calculateAdaptiveDuration(const QPoint &start, const QPoint &end);
    QEasingCurve::Type getEasingCurveForType(const QString &type, bool isHide = false);
    void setWidgetAnimating(QWidget *widget, bool animating);
};

#endif // ANIMATIONWIDGET_H

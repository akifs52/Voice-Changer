#include "animationwidget.h"
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QEasingCurve>

AnimationWidget::AnimationWidget(QWidget *parent)
    : QObject(parent)
{
}

void AnimationWidget::animateSidebarTransition(QWidget *widgetToShow, QWidget *widgetToHide, 
                                            const QString &direction, int duration)
{
    // Create parallel animation group for smooth transitions
    QParallelAnimationGroup *animationGroup = new QParallelAnimationGroup(this);
    
    // Cross-slide logic: opposite directions for hide/show
    QString hideDirection = direction;
    QString showDirection;
    
    if (direction == "left") {
        showDirection = "right";
    } else if (direction == "right") {
        showDirection = "left";
    } else if (direction == "up") {
        showDirection = "down";
    } else if (direction == "down") {
        showDirection = "up";
    } else {
        showDirection = direction; // fallback
    }
    
    // Hide animation for the widget that's disappearing
    if (widgetToHide && widgetToHide->isVisible()) {
        QAbstractAnimation *hideAnimation = createHideAnimation(widgetToHide, hideDirection, duration);
        if (hideAnimation) {
            animationGroup->addAnimation(hideAnimation);
        }
    }
    
    // Show animation for the widget that's appearing
    if (widgetToShow && !widgetToShow->isVisible()) {
        // Show the widget first but make it transparent
        widgetToShow->show();
        
        QAbstractAnimation *showAnimation = createShowAnimation(widgetToShow, showDirection, duration);
        if (showAnimation) {
            animationGroup->addAnimation(showAnimation);
        }
    }
    
    // Start the animation
    animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Connect finished signal to cleanup
    connect(animationGroup, &QAbstractAnimation::finished, [widgetToHide]() {
        if (widgetToHide) {
            widgetToHide->hide();
        }
    });
}

QAbstractAnimation* AnimationWidget::createHideAnimation(QWidget *widget, const QString &direction, int duration)
{
    if (!widget) return nullptr;
    
    // Check if already animating
    if (widget->property("animating").toBool())
        return nullptr;
    
    setWidgetAnimating(widget, true);
    
    // Use pos animation instead of geometry
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
    animation->setDuration(duration);
    animation->setEasingCurve(getEasingCurveForType("hide", true));
    
    QPoint currentPos = widget->pos();
    QPoint endPos = currentPos;
    
    if (direction == "left") {
        endPos.setX(currentPos.x() - widget->width());
    } else if (direction == "right") {
        endPos.setX(currentPos.x() + widget->width());
    } else if (direction == "up") {
        endPos.setY(currentPos.y() - widget->height());
    } else if (direction == "down") {
        endPos.setY(currentPos.y() + widget->height());
    }
    
    // Calculate adaptive duration
    int adaptiveDuration = calculateAdaptiveDuration(currentPos, endPos);
    animation->setDuration(adaptiveDuration);
    
    // Add fade effect with delay
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(opacityEffect);
    
    QPropertyAnimation *opacityAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    opacityAnimation->setDuration(adaptiveDuration);
    opacityAnimation->setStartValue(1.0);
    opacityAnimation->setEndValue(0.0);
    opacityAnimation->setEasingCurve(getEasingCurveForType("hide", true));
    
    // Create sequential group for delay effect
    QSequentialAnimationGroup *opacityGroup = new QSequentialAnimationGroup();
    opacityGroup->addPause(adaptiveDuration * 0.15); // 15% delay for natural feel
    opacityGroup->addAnimation(opacityAnimation);
    
    // Create parallel group for pos and delayed opacity
    QParallelAnimationGroup *group = new QParallelAnimationGroup();
    group->addAnimation(animation);
    group->addAnimation(opacityGroup);
    
    // Set animation values
    animation->setStartValue(currentPos);
    animation->setEndValue(endPos);
    
    // Clean up graphics effect and animating flag when finished
    connect(group, &QAbstractAnimation::finished, [widget, this]() {
        widget->setGraphicsEffect(nullptr);
        setWidgetAnimating(widget, false);
    });
    
    return group;
}

QAbstractAnimation* AnimationWidget::createShowAnimation(QWidget *widget, const QString &direction, int duration)
{
    if (!widget) return nullptr;
    
    // Check if already animating
    if (widget->property("animating").toBool())
        return nullptr;
    
    setWidgetAnimating(widget, true);
    
    // Use pos animation instead of geometry
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
    animation->setDuration(duration);
    animation->setEasingCurve(getEasingCurveForType("show", false));
    
    QPoint currentPos = widget->pos();
    QPoint startPos = currentPos;
    
    if (direction == "left") {
        startPos.setX(currentPos.x() - widget->width());
    } else if (direction == "right") {
        startPos.setX(currentPos.x() + widget->width());
    } else if (direction == "up") {
        startPos.setY(currentPos.y() - widget->height());
    } else if (direction == "down") {
        startPos.setY(currentPos.y() + widget->height());
    }
    
    // Calculate adaptive duration
    int adaptiveDuration = calculateAdaptiveDuration(startPos, currentPos);
    animation->setDuration(adaptiveDuration);
    
    // Add fade effect with delay
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(0.0);
    
    QPropertyAnimation *opacityAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    opacityAnimation->setDuration(adaptiveDuration);
    opacityAnimation->setStartValue(0.0);
    opacityAnimation->setEndValue(1.0);
    opacityAnimation->setEasingCurve(getEasingCurveForType("show", false));
    
    // Create sequential group for delay effect
    QSequentialAnimationGroup *opacityGroup = new QSequentialAnimationGroup();
    opacityGroup->addPause(adaptiveDuration * 0.15); // 15% delay for natural feel
    opacityGroup->addAnimation(opacityAnimation);
    
    // Add subtle scale effect
    QPropertyAnimation *scaleAnimation = new QPropertyAnimation(widget, "geometry");
    scaleAnimation->setDuration(adaptiveDuration);
    scaleAnimation->setEasingCurve(getEasingCurveForType("show", false));
    
    QRect currentGeometry = widget->geometry();
    QRect scaledGeometry = currentGeometry;
    int scaleOffset = static_cast<int>(currentGeometry.width() * 0.03); // 3% scale
    scaledGeometry.adjust(-scaleOffset, -scaleOffset, scaleOffset, scaleOffset);
    
    scaleAnimation->setStartValue(scaledGeometry);
    scaleAnimation->setEndValue(currentGeometry);
    
    // Create parallel group for pos, delayed opacity, and scale
    QParallelAnimationGroup *group = new QParallelAnimationGroup();
    group->addAnimation(animation);
    group->addAnimation(opacityGroup);
    group->addAnimation(scaleAnimation);
    
    // Set animation values
    animation->setStartValue(startPos);
    animation->setEndValue(currentPos);
    
    // Connect opacity animation to cleanup graphics effect and animating flag
    connect(group, &QAbstractAnimation::finished, [widget, this]() {
        widget->setGraphicsEffect(nullptr);
        setWidgetAnimating(widget, false);
    });
    
    return group;
}

void AnimationWidget::animateFadeTransition(QWidget *widgetToShow, QWidget *widgetToHide, int duration)
{
    QParallelAnimationGroup *animationGroup = new QParallelAnimationGroup(this);
    
    // Fade out animation
    if (widgetToHide && widgetToHide->isVisible()) {
        QGraphicsOpacityEffect *hideOpacityEffect = new QGraphicsOpacityEffect(widgetToHide);
        widgetToHide->setGraphicsEffect(hideOpacityEffect);
        
        QPropertyAnimation *hideOpacityAnimation = new QPropertyAnimation(hideOpacityEffect, "opacity");
        hideOpacityAnimation->setDuration(duration);
        hideOpacityAnimation->setStartValue(1.0);
        hideOpacityAnimation->setEndValue(0.0);
        hideOpacityAnimation->setEasingCurve(QEasingCurve::InOutCubic);
        
        animationGroup->addAnimation(hideOpacityAnimation);
        
        connect(hideOpacityAnimation, &QAbstractAnimation::finished, [widgetToHide]() {
            widgetToHide->hide();
            widgetToHide->setGraphicsEffect(nullptr);
        });
    }
    
    // Fade in animation
    if (widgetToShow && !widgetToShow->isVisible()) {
        widgetToShow->show();
        
        QGraphicsOpacityEffect *showOpacityEffect = new QGraphicsOpacityEffect(widgetToShow);
        widgetToShow->setGraphicsEffect(showOpacityEffect);
        showOpacityEffect->setOpacity(0.0);
        
        QPropertyAnimation *showOpacityAnimation = new QPropertyAnimation(showOpacityEffect, "opacity");
        showOpacityAnimation->setDuration(duration);
        showOpacityAnimation->setStartValue(0.0);
        showOpacityAnimation->setEndValue(1.0);
        showOpacityAnimation->setEasingCurve(QEasingCurve::InOutCubic);
        
        animationGroup->addAnimation(showOpacityAnimation);
        
        connect(showOpacityAnimation, &QAbstractAnimation::finished, [widgetToShow]() {
            widgetToShow->setGraphicsEffect(nullptr);
        });
    }
    
    animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationWidget::animateSlideTransition(QWidget *widget, const QString &direction, int duration)
{
    if (!widget) return;
    
    // Check if already animating
    if (widget->property("animating").toBool())
        return;
    
    setWidgetAnimating(widget, true);
    
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
    animation->setDuration(duration);
    animation->setEasingCurve(getEasingCurveForType("slide", false));
    
    QPoint currentPos = widget->pos();
    QPoint startPos = currentPos;
    
    if (direction == "left") {
        startPos.setX(currentPos.x() - widget->width());
    } else if (direction == "right") {
        startPos.setX(currentPos.x() + widget->width());
    } else if (direction == "up") {
        startPos.setY(currentPos.y() - widget->height());
    } else if (direction == "down") {
        startPos.setY(currentPos.y() + widget->height());
    }
    
    // Calculate adaptive duration
    int adaptiveDuration = calculateAdaptiveDuration(startPos, currentPos);
    animation->setDuration(adaptiveDuration);
    
    widget->move(startPos);
    widget->show();
    
    animation->setStartValue(startPos);
    animation->setEndValue(currentPos);
    
    connect(animation, &QAbstractAnimation::finished, [widget, this]() {
        setWidgetAnimating(widget, false);
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

// Helper functions implementation
int AnimationWidget::calculateAdaptiveDuration(const QPoint &start, const QPoint &end)
{
    int distance = QPoint(start - end).manhattanLength();
    return qBound(150, static_cast<int>(distance * 0.6), 350);
}

QEasingCurve::Type AnimationWidget::getEasingCurveForType(const QString &type, bool isHide)
{
    if (type == "hide") {
        return QEasingCurve::InCubic;  // Hide animations: InCubic
    } else if (type == "show") {
        return QEasingCurve::OutCubic;  // Show animations: OutCubic
    } else if (type == "slide") {
        return QEasingCurve::OutBack;  // Slide animations: OutBack for bounce effect
    } else if (type == "sidebar") {
        return isHide ? QEasingCurve::InCubic : QEasingCurve::OutCubic;
    } else if (type == "modal") {
        return QEasingCurve::OutExpo;  // Modal: OutExpo for premium feel
    } else {
        return QEasingCurve::InOutQuad;  // Small UI: InOutQuad
    }
}

void AnimationWidget::setWidgetAnimating(QWidget *widget, bool animating)
{
    if (widget) {
        widget->setProperty("animating", animating);
    }
}

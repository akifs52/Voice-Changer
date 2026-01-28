/********************************************************************************
** Form generated from reading UI file 'loading.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOADING_H
#define UI_LOADING_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoadingWidget
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *loadingFrame;
    QVBoxLayout *verticalLayout_2;
    QLabel *titleLabel;
    QProgressBar *progressBar;
    QLabel *loadingLabel;

    void setupUi(QWidget *LoadingWidget)
    {
        if (LoadingWidget->objectName().isEmpty())
            LoadingWidget->setObjectName("LoadingWidget");
        LoadingWidget->resize(400, 200);
        LoadingWidget->setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        verticalLayout = new QVBoxLayout(LoadingWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        loadingFrame = new QFrame(LoadingWidget);
        loadingFrame->setObjectName("loadingFrame");
        loadingFrame->setFrameShape(QFrame::StyledPanel);
        loadingFrame->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(loadingFrame);
        verticalLayout_2->setSpacing(15);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(20, 20, 20, 20);
        titleLabel = new QLabel(loadingFrame);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(titleLabel);

        progressBar = new QProgressBar(loadingFrame);
        progressBar->setObjectName("progressBar");
        progressBar->setValue(0);
        progressBar->setTextVisible(true);

        verticalLayout_2->addWidget(progressBar);

        loadingLabel = new QLabel(loadingFrame);
        loadingLabel->setObjectName("loadingLabel");
        loadingLabel->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(loadingLabel);


        verticalLayout->addWidget(loadingFrame);


        retranslateUi(LoadingWidget);

        QMetaObject::connectSlotsByName(LoadingWidget);
    } // setupUi

    void retranslateUi(QWidget *LoadingWidget)
    {
        LoadingWidget->setStyleSheet(QCoreApplication::translate("LoadingWidget", "QWidget {\n"
"    background: transparent;\n"
"}", nullptr));
        loadingFrame->setStyleSheet(QCoreApplication::translate("LoadingWidget", "QFrame {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, \n"
"    stop:0 #1a1a2e, stop:0.5 #16213e, stop:1 #0f3460);\n"
"    border-radius: 20px;\n"
"    border: 2px solid rgba(255, 255, 255, 0.1);\n"
"}", nullptr));
        titleLabel->setStyleSheet(QCoreApplication::translate("LoadingWidget", "color: #ffffff;\n"
"font-size: 24px;\n"
"font-weight: bold;\n"
"font-family: 'Segoe UI', Arial, sans-serif;\n"
"padding: 10px;\n"
"background-image: url(qrc:/img/img/icon.png);\n"
"background-repeat: no-repeat;\n"
"background-position: left center;\n"
"padding-left: 45px;", nullptr));
        titleLabel->setText(QCoreApplication::translate("LoadingWidget", "Voice Changer", nullptr));
        progressBar->setStyleSheet(QCoreApplication::translate("LoadingWidget", "QProgressBar {\n"
"    border: none;\n"
"    border-radius: 10px;\n"
"    text-align: center;\n"
"    color: white;\n"
"    font-weight: bold;\n"
"    background: rgba(255, 255, 255, 0.1);\n"
"}\n"
"\n"
"QProgressBar::chunk {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, \n"
"    stop:0 #00d4ff, stop:0.5 #0099cc, stop:1 #006699);\n"
"    border-radius: 10px;\n"
"}", nullptr));
        loadingLabel->setStyleSheet(QCoreApplication::translate("LoadingWidget", "color: #ffffff;\n"
"font-size: 12px;\n"
"font-family: 'Segoe UI', Arial, sans-serif;", nullptr));
        loadingLabel->setText(QCoreApplication::translate("LoadingWidget", "Loading...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoadingWidget: public Ui_LoadingWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOADING_H

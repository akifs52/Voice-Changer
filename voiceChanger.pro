QT       += core gui

QT       += multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    effects.cpp \
    main.cpp \
    mainwindow.cpp \
    recorder.cpp

HEADERS += \
    effects.h \
    mainwindow.h \
    recorder.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavcodec
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavcodec
else:unix: LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavcodec

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavformat
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavformat
else:unix: LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavformat

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavutil
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavutil
else:unix: LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavutil


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavdevice
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavdevice
else:unix: LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/lib/ -lavdevice


INCLUDEPATH += $$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/include
DEPENDPATH += $$PWD/../../ffmpeg-master-latest-win64-gpl-shared/ffmpeg-master-latest-win64-gpl-shared/include

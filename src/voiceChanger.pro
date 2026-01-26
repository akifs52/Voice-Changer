QT       += core gui

QT       += multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    circularbuffer.cpp \
    deletesoundpack.cpp \
    effects.cpp \
    gloweffekt.cpp \
    hotkeys.cpp \
    loadouts.cpp \
    loading.cpp \
    main.cpp \
    mainwindow.cpp \
    presetnotification.cpp \
    psola.cpp \
    recorder.cpp \
    soundpack.cpp \
    audiopipeline.cpp \
    voiceeffects.cpp

HEADERS += \
    circularbuffer.h \
    deletesoundpack.h \
    effects.h \
    gloweffekt.h \
    hotkeys.h \
    loadouts.h \
    loading.h \
    mainwindow.h \
    presetnotification.h \
    psola.h \
    recorder.h \
    soundpack.h \
    audiopipeline.h \
    voiceeffects.h

FORMS += \
    mainwindow.ui \
    loading.ui \
    presetnotification.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}



# FFmpeg libraries - cross-platform
win32 {
    CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavcodec.dll
    else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavcodec.dll
    INCLUDEPATH += $$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/include
    DEPENDPATH += $$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/include

    CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavformat.dll
    else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavformat.dll
    INCLUDEPATH += $$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/include
    DEPENDPATH += $$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/include

    CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavutil.dll
    else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavutil.dll
    INCLUDEPATH += $$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/include
    DEPENDPATH += $$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/include

    CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibswresample.dll
    else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibswresample.dll
    INCLUDEPATH += $$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/include
    DEPENDPATH += $$PWD/../../ffmpeg-master-latest-win64-lgpl-shared/include
}
unix:!macx {
    LIBS += -lavcodec -lavformat -lavutil -lswresample
    INCLUDEPATH += /usr/include/ffmpeg
    DEPENDPATH += /usr/include/ffmpeg
}
macx {
    LIBS += -lavcodec -lavformat -lavutil -lswresample -framework CoreAudio -framework AudioToolbox
}

RESOURCES += \
    Soundpack.qrc

# Icon - cross-platform
win32 {
    RC_ICONS = D:\qt\voiceChanger\img\icon.ico
}
unix:!macx {
    # Linux icon will be handled by .desktop file
}
macx {
    ICON = img/icon.icns
}

win32 {
    LIBS += -luser32
    LIBS += -lmf
    LIBS += -lmfplat
    LIBS += -lmfreadwrite
    LIBS += -lole32
    LIBS += -lksuser
}

# SoundTouch library - cross-platform
win32 {
    CONFIG(release, debug|release): LIBS += -L$$PWD/../../soundtouch_dll-2.3.3/ -lSoundTouchDLL_x64
    else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../soundtouch_dll-2.3.3/ -lSoundTouchDLL_x64
    INCLUDEPATH += $$PWD/../../soundtouch_dll-2.3.3
    DEPENDPATH += $$PWD/../../soundtouch_dll-2.3.3
}
unix:!macx {
    LIBS += -lsoundtouch
    INCLUDEPATH += /usr/include/soundtouch
    DEPENDPATH += /usr/include/soundtouch
}
macx {
    LIBS += -lsoundtouch
}

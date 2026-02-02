QT       += core gui

QT       += multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# WebAssembly plugin imports
wasm {
    QTPLUGIN += qwasmmedia qwasmintegration qsvgicon qgif qicns qico qjpeg qsvg qtga qtiff qwbmp qwebp qtlsbackendcertonly
}

SOURCES += \
    animationwidget.cpp \
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
    voiceeffects.cpp \
    wasm_soundtouch.cpp

HEADERS += \
    animationwidget.h \
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
wasm {
    message("WebAssembly build configuration")
    
    # FFmpeg libraries for WebAssembly - .a files in the main directory
    FFMPEG_LIBS = -LD:/soundtouch-wasm/FFmpeg/wasm_build \
        -lavcodec \
        -lavformat \
        -lavutil \
        -lswresample \
        -lswscale
        
    # SoundTouch library for WebAssembly  
    SOUNDTOUCH_LIBS = -LD:/soundtouch-wasm/soundtouch-wasm/lib \
        -lSoundTouch
        
    # WebAssembly-specific settings
    QMAKE_LFLAGS += -s WASM=1 \
        -s ALLOW_MEMORY_GROWTH=1 \
        -s MAX_WEBGL_VERSION=2 \
        -s WEBGL2_BACKUP_COMPATIBILITY=1 \
        -s DISABLE_EXCEPTION_CATCHING=1 \
        -s PROXY_TO_PTHREAD=1 \
        -s PTHREAD_POOL_SIZE=4 \
        -s EXPORTED_FUNCTIONS="['_main', '_processAudio', '_setEffect', '_setPitch', '_setTempo']" \
        -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" \
        -s MODULARIZE=1 \
        -s EXPORT_NAME="VoiceChangerModule"
        
    # Include paths for WebAssembly libraries
    INCLUDEPATH += D:/soundtouch-wasm/FFmpeg/wasm_build \
        D:/soundtouch-wasm/soundtouch-wasm/include
        
    # Link libraries
    LIBS += $$FFMPEG_LIBS $$SOUNDTOUCH_LIBS
}

win32 {
    # Windows FFmpeg configuration
    win32:CONFIG(release, debug|release): LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibswresample.dll
    else:win32:CONFIG(debug, debug|release): LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibswresample.dll
    else:unix: LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibswresample.dll

    win32:CONFIG(release, debug|release): LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavformat.dll
    else:win32:CONFIG(debug, debug|release): LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavformat.dll
    else:unix: LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavformat.dll

    win32:CONFIG(release, debug|release): LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavcodec.dll
    else:win32:CONFIG(debug, debug|release): LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavcodec.dll
    else:unix: LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavcodec.dll

    # Add missing avutil library
    win32:CONFIG(release, debug|release): LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavutil.dll
    else:win32:CONFIG(debug, debug|release): LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavutil.dll
    else:unix: LIBS += -LD:/ffmpeg-master-latest-win64-lgpl-shared/lib/ -llibavutil.dll

    INCLUDEPATH += D:/ffmpeg-master-latest-win64-lgpl-shared/include
    DEPENDPATH += D:/ffmpeg-master-latest-win64-lgpl-shared/include
}

unix:!macx {
LIBS += -lavcodec -lavformat -lavutil -lswresample -lX11
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
    #ICON = \home\akif\Desktop\voiceChangerUpdated\Voice-Changer\src\img\icon.png
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
    # Windows SoundTouch configuration
    win32:CONFIG(release, debug|release): LIBS += -LD:/soundtouch_dll-2.3.3/ -lSoundTouchDLL_x64
    else:win32:CONFIG(debug, debug|release): LIBS += -LD:/soundtouch_dll-2.3.3/ -lSoundTouchDLL_x64
    else:unix: LIBS += -LD:/soundtouch_dll-2.3.3/ -lSoundTouchDLL_x64

    INCLUDEPATH += D:/soundtouch_dll-2.3.3
    DEPENDPATH += D:/soundtouch_dll-2.3.3
}
unix:!macx {
    LIBS += -lSoundTouch -lX11
    INCLUDEPATH += /usr/include/soundtouch
    DEPENDPATH += /usr/include/soundtouch
}
macx {
    LIBS += -lsoundtouch
}







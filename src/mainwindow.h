#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioSink>
#include <QAudioDecoder>
#include <QAudioSource>
#include <QIODevice>
#include <QAudioFormat>
#include <QMediaPlayer>
#include <QTimer>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QShortcut>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioBuffer>
#include <QMutex>
#include <memory>
#include <mutex>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QPoint>
#include "audiopipeline.h"
#include "circularbuffer.h"
#include "voiceeffects.h"
#include "gloweffekt.h"
#include "qcombobox.h"
#include "qpushbutton.h"
#include "ui_mainwindow.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif
#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif
#ifdef Q_OS_MACOS
#include <Carbon/Carbon.h>
#endif

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

QT_BEGIN_NAMESPACE
namespace Ui {
class VoiceChangerMainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void audioDataReady(const QByteArray &processedAudio);
    void preloadFinished();

private slots:
    // All the original slots...
    void on_outputslider_valueChanged(int value);
    void on_inputslider_valueChanged(int value);
    void on_refreshInput_clicked();
    void on_refreshOutput_clicked();
    void on_inputDeviceCombobox_currentIndexChanged(int index);
    void on_OutputDEviceCombobox_currentIndexChanged(int index);
    void on_virtualInputcombobox_currentIndexChanged(int index);
    void on_VirtualSlider_valueChanged(int value);
    void on_VirtualDriverButton_clicked();
    void processToChildVoice(QByteArray &data);
    void processToRobotVoice(QByteArray &data);
    void processToDevilVoice(QByteArray &data);
    void processToFemaleVoice(QByteArray &data);
    void processToCombineVoice(QByteArray &data);
    void processToEkoVoice(QByteArray &data);
    void processToPhaseVoice(QByteArray &data);
    void processToFlangerVoice(QByteArray &data);
    void on_childButton_clicked(bool checked);
    void on_testButton_clicked(bool checked);
    void on_robotButton_clicked(bool checked);
    void on_devilButton_clicked(bool checked);
    void on_caveButton_clicked(bool checked);
    void on_femaleButton_clicked(bool checked);
    void on_combineButton_clicked(bool checked);
    void on_phaseButton_clicked(bool checked);
    void on_flangerButton_clicked(bool checked);
    void on_startRecord_clicked();
    void on_stopRecord_clicked();
    void on_slot1_clicked();
    void on_slot2_clicked();
    void on_slot3_clicked();
    void on_slot4_clicked();
    void on_slot5_clicked();
    void on_slot6_clicked();
    void on_slot7_clicked();
    void on_slot8_clicked();
    void on_slot9_clicked();
    void on_slot10_clicked();
    void on_slot11_clicked();
    void on_slot12_clicked();
    void on_slot13_clicked();
    void on_slot14_clicked();
    void on_slot15_clicked();
    void on_slot16_clicked();
    void on_slot17_clicked();
    void on_slot18_clicked();
    void on_slot19_clicked();
    void on_slot20_clicked();
    void on_slot1Delete_clicked();
    void on_slot2Delete_clicked();
    void on_slot3Delete_clicked();
    void on_slot4Delete_clicked();
    void on_slot5Delete_clicked();
    void on_slot6Delete_clicked();
    void on_slot7Delete_clicked();
    void on_slot8Delete_clicked();
    void on_slot9Delete_clicked();
    void on_slot10Delete_clicked();
    void on_slot11Delete_clicked();
    void on_slot12Delete_clicked();
    void on_slot13Delete_clicked();
    void on_slot14Delete_clicked();
    void on_slot15Delete_clicked();
    void on_slot16Delete_clicked();
    void on_slot17Delete_clicked();
    void on_slot18Delete_clicked();
    void on_slot19Delete_clicked();
    void on_slot20Delete_clicked();
    void on_load1_clicked();
    void on_load2_clicked();
    void on_load3_clicked();
    void on_load4_clicked();
    void on_load5_clicked();
    void on_save1_clicked();
    void on_save2_clicked();
    void on_savee3_clicked();
    void on_save4_clicked();
    void on_save5_clicked();

    // Titlebar functionality
    void on_minimizeButton_clicked();
    void on_maximizeButton_clicked();
    void on_closeButton_clicked();

    // Slots for hotkeys
    void on_slot1Hotkey_currentTextChanged(const QString &key);
    void on_slot2Hotkey_currentTextChanged(const QString &key);
    void on_slot3Hotkey_currentTextChanged(const QString &key);
    void on_slot4Hotkey_currentTextChanged(const QString &key);
    void on_slot5Hotkey_currentTextChanged(const QString &key);
    void on_slot6Hotkey_currentTextChanged(const QString &key);
    void on_slot7Hotkey_currentTextChanged(const QString &key);
    void on_slot8Hotkey_currentTextChanged(const QString &key);
    void on_slot9Hotkey_currentTextChanged(const QString &key);
    void on_slot10Hotkey_currentTextChanged(const QString &key);
    void on_slot11Hotkey_currentTextChanged(const QString &key);
    void on_slot12Hotkey_currentTextChanged(const QString &key);
    void on_slot13Hotkey_currentTextChanged(const QString &key);
    void on_slot14Hotkey_currentTextChanged(const QString &key);
    void on_slot15Hotkey_currentTextChanged(const QString &key);
    void on_slot16Hotkey_currentTextChanged(const QString &key);
    void on_slot17Hotkey_currentTextChanged(const QString &key);
    void on_slot18Hotkey_currentTextChanged(const QString &key);
    void on_slot19Hotkey_currentTextChanged(const QString &key);
    void on_slot20Hotkey_currentTextChanged(const QString &key);


private:
    Ui::VoiceChangerMainWindow *ui;

    QAudioFormat *format;
    QAudioSink *audioOutput;
    QAudioSource *audioInput;
    QIODevice *inputDevice;
    QIODevice *outputDevice;

    // Virtual audio output için yeni değişkenler
    QAudioSink *virtualAudioOutput;
    QIODevice *virtualOutputDevice;
    bool vbCableFound;
    bool cableInputSelected;
    bool testButtonActive;

    // Titlebar dragging variables
    bool isDragging;
    QPoint dragPosition;
    bool isMaximizing;

    // Soundpack buffer for audio processing
    CircularBuffer *soundpackBuffer;
    
    // Paralel pipeline için ayrı buffer'lar
    CircularBuffer *effectsBuffer;     // Efektli ses için
    CircularBuffer *soundpackBuffer2;  // Soundpack sesleri için
    
    // Audio pipeline for parallel processing
    AudioPipeline *audioPipeline;

    // Dairesel tampon için değişkenler
    std::vector<int16_t> circularBuffer;
    size_t bufferSize = 44100 * 30; // 10 saniyelik buffer (44.1kHz stereo)
    size_t readPos = 0;
    size_t writePos = 0;
    size_t availableSamples = 0;
    std::mutex bufferMutex;
    std::mutex outputDeviceMutex;  // Protect outputDevice access

    // Current playing sound tracking
    QPushButton* currentPlayingButton = nullptr;
    QThread* currentOutputThread = nullptr;
    std::mutex currentSoundMutex;
    bool soundInterrupted = false;  // Flag to stop current audio writing

    QTimer* recordingProcessorTimer = nullptr;

    // Yeni yardımcı fonksiyonlar
    void writeToCircularBuffer(const QByteArray& audioData, bool isMono);
    QByteArray readFromCircularBuffer(size_t samplesNeeded);
    void processRecordedFrames();
    void cleanupRecording();
    void processAudioForRecording(const QByteArray &audioData);
    void setupEffectConnection(const QString &effectName, std::function<void(QByteArray&)> effectProcessor);

    bool usingEffects = true;
    QByteArray data;
    bool isRecording = false;
    bool testButtonWasActive = false;
    QByteArray recordingBuffer;           // Recording için birikim buffer'ı
    QElapsedTimer recordingTimer;
    QElapsedTimer recordingDurationTimer;
    qint64 recordingStartTime = 0;
    qint64 totalRecordedBytes = 0;
    int recordBufferThreshold = 1024 * 10;
    QString currentRecordingFile;

    // Hotkey assignments
    QMap<QString, int> m_hotkeyAssignments; // KeySequence -> Sound Index (1-20)
    QMap<int, QString> m_soundIndexToKey;   // Sound Index (1-20) -> KeySequence
    QMap<QString, QShortcut*> m_shortcuts;    // KeySequence -> QShortcut

    // Global hotkey system for background operation
    QMap<QString, int> m_globalHotkeyIds;    // KeySequence -> Hotkey ID
    static const int GLOBAL_HOTKEY_BASE_ID = 1000;
    
    // Cross-platform global hotkey data
#ifdef Q_OS_LINUX
    Display* m_x11Display = nullptr;
    Window m_rootWindow;
#endif
#ifdef Q_OS_MACOS
    QMap<QString, EventHotKeyRef> m_macHotkeyRefs;
    EventHandlerRef m_macEventHandler = nullptr;
#endif

    // Original sound file variables
    QString filename1 = QCoreApplication::applicationDirPath() + ("/soundpack/YARRA.wav");
    QString pic1 = ":/img/img/yArra.jpg";
    QString filename2 = QCoreApplication::applicationDirPath() + ("/soundpack/YARRA-ULTRA-BASS-EARRAPE.wav");
    QString pic2 = ":/img/img/yArra-Earrape.jpg";
    QString filename3 = QCoreApplication::applicationDirPath() + ("/soundpack/Chinese-Rap-meme.wav");
    QString pic3 = ":/img/img/chineseRap.jpeg" ;
    QString filename4 = QCoreApplication::applicationDirPath() + ("/soundpack/Danger alarm meme sound effect.wav");
    QString pic4 = ":/img/img/ohacet.jpg";
    QString filename5 = QCoreApplication::applicationDirPath() + ("/soundpack/sarhos-talisca.wav");
    QString pic5 = ":/img/img/talisca.jpeg";
    QString filename6 = QCoreApplication::applicationDirPath() + ("/soundpack/Among-us-sus-troll-face-meme.wav");
    QString pic6 = ":/img/img/trollface.jpg";
    QString filename7 = QCoreApplication::applicationDirPath() + ("/soundpack/AMOGUS.wav");
    QString pic7 = ":/img/img/sus.jpg" ;
    QString filename8 = QCoreApplication::applicationDirPath() + ("/soundpack/Yankili-Osuruk-Sesi.wav");
    QString pic8 = ":/img/img/osuruk1.jpg";
    QString filename9 = QCoreApplication::applicationDirPath() + ("/soundpack/ishal-osuruk-sesi.wav");
    QString pic9 = ":/img/img/osuruk.jpg";
    QString filename10 = QCoreApplication::applicationDirPath() + ("/soundpack/Sound-Effect-Yankili-Osuruk-Sesi.wav") ;
    QString pic10 = ":/img/img/osuru2.jpg";
    QString filename11;
    QString pic11;
    QString filename12;
    QString pic12;
    QString filename13;
    QString pic13;
    QString filename14;
    QString pic14;
    QString filename15;
    QString pic15;
    QString filename16;
    QString pic16;
    QString filename17;
    QString pic17;
    QString filename18;
    QString pic18;
    QString filename19;
    QString pic19;
    QString filename20;
    QString pic20;

    QThread *decodeThread = nullptr;
    QThread *outputThread = nullptr;
    QAudioDecoder *audioDecoder = nullptr;

    // Audio cache for preloading
    struct AudioCache {
        QByteArray audioData;
        bool isLoaded;
        QMutex mutex;
        AudioCache() : isLoaded(false) {}
    };
    QMap<QString, AudioCache*> audioCache;
    int preloadCount = 0;
    int preloadTotal = 0;

    // Functions from other files
    void stopCurrentAudio();
    void stopCurrentSound();  // Stop currently playing sound
    void playAudioNotInterrupt(const QString &filename, const QString &picPath, QPushButton *button);
    void preloadAudio(const QString &filename);
    void searchInputDevice();
    void searchOutputDevice();
    void processAudioInput();
    void progressBarOutput();
    void saveLoadout(const QString &loadoutName);


    // VoiceEffects instance for new DSP effects
    VoiceEffects *voiceEffects;
    
    // GlowEffect instance for widget glow effects
    GlowEffect *glowEffect;

    // Virtual audio fonksiyonları
    void searchVirtualDevices();
    void setupVirtualOutput();
    bool detectVBCable();
    void updateVirtualStatusLabel();
    void showVirtualDeviceInstructions();
    void loadLoadout(const QString &loadoutName);
    void handleHotkeyChange(const QString &key, int soundIndex);
    void populateComboBox(QComboBox *combo);
    void allinputKeys();
    void connectAllHotkeys();
    void setupGlobalShortcuts();
    void saveHotkeys();
    void loadHotkeys();
    void stopAllEffects();
#ifdef Q_OS_WIN
    bool winEventFilter(MSG *message, long *result);
#endif
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
};
#endif // MAINWINDOW_H

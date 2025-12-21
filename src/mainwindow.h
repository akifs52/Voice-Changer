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
#include "audiopipeline.h"
#include "circularbuffer.h"
#include "qcombobox.h"
#include "qpushbutton.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
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

signals:
    void audioDataReady(const QByteArray &processedAudio);
    void preloadFinished();

private slots:
    // All the original slots...
    void on_outputslider_valueChanged(int value);
    void on_inputslider_valueChanged(int value);
    void on_refreshInput_clicked();
    void on_refreshOutput_clicked();
    void on_inputcombobox_currentIndexChanged(int index);
    void on_outputcombobox_currentIndexChanged(int index);
    void on_virtualcombobox_currentIndexChanged(int index);
    void on_virtualslider_valueChanged(int value);
    void processToBananaVoice(QByteArray &data);
    void processToRobotVoice(QByteArray &data);
    void processToDevilVoice(QByteArray &data);
    void processToFemaleVoice(QByteArray &data);
    void processToCombineVoice(QByteArray &data);
    void processToEkoVoice(QByteArray &data);
    void on_bananaButton_clicked(bool checked);
    void on_testButton_clicked(bool checked);
    void on_robotButton_clicked(bool checked);
    void on_devilButton_clicked(bool checked);
    void on_ekoButton_clicked(bool checked);
    void on_femaleButton_clicked(bool checked);
    void on_combineButton_clicked(bool checked);
    void on_startRecord_clicked();
    void on_stopRecord_clicked();
    void on_sound1_clicked();
    void on_sound2_clicked();
    void on_sound3_clicked();
    void on_sound4_clicked();
    void on_sound5_clicked();
    void on_sound6_clicked();
    void on_sound7_clicked();
    void on_sound8_clicked();
    void on_sound9_clicked();
    void on_sound10_clicked();
    void on_sound11_clicked();
    void on_sound12_clicked();
    void on_sound13_clicked();
    void on_sound14_clicked();
    void on_sound15_clicked();
    void on_sound16_clicked();
    void on_sound17_clicked();
    void on_sound18_clicked();
    void on_sound19_clicked();
    void on_sound20_clicked();
    void on_delete1_clicked();
    void on_delete2_clicked();
    void on_delete3_clicked();
    void on_delete4_clicked();
    void on_delete5_clicked();
    void on_delete6_clicked();
    void on_delete7_clicked();
    void on_delete8_clicked();
    void on_delete9_clicked();
    void on_delete10_clicked();
    void on_delete11_clicked();
    void on_delete12_clicked();
    void on_delete13_clicked();
    void on_delete14_clicked();
    void on_delete15_clicked();
    void on_delete16_clicked();
    void on_delete17_clicked();
    void on_delete18_clicked();
    void on_delete19_clicked();
    void on_delete20_clicked();
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

    // Slots for hotkeys
    void on_comboBox1_currentTextChanged(const QString &key);
    void on_comboBox2_currentTextChanged(const QString &key);
    void on_comboBox3_currentTextChanged(const QString &key);
    void on_comboBox4_currentTextChanged(const QString &key);
    void on_comboBox5_currentTextChanged(const QString &key);
    void on_comboBox6_currentTextChanged(const QString &key);
    void on_comboBox7_currentTextChanged(const QString &key);
    void on_comboBox8_currentTextChanged(const QString &key);
    void on_comboBox9_currentTextChanged(const QString &key);
    void on_comboBox10_currentTextChanged(const QString &key);
    void on_comboBox11_currentTextChanged(const QString &key);
    void on_comboBox12_currentTextChanged(const QString &key);
    void on_comboBox13_currentTextChanged(const QString &key);
    void on_comboBox14_currentTextChanged(const QString &key);
    void on_comboBox15_currentTextChanged(const QString &key);
    void on_comboBox16_currentTextChanged(const QString &key);
    void on_comboBox17_currentTextChanged(const QString &key);
    void on_comboBox18_currentTextChanged(const QString &key);
    void on_comboBox19_currentTextChanged(const QString &key);
    void on_comboBox20_currentTextChanged(const QString &key);

private:
    Ui::MainWindow *ui;

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

    bool usingEffects = true;
    QByteArray data;
    bool isRecording = false;
    bool testButtonWasActive = false;
    QElapsedTimer recordingTimer;

    // Hotkey assignments
    QMap<QString, int> m_hotkeyAssignments; // KeySequence -> Sound Index (1-20)
    QMap<int, QString> m_soundIndexToKey;   // Sound Index (1-20) -> KeySequence
    QMap<QString, QShortcut*> m_shortcuts;    // KeySequence -> QShortcut

    // Global hotkey system for background operation
    QMap<QString, int> m_globalHotkeyIds;    // KeySequence -> Hotkey ID
    static const int GLOBAL_HOTKEY_BASE_ID = 1000;

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

    // Virtual audio fonksiyonları
    void searchVirtualDevices();
    void setupVirtualOutput();
    bool detectVBCable();
    void updateVirtualStatusLabel();
    void loadLoadout(const QString &loadoutName);
    void handleHotkeyChange(const QString &key, int soundIndex);
    void populateComboBox(QComboBox *combo);
    void allinputKeys();
    void connectAllHotkeys();
    void setupGlobalShortcuts();
    void saveHotkeys();
    void loadHotkeys();
    void stopAllEffects();
    bool winEventFilter(MSG *message, long *result);
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
};
#endif // MAINWINDOW_H

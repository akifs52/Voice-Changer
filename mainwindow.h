#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioSource> //input mic
#include <QAudioSink> //output
#include <QCoreApplication>
#include "QAudioDecoder"
#include "qpushbutton.h"





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



private slots:

    void on_outputslider_valueChanged(int value);

    void on_inputslider_valueChanged(int value);

    void on_refreshInput_clicked();

    void on_refreshOutput_clicked();

    void on_inputcombobox_currentIndexChanged(int index);

    void on_outputcombobox_currentIndexChanged(int index);

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

    void on_boldButton_clicked(bool checked);

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

private:
    Ui::MainWindow *ui;

    QAudioFormat *format;
    QAudioSink *audioOutput;
    QAudioSource *audioInput;
    QIODevice *inputDevice;
    QIODevice *outputDevice;


    bool usingEffects = true;
    QByteArray data;


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




    void stopCurrentAudio();

    void playAudio(const QString &filename, const QString &picPath, QPushButton *button);

    void playAudioNotInterrupt(const QString &filename, const QString &picPath, QPushButton *button);

    void searchInputDevice();
    void searchOutputDevice();
    void processAudioInput();
    void progressBarOutput();
    void saveLoadout(const QString &loadoutName);
    void loadLoadout(const QString &loadoutName);



};
#endif // MAINWINDOW_H

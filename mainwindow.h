#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioSource> //input mic
#include <QAudioSink> //output
#include <QProcess>

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

private:
    Ui::MainWindow *ui;

    QAudioFormat *format;
    QAudioSink *audioOutput;
    QAudioSource *audioInput;
    QIODevice *inputDevice;
    QIODevice *outputDevice;

    QProcess *ffmpegProcess;


    bool usingEffects = true;
    QByteArray data;

    void searchInputDevice();
    void searchOutputDevice();
    void processAudioInput();
    void progressBarOutput();


};
#endif // MAINWINDOW_H

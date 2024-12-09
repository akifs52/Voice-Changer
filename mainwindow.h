#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioSource> //input mic
#include <QAudioSink> //output

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



    void on_testButton_clicked();

    void on_stopButton_clicked();

private:
    Ui::MainWindow *ui;

    QAudioFormat *format;
    QAudioSink *audioOutput;
    QAudioSource *audioInput;
    QIODevice *inputDevice;
    QIODevice *outputDevice;
    bool isListening;

    void searchInputDevice();
    void searchOutputDevice();
    void processAudioInput();
    void progressBarOutput();

};
#endif // MAINWINDOW_H

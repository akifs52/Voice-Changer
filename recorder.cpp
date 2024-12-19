#include "recorder.h"
#include <QFileDialog>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>

recorder::recorder(QWidget *parent)
    : QMainWindow{parent}
{}



void MainWindow::on_startRecord_clicked()
{
    data.clear();

    // MP3 dosyası için kaydetme yeri seçme
    QString mp3FileName = QFileDialog::getSaveFileName(this, "Save MP3 File", "", "MP3 Files (*.mp3)");
    if (mp3FileName.isEmpty()) return;


    if(!audioInput)
    {
        audioInput->resume();
    }

    // FFmpeg işlemi zaten başlatıldıysa tekrar başlatma
    if (!ffmpegProcess || ffmpegProcess->state() != QProcess::Running) {
        QString ffmpegPath = "ffmpeg";  // FFmpeg yolunu doğru ayarlayın

        QStringList arguments;
        arguments << "-f" << "s16le"              // PCM formatı (16-bit little-endian)
                  << "-ar" << "44800"            // Örnekleme hızı
                  << "-ac" << "2"                // Kanal sayısı
                  << "-i" << "pipe:0"            // Standart girişten veri al
                  << mp3FileName;                // Çıktı dosyası

        ffmpegProcess->start(ffmpegPath, arguments);
        if (!ffmpegProcess->waitForStarted()) {
            qWarning() << "Failed to start FFmpeg process:" << ffmpegProcess->errorString();
            return;
        }

        qDebug() << "FFmpeg process started for recording.";
    }

    // Veriyi kaydetmeye başla
    connect(inputDevice, &QIODevice::readyRead, this, [=]() {

        if(usingEffects)
        {
            data = inputDevice->readAll();
        }


        if (data.isEmpty()) {
            qWarning() << "No data to save.";
            return;
        }

        // Kaydedilen ham veriyi FFmpeg'e yaz
        if (ffmpegProcess && ffmpegProcess->state() == QProcess::Running) {
            ffmpegProcess->write(data);
        } else {
            qWarning() << "FFmpeg process is not running.";
        }
    });

}



void MainWindow::on_stopRecord_clicked()
{

    if (ffmpegProcess->state() == QProcess::Running) {
        qDebug() << "Kayıt durduruluyor...";

        // ffmpeg'e özel bir kapatma sinyali gönder
        ffmpegProcess->write("q");  // FFmpeg'e "q" göndererek kaydı bitirmesini istiyoruz
        ffmpegProcess->closeWriteChannel();  // Yazma kanalını kapat, FFmpeg sonlandırmayı işlesin

        if (!ffmpegProcess->waitForFinished(5000)) {  // 5 saniye bekle, hala durmadıysa...
            qDebug() << "FFmpeg işlemi düzgün şekilde sonlandırılamadı, zorla kapatılıyor...";
            ffmpegProcess->kill();  // Zorla sonlandırmak en son çare
            ffmpegProcess->waitForFinished();
        }
    }

    disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
}

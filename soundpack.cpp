#include "soundpack.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QMediaPlayer"
#include "QBuffer"
#include <QThread>


bool isPlaying = false;

std::atomic<bool> stopRequested = false;


soundpack::soundpack(QWidget *parent)
    : QMainWindow{parent}
{}

QByteArray *localData = new QByteArray;


void MainWindow::playAudioNotInterrupt(const QString &filename, const QString &picPath, QPushButton *button)
{

    localData->clear();

    QPixmap icon(picPath);

    QIcon buttonIcon = icon;

    button->setIcon(buttonIcon);

    QSize size(75,75);

    button->setIconSize(size);


    // QThread oluşturuluyor
    QThread *decodeThread = new QThread;
    QAudioDecoder *audioDecoder = new QAudioDecoder();

    audioDecoder->setAudioFormat(*format);

    // Decoder, thread'e taşınıyor
    audioDecoder->moveToThread(decodeThread);

    connect(decodeThread, &QThread::started, audioDecoder, [=]() {
        audioDecoder->setSource(filename);
        audioDecoder->start();
    });

    connect(audioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = audioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        localData->append(pcmData);
    });

    connect(audioDecoder, &QAudioDecoder::finished, this, [=]() {
        qDebug() << "Decoding finished.";
        decodeThread->quit();
    });

    connect(decodeThread, &QThread::finished, audioDecoder, &QAudioDecoder::deleteLater);
    connect(decodeThread, &QThread::finished, decodeThread, &QThread::deleteLater);

    // Decode işlemi bittikten sonra ses çıkışını başlat
    connect(audioDecoder, &QAudioDecoder::finished, this, [=]() {
        // Ses çıkış işlemini yeni bir thread'e taşıyoruz
        QThread *outputThread = new QThread;

        connect(outputThread, &QThread::started, [=]() {
            if (outputDevice) {
                qint64 written = 0;
                while (written < localData->size()) {
                    written += outputDevice->write(localData->mid(written));
                }
                qDebug() << "Playback finished.";
            }
            outputThread->quit();
        });

        connect(outputThread, &QThread::finished, outputThread, &QThread::deleteLater);

        // Ses çıkış thread'ini başlat
        outputThread->start();
    });

    // Decode thread'ini başlat
    decodeThread->start();

    audioDecoder->start();

    localData->clear();
}


void MainWindow::on_sound1_clicked()
{


    if (filename1.isEmpty()) {
        filename1 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename1.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic1.isEmpty()) {
        pic1 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic1.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename1, pic1, ui->sound1);
}


void MainWindow::on_sound2_clicked()
{

    if (filename2.isEmpty()) {
        filename2 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename2.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic2.isEmpty()) {
        pic2 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic2.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename2, pic2, ui->sound2);
}


void MainWindow::on_sound3_clicked()
{

    if (filename3.isEmpty()) {
        filename3 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename3.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic3.isEmpty()) {
        pic3 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic3.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename3, pic3, ui->sound3);
}


void MainWindow::on_sound4_clicked()
{

    if (filename4.isEmpty()) {
        filename4 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename4.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic4.isEmpty()) {
        pic4 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic4.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename4, pic4, ui->sound4);
}


void MainWindow::on_sound5_clicked()
{
    if (filename5.isEmpty()) {
        filename5 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename5.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic5.isEmpty()) {
        pic5= QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic5.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename5, pic5, ui->sound5);

}


void MainWindow::on_sound6_clicked()
{
    if (filename6.isEmpty()) {
        filename6= QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename2.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic6.isEmpty()) {
        pic6 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic6.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename6, pic6, ui->sound6);

}


void MainWindow::on_sound7_clicked()
{
    if (filename7.isEmpty()) {
        filename7 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename7.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic7.isEmpty()) {
        pic7 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic7.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename7, pic7, ui->sound7);
}


void MainWindow::on_sound8_clicked()
{
    if (filename8.isEmpty()) {
        filename8 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename8.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic8.isEmpty()) {
        pic8 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic8.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename8, pic8, ui->sound8);
}



void MainWindow::on_sound9_clicked()
{
    if (filename9.isEmpty()) {
        filename9 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename9.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic9.isEmpty()) {
        pic9 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic9.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename9, pic9, ui->sound9);
}


void MainWindow::on_sound10_clicked()
{
    if (filename10.isEmpty()) {
        filename10 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename10.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic10.isEmpty()) {
        pic10 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic10.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename10, pic10, ui->sound10);
}


void MainWindow::on_sound11_clicked()
{
    if (filename11.isEmpty()) {
        filename11 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename11.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic11.isEmpty()) {
        pic11 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic11.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename11, pic11, ui->sound11);
}


void MainWindow::on_sound12_clicked()
{
    if (filename12.isEmpty()) {
        filename12 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename12.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic12.isEmpty()) {
        pic12 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic12.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename12, pic12, ui->sound12);
}


void MainWindow::on_sound13_clicked()
{
    if (filename13.isEmpty()) {
        filename13 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename13.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic13.isEmpty()) {
        pic13 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic13.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename13, pic13, ui->sound13);
}


void MainWindow::on_sound14_clicked()
{
    if (filename14.isEmpty()) {
        filename14 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename14.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic14.isEmpty()) {
        pic14 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic14.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename14, pic14, ui->sound14);
}


void MainWindow::on_sound15_clicked()
{
    if (filename15.isEmpty()) {
        filename15 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename15.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic15.isEmpty()) {
        pic15 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic15.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename15, pic15, ui->sound15);
}


void MainWindow::on_sound16_clicked()
{
    if (filename16.isEmpty()) {
        filename16 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename16.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic16.isEmpty()) {
        pic16 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic16.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename16, pic16, ui->sound16);
}


void MainWindow::on_sound17_clicked()
{
    if (filename17.isEmpty()) {
        filename17 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename17.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic17.isEmpty()) {
        pic17 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic17.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename17, pic17, ui->sound17);
}


void MainWindow::on_sound18_clicked()
{
    if (filename18.isEmpty()) {
        filename18 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename18.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic18.isEmpty()) {
        pic18 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic18.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename18, pic18, ui->sound18);
}


void MainWindow::on_sound19_clicked()
{
    if (filename19.isEmpty()) {
        filename19 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename19.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic19.isEmpty()) {
        pic19 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic19.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename19, pic19, ui->sound19);
}


void MainWindow::on_sound20_clicked()
{
    if (filename20.isEmpty()) {
        filename20 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename20.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if (pic20.isEmpty()) {
        pic20 = QFileDialog::getOpenFileName(this, tr("Open image"), "", tr("Images (*.jpg *.png *.jpeg)"));
        if (pic20.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    playAudioNotInterrupt(filename20, pic20, ui->sound20);
}

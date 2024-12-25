#include "soundpack.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QMediaPlayer"
#include "QBuffer"
#include <QThread>



soundpack::soundpack(QWidget *parent)
    : QMainWindow{parent}
{}

void MainWindow::stopCurrentAudio() {
    if (currentDecodeThread) {
        currentDecodeThread->quit();
        currentDecodeThread->wait();
        delete currentDecodeThread;
        currentDecodeThread = nullptr;
    }

    if (currentOutputThread) {
        currentOutputThread->quit();
        currentOutputThread->wait();
        delete currentOutputThread;
        currentOutputThread = nullptr;
    }

    if (currentAudioDecoder) {
        currentAudioDecoder->stop();
        currentAudioDecoder->deleteLater();
        currentAudioDecoder = nullptr;
    }

    data.clear();
}

void MainWindow::on_sound1_clicked()
{

    stopCurrentAudio();

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

    QPixmap icon(pic1);
    QIcon buttonIcon = icon;
    ui->sound1->setIcon(buttonIcon);
    ui->sound1->setIconSize(QSize(75, 75));

    currentDecodeThread = new QThread;
    currentAudioDecoder = new QAudioDecoder;
    currentAudioDecoder->setAudioFormat(*format);
    currentAudioDecoder->moveToThread(currentDecodeThread);

    connect(currentDecodeThread, &QThread::started, currentAudioDecoder, [=]() {
        currentAudioDecoder->setSource(filename1);
        currentAudioDecoder->start();
    });

    connect(currentAudioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = currentAudioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
    });

    connect(currentAudioDecoder, &QAudioDecoder::finished, this, [=]() {
        qDebug() << "Decoding finished.";
        currentDecodeThread->quit();
    });

    connect(currentDecodeThread, &QThread::finished, currentAudioDecoder, &QAudioDecoder::deleteLater);
    connect(currentDecodeThread, &QThread::finished, currentDecodeThread, &QThread::deleteLater);

    connect(currentAudioDecoder, &QAudioDecoder::finished, this, [=]() {
        currentOutputThread = new QThread;

        connect(currentOutputThread, &QThread::started, [=]() {
            if (outputDevice) {
                qint64 written = 0;
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
                }
                qDebug() << "Playback finished.";
            }
            currentOutputThread->quit();
        });

        connect(currentOutputThread, &QThread::finished, currentOutputThread, &QThread::deleteLater);

        currentOutputThread->start();
    });

    currentDecodeThread->start();
}


void MainWindow::on_sound2_clicked()
{

    stopCurrentAudio();

    if (filename2.isEmpty()) {
        filename2 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename2.isEmpty()) {
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

    QPixmap icon(pic2);
    QIcon buttonIcon = icon;
    ui->sound2->setIcon(buttonIcon);
    ui->sound2->setIconSize(QSize(75, 75));

    currentDecodeThread = new QThread;
    currentAudioDecoder = new QAudioDecoder;
    currentAudioDecoder->setAudioFormat(*format);
    currentAudioDecoder->moveToThread(currentDecodeThread);

    connect(currentDecodeThread, &QThread::started, currentAudioDecoder, [=]() {
        currentAudioDecoder->setSource(filename2);
        currentAudioDecoder->start();
    });

    connect(currentAudioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = currentAudioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
    });

    connect(currentAudioDecoder, &QAudioDecoder::finished, this, [=]() {
        qDebug() << "Decoding finished.";
        currentDecodeThread->quit();
    });

    connect(currentDecodeThread, &QThread::finished, currentAudioDecoder, &QAudioDecoder::deleteLater);
    connect(currentDecodeThread, &QThread::finished, currentDecodeThread, &QThread::deleteLater);

    connect(currentAudioDecoder, &QAudioDecoder::finished, this, [=]() {
        currentOutputThread = new QThread;

        connect(currentOutputThread, &QThread::started, [=]() {
            if (outputDevice) {
                qint64 written = 0;
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
                }
                qDebug() << "Playback finished.";
            }
            currentOutputThread->quit();
        });

        connect(currentOutputThread, &QThread::finished, currentOutputThread, &QThread::deleteLater);

        currentOutputThread->start();
    });

    currentDecodeThread->start();
}


void MainWindow::on_sound3_clicked()
{

    stopCurrentAudio();

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

    QPixmap icon(pic3);
    QIcon buttonIcon = icon;
    ui->sound3->setIcon(buttonIcon);
    ui->sound3->setIconSize(QSize(75, 75));

    currentDecodeThread = new QThread;
    currentAudioDecoder = new QAudioDecoder;
    currentAudioDecoder->setAudioFormat(*format);
    currentAudioDecoder->moveToThread(currentDecodeThread);

    connect(currentDecodeThread, &QThread::started, currentAudioDecoder, [=]() {
        currentAudioDecoder->setSource(filename3);
        currentAudioDecoder->start();
    });

    connect(currentAudioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = currentAudioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
    });

    connect(currentAudioDecoder, &QAudioDecoder::finished, this, [=]() {
        qDebug() << "Decoding finished.";
        currentDecodeThread->quit();
    });

    connect(currentDecodeThread, &QThread::finished, currentAudioDecoder, &QAudioDecoder::deleteLater);
    connect(currentDecodeThread, &QThread::finished, currentDecodeThread, &QThread::deleteLater);

    connect(currentAudioDecoder, &QAudioDecoder::finished, this, [=]() {
        currentOutputThread = new QThread;

        connect(currentOutputThread, &QThread::started, [=]() {
            if (outputDevice) {
                qint64 written = 0;
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
                }
                qDebug() << "Playback finished.";
            }
            currentOutputThread->quit();
        });

        connect(currentOutputThread, &QThread::finished, currentOutputThread, &QThread::deleteLater);

        currentOutputThread->start();
    });

    currentDecodeThread->start();
}


void MainWindow::on_sound4_clicked()
{

    stopCurrentAudio();

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

    QPixmap icon(pic4);
    QIcon buttonIcon = icon;
    ui->sound4->setIcon(buttonIcon);
    ui->sound4->setIconSize(QSize(75, 75));

    currentDecodeThread = new QThread;
    currentAudioDecoder = new QAudioDecoder;
    currentAudioDecoder->setAudioFormat(*format);
    currentAudioDecoder->moveToThread(currentDecodeThread);

    connect(currentDecodeThread, &QThread::started, currentAudioDecoder, [=]() {
        currentAudioDecoder->setSource(filename4);
        currentAudioDecoder->start();
    });

    connect(currentAudioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = currentAudioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
    });

    connect(currentAudioDecoder, &QAudioDecoder::finished, this, [=]() {
        qDebug() << "Decoding finished.";
        currentDecodeThread->quit();
    });

    connect(currentDecodeThread, &QThread::finished, currentAudioDecoder, &QAudioDecoder::deleteLater);
    connect(currentDecodeThread, &QThread::finished, currentDecodeThread, &QThread::deleteLater);

    connect(currentAudioDecoder, &QAudioDecoder::finished, this, [=]() {
        currentOutputThread = new QThread;

        connect(currentOutputThread, &QThread::started, [=]() {
            if (outputDevice) {
                qint64 written = 0;
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
                }
                qDebug() << "Playback finished.";
            }
            currentOutputThread->quit();
        });

        connect(currentOutputThread, &QThread::finished, currentOutputThread, &QThread::deleteLater);

        currentOutputThread->start();
    });

    currentDecodeThread->start();
}


void MainWindow::on_sound5_clicked()
{
    if(filename5.isEmpty())
    {
        filename5 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));

        // Kullanıcı dosya seçmezse işlemi durdur
        if (filename5.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if(pic5.isEmpty())
    {
        pic5 = QFileDialog::getOpenFileName(this, tr("Open image"),"", tr("Images (*.jpg *.png *.jpeg)"));
        if(pic5.isEmpty())
        {
            qWarning() << "No file selected.";
            return;
        }
    }

    QPixmap icon(pic5);

    QIcon buttonIcon = icon;

    ui->sound5->setIcon(buttonIcon);

    QSize size(75,75);

    ui->sound5->setIconSize(size);


    // QThread oluşturuluyor
    QThread *decodeThread = new QThread;
    QAudioDecoder *audioDecoder = new QAudioDecoder();

    audioDecoder->setAudioFormat(*format);

    // Decoder, thread'e taşınıyor
    audioDecoder->moveToThread(decodeThread);

    connect(decodeThread, &QThread::started, audioDecoder, [=]() {
        audioDecoder->setSource(filename5);
        audioDecoder->start();
    });

    connect(audioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = audioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
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
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
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

    audioOutput->suspend();

    data.clear();
}


void MainWindow::on_sound6_clicked()
{
    if(filename6.isEmpty())
    {
        filename6 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));

        // Kullanıcı dosya seçmezse işlemi durdur
        if (filename6.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if(pic6.isEmpty())
    {
        pic6 = QFileDialog::getOpenFileName(this, tr("Open image"),"", tr("Images (*.jpg *.png *.jpeg)"));
        if(pic6.isEmpty())
        {
            qWarning() << "No file selected.";
            return;
        }
    }

    QPixmap icon(pic6);

    QIcon buttonIcon = icon;

    ui->sound6->setIcon(buttonIcon);

    QSize size(75,75);

    ui->sound6->setIconSize(size);


    // QThread oluşturuluyor
    QThread *decodeThread = new QThread;
    QAudioDecoder *audioDecoder = new QAudioDecoder();

    audioDecoder->setAudioFormat(*format);

    // Decoder, thread'e taşınıyor
    audioDecoder->moveToThread(decodeThread);

    connect(decodeThread, &QThread::started, audioDecoder, [=]() {
        audioDecoder->setSource(filename6);
        audioDecoder->start();
    });

    connect(audioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = audioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
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
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
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

    audioOutput->suspend();

    data.clear();
}


void MainWindow::on_sound7_clicked()
{
    if(filename7.isEmpty())
    {
        filename7 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));

        // Kullanıcı dosya seçmezse işlemi durdur
        if (filename7.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if(pic7.isEmpty())
    {
        pic7 = QFileDialog::getOpenFileName(this, tr("Open image"),"", tr("Images (*.jpg *.png *.jpeg)"));
        if(pic7.isEmpty())
        {
            qWarning() << "No file selected.";
            return;
        }
    }

    QPixmap icon(pic7);

    QIcon buttonIcon = icon;

    ui->sound7->setIcon(buttonIcon);

    QSize size(75,75);

    ui->sound7->setIconSize(size);


    // QThread oluşturuluyor
    QThread *decodeThread = new QThread;
    QAudioDecoder *audioDecoder = new QAudioDecoder();

    audioDecoder->setAudioFormat(*format);

    // Decoder, thread'e taşınıyor
    audioDecoder->moveToThread(decodeThread);

    connect(decodeThread, &QThread::started, audioDecoder, [=]() {
        audioDecoder->setSource(filename7);
        audioDecoder->start();
    });

    connect(audioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = audioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
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
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
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

    audioOutput->suspend();

    data.clear();
}


void MainWindow::on_sound8_clicked()
{
    if(filename8.isEmpty())
    {
        filename8 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));

        // Kullanıcı dosya seçmezse işlemi durdur
        if (filename8.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if(pic8.isEmpty())
    {
        pic8 = QFileDialog::getOpenFileName(this, tr("Open image"),"", tr("Images (*.jpg *.png *.jpeg)"));
        if(pic8.isEmpty())
        {
            qWarning() << "No file selected.";
            return;
        }
    }

    QPixmap icon(pic8);

    QIcon buttonIcon = icon;

    ui->sound8->setIcon(buttonIcon);

    QSize size(75,75);

    ui->sound8->setIconSize(size);


    // QThread oluşturuluyor
    QThread *decodeThread = new QThread;
    QAudioDecoder *audioDecoder = new QAudioDecoder();

    audioDecoder->setAudioFormat(*format);

    // Decoder, thread'e taşınıyor
    audioDecoder->moveToThread(decodeThread);

    connect(decodeThread, &QThread::started, audioDecoder, [=]() {
        audioDecoder->setSource(filename8);
        audioDecoder->start();
    });

    connect(audioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = audioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
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
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
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

    audioOutput->suspend();

    data.clear();
}



void MainWindow::on_sound9_clicked()
{
    if(filename9.isEmpty())
    {
        filename9 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));

        // Kullanıcı dosya seçmezse işlemi durdur
        if (filename9.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if(pic9.isEmpty())
    {
        pic9 = QFileDialog::getOpenFileName(this, tr("Open image"),"", tr("Images (*.jpg *.png *.jpeg)"));
        if(pic9.isEmpty())
        {
            qWarning() << "No file selected.";
            return;
        }
    }

    QPixmap icon(pic9);

    QIcon buttonIcon = icon;

    ui->sound9->setIcon(buttonIcon);

    QSize size(75,75);

    ui->sound9->setIconSize(size);


    // QThread oluşturuluyor
    QThread *decodeThread = new QThread;
    QAudioDecoder *audioDecoder = new QAudioDecoder();

    audioDecoder->setAudioFormat(*format);

    // Decoder, thread'e taşınıyor
    audioDecoder->moveToThread(decodeThread);

    connect(decodeThread, &QThread::started, audioDecoder, [=]() {
        audioDecoder->setSource(filename9);
        audioDecoder->start();
    });

    connect(audioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = audioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
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
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
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

    audioOutput->suspend();

    data.clear();
}


void MainWindow::on_sound10_clicked()
{
    if(filename10.isEmpty())
    {
        filename10 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));

        // Kullanıcı dosya seçmezse işlemi durdur
        if (filename10.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if(pic10.isEmpty())
    {
        pic10 = QFileDialog::getOpenFileName(this, tr("Open image"),"", tr("Images (*.jpg *.png *.jpeg)"));
        if(pic10.isEmpty())
        {
            qWarning() << "No file selected.";
            return;
        }
    }

    QPixmap icon(pic10);

    QIcon buttonIcon = icon;

    ui->sound10->setIcon(buttonIcon);

    QSize size(75,75);

    ui->sound10->setIconSize(size);


    // QThread oluşturuluyor
    QThread *decodeThread = new QThread;
    QAudioDecoder *audioDecoder = new QAudioDecoder();

    audioDecoder->setAudioFormat(*format);

    // Decoder, thread'e taşınıyor
    audioDecoder->moveToThread(decodeThread);

    connect(decodeThread, &QThread::started, audioDecoder, [=]() {
        audioDecoder->setSource(filename10);
        audioDecoder->start();
    });

    connect(audioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = audioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
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
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
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

    audioOutput->suspend();

    data.clear();
}


void MainWindow::on_sound11_clicked()
{
    if(filename11.isEmpty())
    {
        filename11 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));

        // Kullanıcı dosya seçmezse işlemi durdur
        if (filename11.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if(pic11.isEmpty())
    {
        pic11 = QFileDialog::getOpenFileName(this, tr("Open image"),"", tr("Images (*.jpg *.png *.jpeg)"));
        if(pic11.isEmpty())
        {
            qWarning() << "No file selected.";
            return;
        }
    }

    QPixmap icon(pic11);

    QIcon buttonIcon = icon;

    ui->sound11->setIcon(buttonIcon);

    QSize size(75,75);

    ui->sound11->setIconSize(size);


    // QThread oluşturuluyor
    QThread *decodeThread = new QThread;
    QAudioDecoder *audioDecoder = new QAudioDecoder();

    audioDecoder->setAudioFormat(*format);

    // Decoder, thread'e taşınıyor
    audioDecoder->moveToThread(decodeThread);

    connect(decodeThread, &QThread::started, audioDecoder, [=]() {
        audioDecoder->setSource(filename10);
        audioDecoder->start();
    });

    connect(audioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = audioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
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
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
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

    audioOutput->suspend();

    data.clear();
}


void MainWindow::on_sound12_clicked()
{
    if(filename12.isEmpty())
    {
        filename12 = QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));

        // Kullanıcı dosya seçmezse işlemi durdur
        if (filename12.isEmpty()) {
            qWarning() << "No file selected.";
            return;
        }
    }

    if(pic12.isEmpty())
    {
        pic12 = QFileDialog::getOpenFileName(this, tr("Open image"),"", tr("Images (*.jpg *.png *.jpeg)"));
        if(pic12.isEmpty())
        {
            qWarning() << "No file selected.";
            return;
        }
    }

    QPixmap icon(pic12);

    QIcon buttonIcon = icon;

    ui->sound12->setIcon(buttonIcon);

    QSize size(75,75);

    ui->sound12->setIconSize(size);


    // QThread oluşturuluyor
    QThread *decodeThread = new QThread;
    QAudioDecoder *audioDecoder = new QAudioDecoder();

    audioDecoder->setAudioFormat(*format);

    // Decoder, thread'e taşınıyor
    audioDecoder->moveToThread(decodeThread);

    connect(decodeThread, &QThread::started, audioDecoder, [=]() {
        audioDecoder->setSource(filename12);
        audioDecoder->start();
    });

    connect(audioDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = audioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        data.append(pcmData);
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
                while (written < data.size()) {
                    written += outputDevice->write(data.mid(written));
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

    audioOutput->suspend();

    data.clear();
}


void MainWindow::on_sound13_clicked()
{

}


void MainWindow::on_sound14_clicked()
{

}


void MainWindow::on_sound15_clicked()
{

}


void MainWindow::on_sound16_clicked()
{

}


void MainWindow::on_sound17_clicked()
{

}


void MainWindow::on_sound18_clicked()
{

}


void MainWindow::on_sound19_clicked()
{

}


void MainWindow::on_sound20_clicked()
{

}

#include "soundpack.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QMediaPlayer"
#include "QBuffer"
#include <QThread>
#include <QMutex>
#include "circularbuffer.h"


soundpack::soundpack(QWidget *parent)
    : QMainWindow{parent}
{
    
    QList<QPushButton*> buttons =
        parent->findChildren<QPushButton*>();

    for (auto btn : buttons) {
        btn->setProperty("class", "soundPack");
        btn->style()->polish(btn);
    }
}

void MainWindow::preloadAudio(const QString &filename)
{
    if (audioCache.contains(filename) && audioCache[filename]->isLoaded) {
        return; // Already loaded
    }

    if (!audioCache.contains(filename)) {
        audioCache[filename] = new AudioCache();
    }

    AudioCache* cache = audioCache[filename];
    
    QAudioDecoder *preloadDecoder = new QAudioDecoder();
    preloadDecoder->setAudioFormat(*format);
    preloadDecoder->setSource(filename);

    connect(preloadDecoder, &QAudioDecoder::bufferReady, this, [=]() {
        const QAudioBuffer buffer = preloadDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        
        QMutexLocker locker(&cache->mutex);
        cache->audioData.append(pcmData);
    });

    connect(preloadDecoder, &QAudioDecoder::finished, this, [=]() {
        QMutexLocker locker(&cache->mutex);
        cache->isLoaded = true;
        qDebug() << "Preloaded audio:" << filename << "Size:" << cache->audioData.size() << "bytes";
        preloadDecoder->deleteLater();
        
        preloadCount++;
        if (preloadCount >= preloadTotal) {
            emit preloadFinished();
        }
    });

    preloadDecoder->start();
}

void MainWindow::stopCurrentSound()
{
    std::lock_guard<std::mutex> lock(currentSoundMutex);
    
    // Set interruption flag to stop audio writing
    soundInterrupted = true;
    
    if (currentPlayingButton) {
        // Reset previous button state - move UI updates to main thread
        QPushButton* buttonToUpdate = currentPlayingButton;
        QMetaObject::invokeMethod(buttonToUpdate, [buttonToUpdate]() {
            buttonToUpdate->setProperty("playing", false);
            buttonToUpdate->setProperty("selected", false);
            buttonToUpdate->style()->polish(buttonToUpdate);
        }, Qt::QueuedConnection);
        currentPlayingButton = nullptr;
    }
    
    if (currentOutputThread) {
        currentOutputThread->quit();
        if (!currentOutputThread->wait(1000)) { // 1 saniye bekle
            qWarning() << "Thread did not stop gracefully, terminating...";
            currentOutputThread->terminate(); // Son çare
            currentOutputThread->wait(500); // 500ms daha bekle
        }
        currentOutputThread->deleteLater();
        currentOutputThread = nullptr;
    }
    
    // Small delay to ensure resources are fully cleaned up
    QThread::msleep(50);
    
    // Reset interruption flag after stopping
    soundInterrupted = false;
}

void MainWindow::playAudioNotInterrupt(const QString &filename, const QString &picPath, QPushButton *button)
{
    // Prevent rapid successive presses
    static QDateTime lastPressTime;
    QDateTime currentTime = QDateTime::currentDateTime();
    if (lastPressTime.isValid() && lastPressTime.msecsTo(currentTime) < 100) {
        return; // Ignore presses less than 100ms apart
    }
    lastPressTime = currentTime;

    // Stop currently playing sound first
    stopCurrentSound();

    // Clear any residual data
    data.clear();

    // Check if audio is cached
    if (audioCache.contains(filename) && audioCache[filename]->isLoaded) {
        AudioCache* cache = audioCache[filename];
        QMutexLocker locker(&cache->mutex);

        // Use cached data directly
        QByteArray *localData = new QByteArray(cache->audioData);

        // Move UI updates to main thread
        QMetaObject::invokeMethod(button, [button, picPath]() {
            QPixmap icon(picPath);
            QIcon buttonIcon = icon;
            button->setIcon(buttonIcon);
            QSize size(75,75);
            button->setIconSize(size);
            
            button->setProperty("selected", true);
            button->style()->polish(button);
            button->setProperty("playing", true);
            button->style()->polish(button);
        }, Qt::QueuedConnection);

        // Set current playing sound tracking
        {
            std::lock_guard<std::mutex> lock(currentSoundMutex);
            currentPlayingButton = button;
        }

        // Play cached audio directly
        QThread *outputThread = new QThread;
        {
            std::lock_guard<std::mutex> lock(currentSoundMutex);
            currentOutputThread = outputThread;
        }

        // Thread önceliğini artır - UI thread'i ile çakışmayı önle
        // setPriority thread başlatıldıktan sonra çağrılmalı
        // outputThread->setPriority(QThread::HighPriority);

        // Capture test button state before starting thread (thread safety)
        bool isTestButtonChecked = ui->testButton->isChecked();

        connect(outputThread, &QThread::started, [=, localData = localData]() {
            // Thread başladıktan sonra önceliği ayarla
            QThread::currentThread()->setPriority(QThread::HighPriority);
            
            std::lock_guard<std::mutex> lock(outputDeviceMutex);
            if (outputDevice) {
                qDebug() << "Soundpack: Output device is available, starting playback...";
                qDebug() << "Soundpack: Test button state:" << isTestButtonChecked;
                qDebug() << "Soundpack: Output device open:" << outputDevice->isOpen();
                qDebug() << "Soundpack: Output device type:" << outputDevice->metaObject()->className();
                
                qint64 written = 0;
                // 48kHz stereo: 48000 samples/sec * 2 channels * 2 bytes = 192000 bytes/sec
                // 256 samples chunks: 1024 bytes for optimal balance
                const int chunkSize = 1024; // 256 samples at 48kHz stereo
                const int sleepMs = 5; // 5ms delay for optimal playback

                QElapsedTimer timer;
                timer.start();

                while (written < localData->size() && !soundInterrupted) {
                    QByteArray chunk = localData->mid(written, chunkSize);

                    // Always increment written by chunk size for timing
                    written += chunkSize;




                    // Paralel pipeline: Soundpack sesini AudioPipeline'a gönder
                    // AudioPipeline kendi timer'ı ile processBuffers çağırır
                    if (audioPipeline) {
                        audioPipeline->writeSoundpackAudio(chunk);
                        // processBuffers() çağrma - timer zaten yapıyor
                    }


                    // Write to outputDevice only if test button was checked when thread started
                    if (isTestButtonChecked) {
                        // DISABLED: Use only AudioPipeline for output to avoid threading issues
                        // All output is now handled by AudioPipeline processBuffers()
                        
                        // Debug mesajlarını azalt
                        static int disabledCounter = 0;
                        if (++disabledCounter % 1000 == 0) {
                            qDebug() << "Soundpack: OutputDevice writing disabled - using AudioPipeline only";
                        }
                    }

                    // Recording için soundpack sesini AudioPipeline üzerinden gönder
                    // Bu, takılmayı önler ve proper mixing sağlar
                    if (isRecording && audioPipeline) {
                        // Soundpack sesini AudioPipeline'a gönder, recording sinyali oradan emit edilir
                        audioPipeline->writeSoundpackAudio(chunk);
                        // processBuffers() çağrma - timer zaten yapıyor
                    }

                    // Real-time timing control - daha az CPU yükü için optimize
                    qint64 expectedTime = (written * 1000) / 192000; // ms elapsed
                    qint64 actualTime = timer.elapsed();

                    if (actualTime < expectedTime) {
                        int sleepTime = expectedTime - actualTime;
                        if (sleepTime > 5) {
                            QThread::msleep(sleepTime);
                        } else if (sleepTime > 0) {
                            QThread::usleep(sleepTime * 500);
                        }
                        // yieldCurrentThread() kaldırıldı - CPU yükünü azaltmak için
                    }
                }

                if (!soundInterrupted) {
                    qDebug() << "Cached playback finished.";
                } else {
                    qDebug() << "Cached playback interrupted.";
                }

                // Reset button state only if this is still the current playing sound
                {
                    std::lock_guard<std::mutex> currentLock(currentSoundMutex);
                    if (currentPlayingButton == button) {
                        // Move UI updates to main thread to avoid timer errors
                        QMetaObject::invokeMethod(button, [button]() {
                            button->setProperty("playing", false);
                            button->style()->polish(button);
                        }, Qt::QueuedConnection);
                        currentPlayingButton = nullptr;
                        currentOutputThread = nullptr;
                    }
                }
            }
            outputThread->quit();
        });

        connect(outputThread, &QThread::finished, this, [localData]() {
            delete localData;
        });
        connect(outputThread, &QThread::finished, outputThread, &QThread::deleteLater);

        outputThread->start();
        return;
    }

    // Original decoding path for non-cached audio
    QByteArray *localData = new QByteArray;

    // Move UI updates to main thread
    QMetaObject::invokeMethod(button, [button, picPath]() {
        QPixmap icon(picPath);
        QIcon buttonIcon = icon;
        button->setIcon(buttonIcon);
        QSize size(75,75);
        button->setIconSize(size);
        
        // Seçili slot olarak işaretle
        button->setProperty("selected", true);
        button->style()->polish(button);
    }, Qt::QueuedConnection);

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

    connect(audioDecoder, &QAudioDecoder::bufferReady, this, [=, localData = localData]() {
        const QAudioBuffer buffer = audioDecoder->read();
        QByteArray pcmData(reinterpret_cast<const char *>(buffer.data<void>()), buffer.byteCount());
        localData->append(pcmData);
    });

    connect(audioDecoder, &QAudioDecoder::finished, this, [=]() {
        qDebug() << "Decoding finished.";
        decodeThread->quit();
    });

    connect(decodeThread, &QThread::finished, this, [localData]() {
        delete localData;
    });
    connect(decodeThread, &QThread::finished, audioDecoder, &QAudioDecoder::deleteLater);
    connect(decodeThread, &QThread::finished, decodeThread, &QThread::deleteLater);

    // Decode işlemi bittikten sonra ses çıkışını başlat
    connect(audioDecoder, &QAudioDecoder::finished, this, [=, localData = localData]() {
        // Move UI updates to main thread
        QMetaObject::invokeMethod(button, [button]() {
            button->setProperty("playing", true);
            button->style()->polish(button);
        }, Qt::QueuedConnection);

        // Set current playing sound tracking
        {
            std::lock_guard<std::mutex> lock(currentSoundMutex);
            currentPlayingButton = button;
        }

        // Ses çıkış işlemini yeni bir thread'e taşıyoruz
        QThread *outputThread = new QThread;
        {
            std::lock_guard<std::mutex> lock(currentSoundMutex);
            currentOutputThread = outputThread;
        }

        // Thread önceliğini artır - UI thread'i ile çakışmayı önle
        // setPriority thread başlatıldıktan sonra çağrılmalı
        // outputThread->setPriority(QThread::HighPriority);

        // Capture test button state before starting thread (thread safety)
        bool isTestButtonChecked = ui->testButton->isChecked();

        connect(outputThread, &QThread::started, [=, localData = localData]() {
            // Thread başladıktan sonra önceliği ayarla
            QThread::currentThread()->setPriority(QThread::HighPriority);
            
            if (outputDevice) {
                qDebug() << "Soundpack: Output device is available, starting playback...";
                qDebug() << "Soundpack: Test button state:" << isTestButtonChecked;
                qDebug() << "Soundpack: Output device open:" << outputDevice->isOpen();
                qDebug() << "Soundpack: Output device type:" << outputDevice->metaObject()->className();
                
                qint64 written = 0;
                // 48kHz stereo: 48000 samples/sec * 2 channels * 2 bytes = 192000 bytes/sec
                // 256 samples chunks: 1024 bytes for optimal balance
                const int chunkSize = 1024; // 256 samples at 48kHz stereo

                QElapsedTimer timer;
                timer.start();

                while (written < localData->size() && !soundInterrupted) {
                    QByteArray chunk = localData->mid(written, chunkSize);

                    // Always increment written by chunk size for timing
                    written += chunkSize;




                    // Paralel pipeline: Soundpack sesini AudioPipeline'a gönder
                    // AudioPipeline kendi timer'ı ile processBuffers çağırır
                    if (audioPipeline) {
                        audioPipeline->writeSoundpackAudio(chunk);
                        // processBuffers() çağrma - timer zaten yapıyor
                    }


                    // Write to outputDevice only if test button was checked when thread started
                    if (isTestButtonChecked) {
                        // DISABLED: Use only AudioPipeline for output to avoid threading issues
                        // All output is now handled by AudioPipeline processBuffers()
                        
                        // Debug mesajlarını azalt
                        static int disabledCounter = 0;
                        if (++disabledCounter % 1000 == 0) {
                            qDebug() << "Soundpack: OutputDevice writing disabled - using AudioPipeline only";
                        }
                    }

                    // Recording için soundpack sesini AudioPipeline üzerinden gönder
                    // Bu, takılmayı önler ve proper mixing sağlar
                    if (isRecording && audioPipeline) {
                        // Soundpack sesini AudioPipeline'a gönder, recording sinyali oradan emit edilir
                        audioPipeline->writeSoundpackAudio(chunk);
                        // processBuffers() çağrma - timer zaten yapıyor
                    }

                    // Real-time timing control - daha az CPU yükü için optimize
                    qint64 expectedTime = (written * 1000) / 192000; // ms elapsed
                    qint64 actualTime = timer.elapsed();

                    if (actualTime < expectedTime) {
                        int sleepTime = expectedTime - actualTime;
                        if (sleepTime > 5) {
                            QThread::msleep(sleepTime);
                        } else if (sleepTime > 0) {
                            QThread::usleep(sleepTime * 500);
                        }
                        // yieldCurrentThread() kaldırıldı - CPU yükünü azaltmak için
                    }
                }

                if (!soundInterrupted) {
                    qDebug() << "Playback finished.";
                } else {
                    qDebug() << "Playback interrupted.";
                }

                // Reset button state only if this is still the current playing sound
                {
                    std::lock_guard<std::mutex> currentLock(currentSoundMutex);
                    if (currentPlayingButton == button) {
                        // Move UI updates to main thread to avoid timer errors
                        QMetaObject::invokeMethod(button, [button]() {
                            button->setProperty("playing", false);
                            button->style()->polish(button);
                        }, Qt::QueuedConnection);
                        currentPlayingButton = nullptr;
                        currentOutputThread = nullptr;
                    }
                }
            }
            outputThread->quit();
        });

        connect(outputThread, &QThread::finished, this, [localData]() {
            delete localData;
        });
        connect(outputThread, &QThread::finished, outputThread, &QThread::deleteLater);

        // Ses çıkış thread'ini başlat
        outputThread->start();
    });

    // Decode thread'ini başlat
    decodeThread->start();
}



void MainWindow::on_sound1_clicked()
{
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }


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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

    if (filename6.isEmpty()) {
        filename6= QFileDialog::getOpenFileName(this, tr("Open MP3 File"), "", tr("Audio Files (*.wav)"));
        if (filename6.isEmpty()) {
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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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
    // SoundPack başlamadan önce buffer temizliği - mix kalitesi için
    if (audioPipeline) {
        audioPipeline->clearSoundpackBuffer();
    }

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

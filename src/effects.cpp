#include "effects.h"
#include "ui_mainwindow.h"
#include <QTime>
#include "mainwindow.h"
#include "psola.h"

// Efekt yönetimi için yardımcı fonksiyon
void MainWindow::stopAllEffects()
{
    // Tüm efekt butonlarını durdur ve text'lerini geri getir
    ui->robotButton->setChecked(false);
    ui->robotButton->setText("Robot sesi");

    ui->bananaButton->setChecked(false);
    ui->bananaButton->setText("Çocuk Sesi");

    ui->devilButton->setChecked(false);
    ui->devilButton->setText("Canavar sesi");

    ui->femaleButton->setChecked(false);
    ui->femaleButton->setText("Kadın sesi");

    ui->combineButton->setChecked(false);
    ui->combineButton->setText("Birleşik ses");

    ui->ekoButton->setChecked(false);
    ui->ekoButton->setText("Eko");

    // Mevcut bağlantıları kopar
    if (inputDevice) {
        disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);

        // Test durumuna göre bağlantı kur
        if (ui->testButton->isChecked()) {
            // Test modu: Efektli sesi hem virtual output'a (mix için) hem normal output'a gönder
            connect(inputDevice, &QIODevice::readyRead, this, [=](){
                data = inputDevice->readAll();
                progressBarOutput();

                // Emit signal for recording when recording is active
                if (isRecording) {
                    qDebug() << "EMITTING SIGNAL (EFFECT-TEST): Audio size:" << data.size() << "bytes";
                    emit audioDataReady(data);
                }

                // Virtual output'a efektli ses olarak gönder (soundpack ile mix için)
                if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                    if (audioPipeline) {
                        audioPipeline->writeEffectsAudio(data);
                    } else {
                        virtualOutputDevice->write(data);
                    }
                    qDebug() << "Effects stopped (test mode): Writing effect-free audio to virtual output, size:" << data.size() << "bytes";
                } else {
                    qWarning() << "Effects stopped (test mode): Virtual output device not available!";
                }

                // Test modunda normal output'a da doğrudan gönder (kullanıcı duymalı)
                if (outputDevice && outputDevice->isOpen()) {
                    outputDevice->write(data);
                }
            });
        } else {
            // Normal mod: Sadece virtual output'a gönder (normal output'a gönderme)
            connect(inputDevice, &QIODevice::readyRead, this, [=](){
                data = inputDevice->readAll();
                progressBarOutput();

                // Emit signal for recording when recording is active
                if (isRecording) {
                    qDebug() << "EMITTING SIGNAL (EFFECT-NORMAL): Audio size:" << data.size() << "bytes";
                    emit audioDataReady(data);
                }

                // Normal output'a gönderme (test modu kapalı olduğu için)

                // Sadece virtual output'a gönder
                if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                    // AudioPipeline kullanarak gönder
                    if (audioPipeline) {
                        audioPipeline->writeInputAudio(data);
                    } else {
                        virtualOutputDevice->write(data);
                    }
                    qDebug() << "Effects stopped: Writing effect-free audio to virtual output, size:" << data.size() << "bytes";
                } else {
                    qWarning() << "Effects stopped: Virtual output device not available!";
                }
            });
        }
    }

    data.clear();
    usingEffects = true; // Normal voice changer'a geri dön
}


effects::effects(QWidget *parent)
    : QMainWindow{parent}

{

}

void MainWindow::processToBananaVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < sizeof(int16_t)) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToBananaVoice";
        return;
    }

    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    // Ensure we have enough samples for PSOLA processing
    if (sampleCount < 100) {
        return;
    }

    m_psola.process(samples, sampleCount, format->sampleRate(),
                 PSOLA::getDefaultPitchFactor(PSOLA::BANANA),
                 PSOLA::BANANA);
}

void MainWindow::processToRobotVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < sizeof(int16_t)) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToRobotVoice";
        return;
    }

    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    // Ensure we have enough samples for PSOLA processing
    if (sampleCount < 100) {
        return;
    }

    m_psola.process(samples, sampleCount, format->sampleRate(),
                 PSOLA::getDefaultPitchFactor(PSOLA::ROBOT),
                 PSOLA::ROBOT);

    // Add robot-specific characteristics (light square wave modulation)
    for (int i = 0; i < sampleCount; ++i) {
        // Very light square wave modulation for robotic feel
        double modulator = 0.05 * sin(2.0 * M_PI * 50 * i / format->sampleRate()); // 50Hz
        samples[i] = static_cast<int16_t>(samples[i] * (1.0 + modulator));
    }
}

void MainWindow::processToDevilVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < sizeof(int16_t)) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToDevilVoice";
        return;
    }

    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    // Ensure we have enough samples for PSOLA processing
    if (sampleCount < 100) {
        return;
    }

    m_psola.process(samples, sampleCount, format->sampleRate(),
                 PSOLA::getDefaultPitchFactor(PSOLA::DEVIL),
                 PSOLA::DEVIL);
}

void MainWindow::processToFemaleVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < sizeof(int16_t)) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToFemaleVoice";
        return;
    }

    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    // Ensure we have enough samples for PSOLA processing
    if (sampleCount < 100) {
        return;
    }

    m_psola.process(samples, sampleCount, format->sampleRate(),
                 PSOLA::getDefaultPitchFactor(PSOLA::FEMALE),
                 PSOLA::FEMALE);
}

void MainWindow::processToCombineVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < sizeof(int16_t)) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToCombineVoice";
        return;
    }

    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    // Ensure we have enough samples for PSOLA processing
    if (sampleCount < 100) {
        return;
    }

    m_psola.process(samples, sampleCount, format->sampleRate(),
                 PSOLA::getDefaultPitchFactor(PSOLA::COMBINE),
                 PSOLA::COMBINE);
}

void MainWindow::processToEkoVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < sizeof(int16_t)) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToEkoVoice";
        return;
    }

    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    // Ensure we have enough samples for PSOLA processing
    if (sampleCount < 100) {
        return;
    }

    m_psola.process(samples, sampleCount, format->sampleRate(),
                  PSOLA::getDefaultPitchFactor(PSOLA::EKO),
                  PSOLA::EKO);
}


void MainWindow::on_robotButton_clicked(bool checked)
{
    if(checked)
    {
        // Diğer tüm efektleri durdur
        stopAllEffects();

        // Robot efektini başlat
        ui->robotButton->setChecked(true);
        ui->robotButton->setText("Stop");

        // Audio input'u kontrol et ama kapatma
        if(audioInput)
        {
            // Mevcut bağlantıyı kopar
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);

            // Yeni efekt bağlantısı kur
            connect(inputDevice, &QIODevice::readyRead, this, [=](){
                data = inputDevice->readAll();
                processToRobotVoice(data);
                progressBarOutput();
                // Emit signal for recording when recording is active
                if (isRecording) {
                    emit audioDataReady(data);
                } // Progress bar'ı güncelle

                // Emit signal for recording when recording is active (after effects)
                if (isRecording) {
                    qDebug() << "EMITTING SIGNAL (ROBOT): Processed audio size:" << data.size() << "bytes";
                    emit audioDataReady(data);
                }

                // Her zaman virtual output'a gönder (Cable Input)
                if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                    // AudioPipeline kullanarak efekti sesi gönder
                    if (audioPipeline) {
                        audioPipeline->writeEffectsAudio(data);
                    } else {
                        virtualOutputDevice->write(data);
                    }
                }

                // SADECE test modunda fiziksel output'a gönder
                if (ui->testButton->isChecked()) {
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                }
            });
        }
        else {
            qWarning() << "Audio devices are not properly initialized.";
        }

        usingEffects = false; // Efekt modu
        qDebug() << "robot effect started.";
    }
    else
    {
        ui->robotButton->setText("Robot sesi");

        // Sadece robot efektini durdur
        if (inputDevice)
        {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            // Normal bağlantıyı geri kur (test veya normal mode)
            if (ui->testButton->isChecked()) {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Önce virtual output'a gönder (mix için)
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak gönder
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Sonra normal output'a gönder
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                });
            } else {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Test modu kapalı: Sadece virtual output'a efektli ses gönder
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak efekti sesi gönder (mix için)
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Normal output'a gönderme (test modu kapalı)
                });
            }
        }

        data.clear();
        usingEffects = true; // Normal voice changer'a geri dön
        qDebug() << "robot effect stopped.";
    }
}


void MainWindow::on_bananaButton_clicked(bool checked)
{
    if (checked) {
        // Diğer tüm efektleri durdur
        stopAllEffects();

        // Banana efektini başlat
        ui->bananaButton->setChecked(true);
        ui->bananaButton->setText("Stop");

        if (audioInput) {
            // Mevcut bağlantıyı kopar
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);

            // Yeni efekt bağlantısı kur
            connect(inputDevice, &QIODevice::readyRead, this, [=]() {
                data = inputDevice->readAll();
                processToBananaVoice(data);
                progressBarOutput();
                // Emit signal for recording when recording is active
                if (isRecording) {
                    emit audioDataReady(data);
                } // Progress bar'ı güncelle

                // Her zaman virtual output'a gönder (Cable Input)
                if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                    // AudioPipeline kullanarak gönder
                    if (audioPipeline) {
                        audioPipeline->writeEffectsAudio(data);
                    } else {
                        virtualOutputDevice->write(data);
                    }
                }

                // SADECE test modunda fiziksel output'a gönder
                if (ui->testButton->isChecked()) {
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                }
            });

            usingEffects = false;
            qDebug() << "Child voice effect started.";
        }
        else {
            qWarning() << "Audio devices are not properly initialized.";
        }
    } else {
        ui->bananaButton->setText("Çocuk Sesi");

        // Sadece banana efektini durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            // Normal bağlantıyı geri kur (test veya normal mode)
            if (ui->testButton->isChecked()) {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Önce virtual output'a gönder (mix için)
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak gönder
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Sonra normal output'a gönder
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                });
            } else {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Test modu kapalı: Sadece virtual output'a temiz ses gönder
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak temiz sesi gönder (mix için)
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Normal output'a gönderme (test modu kapalı)
                });
            }
        }

        data.clear();
        usingEffects = true;
        qDebug() << "Child voice effect stopped.";
    }
}

void MainWindow::on_devilButton_clicked(bool checked)
{
    if(checked)
    {
        // Diğer tüm efektleri durdur
        stopAllEffects();
        
        // Devil efektini başlat
        ui->devilButton->setChecked(true);
        ui->devilButton->setText("Stop");

        if(audioInput)
        {
            // Mevcut bağlantıyı kopar
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            // Yeni efekt bağlantısı kur
            connect(inputDevice, &QIODevice::readyRead, this, [=](){
                data = inputDevice->readAll();
                processToDevilVoice(data);
                progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    } // Progress bar'ı güncelle
                
                // Her zaman virtual output'a gönder (Cable Input)
                if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                    // AudioPipeline kullanarak gönder
                    if (audioPipeline) {
                        audioPipeline->writeEffectsAudio(data);
                    } else {
                        virtualOutputDevice->write(data);
                    }
                }
                
                // SADECE test modunda fiziksel output'a gönder
                if (ui->testButton->isChecked()) {
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                }
            });

            usingEffects = false;
            qDebug() << "Devil voice effect started.";
        }
        else
        {
            qWarning() << "Audio devices are not properly initialized.";
        }
    }
    else
    {
        ui->devilButton->setText("Canavar sesi");

        // Sadece devil efektini durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            // Normal bağlantıyı geri kur (test veya normal mode)
            if (ui->testButton->isChecked()) {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Önce virtual output'a gönder (mix için)
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak gönder
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Sonra normal output'a gönder
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                });
            } else {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Test modu kapalı: Sadece virtual output'a efektli ses gönder
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak efekti sesi gönder (mix için)
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Normal output'a gönderme (test modu kapalı)
                });
            }
        }

        data.clear();
        usingEffects = true;
        qDebug() << "Devil voice effect stopped.";
    }
}

void MainWindow::on_ekoButton_clicked(bool checked)
{
    if(checked)
    {
        // Diğer tüm efektleri durdur
        stopAllEffects();
        
        // Eko efektini başlat
        ui->ekoButton->setChecked(true);
        ui->ekoButton->setText("Stop");

        if(audioInput)
        {
            // Mevcut bağlantıyı kopar
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            // Yeni efekt bağlantısı kur
            connect(inputDevice, &QIODevice::readyRead, this, [=] {
                data = inputDevice->readAll();
                processToEkoVoice(data);
                progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    } // Progress bar'ı güncelle
                
                // Her zaman virtual output'a gönder (Cable Input)
                if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                    // AudioPipeline kullanarak gönder
                    if (audioPipeline) {
                        audioPipeline->writeEffectsAudio(data);
                    } else {
                        virtualOutputDevice->write(data);
                    }
                }
                
                // SADECE test modunda fiziksel output'a gönder
                if (ui->testButton->isChecked()) {
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                }
            });

            usingEffects = false;
            qDebug() << "eko voice effect started.";
        }
        else
        {
            qWarning() << "Audio devices are not properly initialized.";
        }
    }
    else
    {
        ui->ekoButton->setText("Eko");

        // Sadece eko efektini durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            // Normal bağlantıyı geri kur (test veya normal mode)
            if (ui->testButton->isChecked()) {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Önce virtual output'a gönder (mix için)
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak gönder
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Sonra normal output'a gönder
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                });
            } else {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Test modu kapalı: Sadece virtual output'a efektli ses gönder
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak efekti sesi gönder (mix için)
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Normal output'a gönderme (test modu kapalı)
                });
            }
        }

        data.clear();
        usingEffects = true;
        qDebug() << "eko effect stopped.";
    }
}

void MainWindow::on_femaleButton_clicked(bool checked)
{
    if(checked)
    {
        // Diğer tüm efektleri durdur
        stopAllEffects();
        
        // Female efektini başlat
        ui->femaleButton->setChecked(true);
        ui->femaleButton->setText("Stop");

        if(audioInput)
        {
            // Mevcut bağlantıyı kopar
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            // Yeni efekt bağlantısı kur
            connect(inputDevice, &QIODevice::readyRead, this, [=] {
                data = inputDevice->readAll();
                processToFemaleVoice(data);
                progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    } // Progress bar'ı güncelle
                
                // Her zaman virtual output'a gönder (Cable Input)
                if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                    // AudioPipeline kullanarak gönder
                    if (audioPipeline) {
                        audioPipeline->writeEffectsAudio(data);
                    } else {
                        virtualOutputDevice->write(data);
                    }
                }
                
                // SADECE test modunda fiziksel output'a gönder
                if (ui->testButton->isChecked()) {
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                }
            });

            usingEffects = false;
            qDebug() << "female voice effect started.";
        }
        else
        {
            qWarning() << "Audio devices are not properly initialized.";
        }
    }
    else
    {
        ui->femaleButton->setText("Kadın sesi");

        // Sadece female efektini durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            // Normal bağlantıyı geri kur (test veya normal mode)
            if (ui->testButton->isChecked()) {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Önce virtual output'a gönder (mix için)
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak gönder
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Sonra normal output'a gönder
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                });
            } else {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Test modu kapalı: Sadece virtual output'a efektli ses gönder
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak efekti sesi gönder (mix için)
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Normal output'a gönderme (test modu kapalı)
                });
            }
        }

        data.clear();
        usingEffects = true;
        qDebug() << "female effect stopped.";
    }
}


void MainWindow::on_combineButton_clicked(bool checked)
{
    if(checked)
    {
        // Diğer tüm efektleri durdur
        stopAllEffects();
        
        // Combine efektini başlat
        ui->combineButton->setChecked(true);
        ui->combineButton->setText("Stop");

        if(audioInput)
        {
            // Mevcut bağlantıyı kopar
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            // Yeni efekt bağlantısı kur
            connect(inputDevice, &QIODevice::readyRead, this, [=] {
                data = inputDevice->readAll();
                processToCombineVoice(data);
                progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    } // Progress bar'ı güncelle
                
                // Her zaman virtual output'a gönder (Cable Input)
                if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                    // AudioPipeline kullanarak gönder
                    if (audioPipeline) {
                        audioPipeline->writeEffectsAudio(data);
                    } else {
                        virtualOutputDevice->write(data);
                    }
                }
                
                // SADECE test modunda fiziksel output'a gönder
                if (ui->testButton->isChecked()) {
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                }
            });

            usingEffects = false;
            qDebug() << "combine voice effect started.";
        }
        else
        {
            qWarning() << "Audio devices are not properly initialized.";
        }
    }
    else
    {
        ui->combineButton->setText("Birleşik ses");

        // Sadece combine efektini durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            // Normal bağlantıyı geri kur (test veya normal mode)
            if (ui->testButton->isChecked()) {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Önce virtual output'a gönder (mix için)
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak gönder
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Sonra normal output'a gönder
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                });
            } else {
                connect(inputDevice, &QIODevice::readyRead, this, [=](){
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        emit audioDataReady(data);
                    }
                    // Test modu kapalı: Sadece virtual output'a efektli ses gönder
                    if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                        // AudioPipeline kullanarak efekti sesi gönder (mix için)
                        if (audioPipeline) {
                            audioPipeline->writeInputAudio(data);
                        } else {
                            virtualOutputDevice->write(data);
                        }
                    }
                    // Normal output'a gönderme (test modu kapalı)
                });
            }
        }

        data.clear();
        usingEffects = true;
        qDebug() << "combine effect stopped.";
    }
}

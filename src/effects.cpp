#include "effects.h"
#include "ui_mainwindow.h"
#include <QTime>
#include "mainwindow.h"
#include "psola.h"


effects::effects(QWidget *parent)
    : QMainWindow{parent}

{

}

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

    // Mevcut bağlantıları kopar - AMA kayıt sinyali bağlantısını koparma!
    if (inputDevice) {
        disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);

        // Yeni bağlantıyı kur - kayıt sinyalini HER ZAMAN emit et
        connect(inputDevice, &QIODevice::readyRead, this, [=](){
            data = inputDevice->readAll();
            progressBarOutput();

            // KAYIT İÇİN SİNYALİ HER DURUMDA EMIT ET
            if (isRecording) {
                qDebug() << "EMITTING SIGNAL (NO EFFECT): Audio size:" << data.size() << "bytes";
                emit audioDataReady(data);
            }

            if (ui->testButton->isChecked()) {
                // Test modu: virtual output ve normal output'a gönder
                if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                    if (audioPipeline) {
                        audioPipeline->writeInputAudio(data);
                    } else {
                        virtualOutputDevice->write(data);
                    }
                }
                
                if (outputDevice && outputDevice->isOpen()) {
                    outputDevice->write(data);
                }
            } else {
                // Normal mod: sadece virtual output'a gönder
                if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                    if (audioPipeline) {
                        audioPipeline->writeInputAudio(data);
                    } else {
                        virtualOutputDevice->write(data);
                    }
                }
            }
        });
    }

    data.clear();
    usingEffects = true; // Normal voice changer'a geri dön
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

        if(audioInput)
        {
            setupEffectConnection("ROBOT", [this](QByteArray &data) {
                processToRobotVoice(data);
            });
        }
        else {
            qWarning() << "Audio devices are not properly initialized.";
        }

        usingEffects = false;
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
                    if (ui->testButton->isChecked() && outputDevice && outputDevice->isOpen()) {
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
            setupEffectConnection("BANANA", [this](QByteArray &data) {
                processToBananaVoice(data);
            });
        }
        else {
            qWarning() << "Audio devices are not properly initialized.";
        }

        usingEffects = false;
        qDebug() << "Child voice effect started.";
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
                    if (ui->testButton->isChecked() && outputDevice && outputDevice->isOpen()) {
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
            setupEffectConnection("DEVIL", [this](QByteArray &data) {
                processToDevilVoice(data);
            });
        }
        else
        {
            qWarning() << "Audio devices are not properly initialized.";
        }

        usingEffects = false;
        qDebug() << "Devil voice effect started.";
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
                    if (ui->testButton->isChecked() && outputDevice && outputDevice->isOpen()) {
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
            setupEffectConnection("EKO", [this](QByteArray &data) {
                processToEkoVoice(data);
            });
        }
        else
        {
            qWarning() << "Audio devices are not properly initialized.";
        }

        usingEffects = false;
        qDebug() << "eko voice effect started.";
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
                    if (ui->testButton->isChecked() && outputDevice && outputDevice->isOpen()) {
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
            setupEffectConnection("FEMALE", [this](QByteArray &data) {
                processToFemaleVoice(data);
            });
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
                    if (ui->testButton->isChecked() && outputDevice && outputDevice->isOpen()) {
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
            setupEffectConnection("COMBINE", [this](QByteArray &data) {
                processToCombineVoice(data);
            });
        }
        else
        {
            qWarning() << "Audio devices are not properly initialized.";
        }

        usingEffects = false;
        qDebug() << "combine voice effect started.";
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
                    if (ui->testButton->isChecked() && outputDevice && outputDevice->isOpen()) {
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

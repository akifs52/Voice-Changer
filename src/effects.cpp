#include "effects.h"
#include "ui_mainwindow.h"
#include <QTime>
#include "mainwindow.h"
#include "psola.h"
#include "voiceeffects.h"


effects::effects(QWidget *parent)
    : QMainWindow{parent}

{

}

// Efekt yönetimi için yardımcı fonksiyon
void MainWindow::stopAllEffects()
{
    // Tüm efekt butonlarını durdur ve text'lerini geri getir
    ui->robotButton->setChecked(false);
    ui->robotButton->setText("Robot");

    ui->bananaButton->setChecked(false);
    ui->bananaButton->setText("Child");

    ui->devilButton->setChecked(false);
    ui->devilButton->setText("Monster");

    ui->femaleButton->setChecked(false);
    ui->femaleButton->setText("Female");

    ui->combineButton->setChecked(false);
    ui->combineButton->setText("Combine");

    ui->ekoButton->setChecked(false);
    ui->ekoButton->setText("Echo");

    ui->phaserButton->setChecked(false);
    ui->phaserButton->setText("Phaser");

    ui->flangerButton->setChecked(false);
    ui->flangerButton->setText("Flanger");

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
                
                // Test modunda output routing AudioPipeline tarafından yönetiliyor
                // Burada doğrudan outputDevice'e yazma - AudioPipeline test moduna göre yönlendirir
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
    if (data.isEmpty() || data.size() < static_cast<qsizetype>(sizeof(int16_t))) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToBananaVoice";
        return;
    }

    // Convert int16 to float for VoiceEffects processing
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);
    
    std::vector<float> floatInput(sampleCount);
    std::vector<float> floatOutput(sampleCount);
    
    // Convert int16 to float [-1.0, 1.0]
    for (int i = 0; i < sampleCount; ++i) {
        floatInput[i] = static_cast<float>(samples[i]) / 32768.0f;
    }
    
    // Apply BANANA effect using new VoiceEffects
    voiceEffects->processBanana(floatInput.data(), floatOutput.data(), sampleCount, format->sampleRate());
    
    // Convert float back to int16
    for (int i = 0; i < sampleCount; ++i) {
        float clamped = std::max(-1.0f, std::min(1.0f, floatOutput[i]));
        samples[i] = static_cast<int16_t>(clamped * 32767.0f);
    }
}

void MainWindow::processToRobotVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < static_cast<qsizetype>(sizeof(int16_t))) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToRobotVoice";
        return;
    }

    // Convert int16 to float for VoiceEffects processing
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);
    
    std::vector<float> floatInput(sampleCount);
    std::vector<float> floatOutput(sampleCount);
    
    // Convert int16 to float [-1.0, 1.0]
    for (int i = 0; i < sampleCount; ++i) {
        floatInput[i] = static_cast<float>(samples[i]) / 32768.0f;
    }
    
    // Apply ROBOT effect using new VoiceEffects with autotune
    voiceEffects->processRobot(floatInput.data(), floatOutput.data(), sampleCount, format->sampleRate());
    
    // Convert float back to int16
    for (int i = 0; i < sampleCount; ++i) {
        float clamped = std::max(-1.0f, std::min(1.0f, floatOutput[i]));
        samples[i] = static_cast<int16_t>(clamped * 32767.0f);
    }
}

void MainWindow::processToDevilVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < static_cast<qsizetype>(sizeof(int16_t))) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToDevilVoice";
        return;
    }

    // Convert int16 to float for VoiceEffects processing
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);
    
    std::vector<float> floatInput(sampleCount);
    std::vector<float> floatOutput(sampleCount);
    
    // Convert int16 to float [-1.0, 1.0]
    for (int i = 0; i < sampleCount; ++i) {
        floatInput[i] = static_cast<float>(samples[i]) / 32768.0f;
    }
    
    // Apply DEVIL effect using new VoiceEffects
    voiceEffects->processDevil(floatInput.data(), floatOutput.data(), sampleCount, format->sampleRate());
    
    // Convert float back to int16
    for (int i = 0; i < sampleCount; ++i) {
        float clamped = std::max(-1.0f, std::min(1.0f, floatOutput[i]));
        samples[i] = static_cast<int16_t>(clamped * 32767.0f);
    }
}

void MainWindow::processToFemaleVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < static_cast<qsizetype>(sizeof(int16_t))) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToFemaleVoice";
        return;
    }

    // Convert int16 to float for VoiceEffects processing
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);
    
    std::vector<float> floatInput(sampleCount);
    std::vector<float> floatOutput(sampleCount);
    
    // Convert int16 to float [-1.0, 1.0]
    for (int i = 0; i < sampleCount; ++i) {
        floatInput[i] = static_cast<float>(samples[i]) / 32768.0f;
    }
    
    // Apply FEMALE effect using new VoiceEffects
    voiceEffects->processFemale(floatInput.data(), floatOutput.data(), sampleCount, format->sampleRate());
    
    // Convert float back to int16
    for (int i = 0; i < sampleCount; ++i) {
        float clamped = std::max(-1.0f, std::min(1.0f, floatOutput[i]));
        samples[i] = static_cast<int16_t>(clamped * 32767.0f);
    }
}

void MainWindow::processToCombineVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < static_cast<qsizetype>(sizeof(int16_t))) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToCombineVoice";
        return;
    }

    // Convert int16 to float for VoiceEffects processing
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);
    
    std::vector<float> floatInput(sampleCount);
    std::vector<float> floatOutput(sampleCount);
    
    // Convert int16 to float [-1.0, 1.0]
    for (int i = 0; i < sampleCount; ++i) {
        floatInput[i] = static_cast<float>(samples[i]) / 32768.0f;
    }
    
    // Apply MILITARY effect using new VoiceEffects (Combine = Military)
    voiceEffects->processMilitary(floatInput.data(), floatOutput.data(), sampleCount, format->sampleRate());
    
    // Convert float back to int16
    for (int i = 0; i < sampleCount; ++i) {
        float clamped = std::max(-1.0f, std::min(1.0f, floatOutput[i]));
        samples[i] = static_cast<int16_t>(clamped * 32767.0f);
    }
}

void MainWindow::processToPhaserVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < static_cast<qsizetype>(sizeof(int16_t))) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToPhaserVoice";
        return;
    }

    // Convert int16 to float for VoiceEffects processing
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);
    
    std::vector<float> floatInput(sampleCount);
    std::vector<float> floatOutput(sampleCount);
    
    // Convert int16 to float [-1.0, 1.0]
    for (int i = 0; i < sampleCount; ++i) {
        floatInput[i] = static_cast<float>(samples[i]) / 32768.0f;
    }
    
    // Apply PHASER effect using new VoiceEffects
    voiceEffects->processPhaser(floatInput.data(), floatOutput.data(), sampleCount, format->sampleRate());
    
    // Convert float back to int16
    for (int i = 0; i < sampleCount; ++i) {
        float clamped = std::max(-1.0f, std::min(1.0f, floatOutput[i]));
        samples[i] = static_cast<int16_t>(clamped * 32767.0f);
    }
}

void MainWindow::processToFlangerVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < static_cast<qsizetype>(sizeof(int16_t))) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToFlangerVoice";
        return;
    }

    // Convert int16 to float for VoiceEffects processing
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);
    
    std::vector<float> floatInput(sampleCount);
    std::vector<float> floatOutput(sampleCount);
    
    // Convert int16 to float [-1.0, 1.0]
    for (int i = 0; i < sampleCount; ++i) {
        floatInput[i] = static_cast<float>(samples[i]) / 32768.0f;
    }
    
    // Apply FLANGER effect using new VoiceEffects
    voiceEffects->processFlanger(floatInput.data(), floatOutput.data(), sampleCount, format->sampleRate());
    
    // Convert float back to int16
    for (int i = 0; i < sampleCount; ++i) {
        float clamped = std::max(-1.0f, std::min(1.0f, floatOutput[i]));
        samples[i] = static_cast<int16_t>(clamped * 32767.0f);
    }
}

void MainWindow::processToEkoVoice(QByteArray &data)
{
    // Safety checks
    if (data.isEmpty() || data.size() < static_cast<qsizetype>(sizeof(int16_t))) {
        return;
    }

    if (!format) {
        qWarning() << "Audio format is null in processToEkoVoice";
        return;
    }

    // Convert int16 to float for VoiceEffects processing
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);
    
    std::vector<float> floatInput(sampleCount);
    std::vector<float> floatOutput(sampleCount);
    
    // Convert int16 to float [-1.0, 1.0]
    for (int i = 0; i < sampleCount; ++i) {
        floatInput[i] = static_cast<float>(samples[i]) / 32768.0f;
    }
    
    // Apply EKO effect using new VoiceEffects
    voiceEffects->processEko(floatInput.data(), floatOutput.data(), sampleCount, format->sampleRate());
    
    // Convert float back to int16
    for (int i = 0; i < sampleCount; ++i) {
        float clamped = std::max(-1.0f, std::min(1.0f, floatOutput[i]));
        samples[i] = static_cast<int16_t>(clamped * 32767.0f);
    }
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
                    // Test modunda output routing AudioPipeline tarafından yönetiliyor
                    // AudioPipeline test moduna göre normal output'a otomatik olarak yazar
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
                    // Test modunda output routing AudioPipeline tarafından yönetiliyor
                    // AudioPipeline test moduna göre normal output'a otomatik olarak yazar
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
                    // Test modunda output routing AudioPipeline tarafından yönetiliyor
                    // AudioPipeline test moduna göre normal output'a otomatik olarak yazar
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
                    // Test modunda output routing AudioPipeline tarafından yönetiliyor
                    // AudioPipeline test moduna göre normal output'a otomatik olarak yazar
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
                    // Test modunda output routing AudioPipeline tarafından yönetiliyor
                    // AudioPipeline test moduna göre normal output'a otomatik olarak yazar
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
                    // Test modunda output routing AudioPipeline tarafından yönetiliyor
                    // AudioPipeline test moduna göre normal output'a otomatik olarak yazar
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

void MainWindow::on_phaserButton_clicked(bool checked)
{
    if(checked)
    {
        // Diğer tüm efektleri durdur
        stopAllEffects();
        
        // Phaser efektini başlat
        ui->phaserButton->setChecked(true);
        ui->phaserButton->setText("Stop");

        if(audioInput)
        {
            setupEffectConnection("PHASER", [this](QByteArray &data) {
                processToPhaserVoice(data);
            });
        }
        else
        {
            qWarning() << "Audio devices are not properly initialized.";
        }

        usingEffects = false;
        qDebug() << "phaser effect started.";
    }
    else
    {
        ui->phaserButton->setText("Phaser");

        // Sadece phaser efektini durdur
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
                    // Test modunda output routing AudioPipeline tarafından yönetiliyor
                    // AudioPipeline test moduna göre normal output'a otomatik olarak yazar
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
        qDebug() << "phaser effect stopped.";
    }
}

void MainWindow::on_flangerButton_clicked(bool checked)
{
    if(checked)
    {
        // Diğer tüm efektleri durdur
        stopAllEffects();
        
        // Flanger efektini başlat
        ui->flangerButton->setChecked(true);
        ui->flangerButton->setText("Stop");

        if(audioInput)
        {
            setupEffectConnection("FLANGER", [this](QByteArray &data) {
                processToFlangerVoice(data);
            });
        }
        else
        {
            qWarning() << "Audio devices are not properly initialized.";
        }

        usingEffects = false;
        qDebug() << "flanger effect started.";
    }
    else
    {
        ui->flangerButton->setText("Flanger");

        // Sadece flanger efektini durdur
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
                    // Test modunda output routing AudioPipeline tarafından yönetiliyor
                    // AudioPipeline test moduna göre normal output'a otomatik olarak yazar
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
        qDebug() << "flanger effect stopped.";
    }
}

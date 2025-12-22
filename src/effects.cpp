#include "effects.h"
#include "ui_mainwindow.h"
#include <QTime>
#include "mainwindow.h"

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
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    // Yeni boyutu hesapla (örneğin, frekansı 1.5x artırmak için)
    int newSampleCount = sampleCount / 1.5; // Ses frekansını artırıyoruz
    QByteArray newData(newSampleCount * sizeof(int16_t), Qt::Uninitialized);
    int16_t *newSamples = reinterpret_cast<int16_t *>(newData.data());

    // Lineer interpolasyon ile frekansı artır
    for (int i = 0; i < newSampleCount; ++i) {
        double srcIndex = i * 1.5; // Oranına göre örnek seç
        int index1 = static_cast<int>(srcIndex);
        int index2 = qMin(index1 + 1, sampleCount - 1);

        // İki örnek arasında ağırlıklı ortalama al
        double weight = srcIndex - index1;
        newSamples[i] = static_cast<int16_t>((1 - weight) * samples[index1] + weight * samples[index2]);
    }

    // Yeni veriyi kullan
    data = newData;


}

void MainWindow::processToRobotVoice(QByteArray &data)
{
    // Ses verisini 16-bit PCM olarak yorumlayın
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());

    // Örnek sayısını hesaplayın
    int sampleCount = data.size() / sizeof(int16_t);

    // Echo gecikmesi için tampon boyutunu hesaplayın
    int delaySamples = format->sampleRate() / 200; // 5ms gecikme (daha kısa)

    // Echo için bir tampon oluşturun
    QVector<int16_t> echoBuffer(delaySamples, 0);

    for (int i = 0; i < sampleCount; ++i) {
        // Daha yumuşak kare dalga (daha düşük genlik)
        int16_t robotSample = samples[i] > 0 ? 4000 : -4000; // Genlik yarıya indirildi
        
        // Çok hafif modülasyon (pıt pıt önlemek için)
        double modulator = 0.02 * sin(2.0 * M_PI * 30 * i / format->sampleRate()); // 30Hz, çok düşük amplitude
        robotSample = static_cast<int16_t>(robotSample * (1.0 + modulator));

        // Çok hafif echo efekti
        if (i >= delaySamples) {
            int16_t echoSample = static_cast<int16_t>(0.02 * echoBuffer[i % delaySamples]); // Çok düşük echo
            robotSample = static_cast<int16_t>(robotSample + echoSample);
        }

        // Echo tamponunu güncelle
        echoBuffer[i % delaySamples] = robotSample;

        // Genliği daha yumuşak sınırlandır
        samples[i] = static_cast<int16_t>(qBound(-8000, robotSample, 8000));
    }
}

void MainWindow::processToDevilVoice(QByteArray &data)
{
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    // Yeni boyutu hesapla (örneğin, frekansı 1.5x artırmak için)
    int newSampleCount = sampleCount * 1.5;
    QByteArray newData(newSampleCount * sizeof(int16_t), Qt::Uninitialized);
    int16_t *newSamples = reinterpret_cast<int16_t *>(newData.data());

    // Cubic interpolasyon
    for (int i = 0; i < newSampleCount; ++i) {
        double srcIndex = i / 1.5;
        int index1 = static_cast<int>(srcIndex);
        int index2 = qMin(index1 + 1, sampleCount - 1);
        int index3 = qMin(index1 + 2, sampleCount - 1);
        int index4 = qMin(index1 + 3, sampleCount - 1);

        // Cubic interpolasyon hesaplama
        double t = srcIndex - index1;
        double a0 = samples[index4] - samples[index3] - samples[index1] + samples[index2];
        double a1 = samples[index1] - samples[index2] - a0;
        double a2 = samples[index3] - samples[index1];
        double a3 = samples[index2];

        // Yumuşak geçiş için cubic interpolasyon kullanılıyor
        newSamples[i] = static_cast<int16_t>(a0 * t * t * t + a1 * t * t + a2 * t + a3);
    }

    // Yeni veriyi kullan
    data = newData;
}

void MainWindow::processToFemaleVoice(QByteArray &data)
{
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    // Frekansı artırmak için yeni örnek sayısını belirleyin
    double pitchFactor = 1.15; // Daha az frekans artırımı
    int newSampleCount = sampleCount / pitchFactor;

    QByteArray newData(newSampleCount * sizeof(int16_t), Qt::Uninitialized);
    int16_t *newSamples = reinterpret_cast<int16_t *>(newData.data());

    // Frekans artırımı için interpolasyon işlemi
    for (int i = 0; i < newSampleCount; ++i) {
        double srcIndex = i * pitchFactor;
        int index1 = static_cast<int>(srcIndex);
        int index2 = qMin(index1 + 1, sampleCount - 1);

        double weight = srcIndex - index1; // İki örnek arasındaki ağırlık
        newSamples[i] = static_cast<int16_t>((1 - weight) * samples[index1] + weight * samples[index2]);
    }

    // Çok hafif modülasyon (pıt pıt önlemek için)
    double modFrequency = 80.0; // Daha düşük modülasyon frekansı
    double modAmplitude = 0.03;   // Çok düşük modülasyon genliği
    double sampleRate = format->sampleRate();

    for (int i = 0; i < newSampleCount; ++i) {
        double modulator = modAmplitude * sin(2.0 * M_PI * modFrequency * i / sampleRate);
        newSamples[i] = static_cast<int16_t>(newSamples[i] * (1.0 + modulator));
    }

    // Yeni işlenmiş veriyi kullan
    data = newData;
}

void MainWindow::processToCombineVoice(QByteArray &data)
{
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    // 1. Dar Bant Filtreleme (300 Hz - 3000 Hz band geçiren filtre)
    double lowCutoff = 300.0;  // Alt frekans sınırı
    double highCutoff = 3000.0; // Üst frekans sınırı
    double sampleRate = 44100.0; // Örnekleme frekansı (değiştirilebilir)
    QVector<double> filteredSamples(sampleCount, 0);

    for (int i = 0; i < sampleCount; ++i) {
        double t = i / sampleRate;
        filteredSamples[i] = samples[i] * (qSin(2 * M_PI * highCutoff * t) - qSin(2 * M_PI * lowCutoff * t));
    }

    // 2. Hafif Gürültü Ekleme (Beyaz gürültü)
    srand(QTime::currentTime().msec());
    for (int i = 0; i < sampleCount; ++i) {
        double noise = (rand() % 200 - 100) / 1000.0; // Hafif rastgele gürültü
        filteredSamples[i] += noise;
    }

    // 3. Hafif Distorsiyon
    for (int i = 0; i < sampleCount; ++i) {
        if (filteredSamples[i] > 30000) {
            filteredSamples[i] = 30000; // Pozitif klipleme
        } else if (filteredSamples[i] < -30000) {
            filteredSamples[i] = -30000; // Negatif klipleme
        }
    }

    // 4. Cubic Interpolasyon ile Frekans Değiştirme
    int newSampleCount = sampleCount * 1.2; // Frekansı artırmak için
    QByteArray newData(newSampleCount * sizeof(int16_t), Qt::Uninitialized);
    int16_t *newSamples = reinterpret_cast<int16_t *>(newData.data());

    for (int i = 0; i < newSampleCount; ++i) {
        double srcIndex = i / 1.2;
        int index1 = static_cast<int>(srcIndex);
        int index2 = qMin(index1 + 1, sampleCount - 1);
        int index3 = qMin(index1 + 2, sampleCount - 1);
        int index4 = qMin(index1 + 3, sampleCount - 1);

        double t = srcIndex - index1;
        double a0 = filteredSamples[index4] - filteredSamples[index3] - filteredSamples[index1] + filteredSamples[index2];
        double a1 = filteredSamples[index1] - filteredSamples[index2] - a0;
        double a2 = filteredSamples[index3] - filteredSamples[index1];
        double a3 = filteredSamples[index2];

        newSamples[i] = static_cast<int16_t>(a0 * t * t * t + a1 * t * t + a2 * t + a3);
    }

    // Yeni veriyi kullan
    data = newData;
}

void MainWindow::processToEkoVoice(QByteArray &data)
{
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    // Mağara ekosu parametreleri
    double decay1 = 0.4;  // İlk yankı gücü
    double decay2 = 0.25; // İkinci yankı gücü  
    double decay3 = 0.15; // Üçüncü yankı gücü
    int delay1 = format->sampleRate() / 5;   // 200ms - ilk yankı
    int delay2 = format->sampleRate() / 3;   // 333ms - ikinci yankı
    int delay3 = format->sampleRate() / 2;   // 500ms - üçüncü yankı

    // Yeni boyut: Orijinal + en uzun yankı alanı
    int newSampleCount = sampleCount + delay3;
    QByteArray newData(newSampleCount * sizeof(int16_t), Qt::Uninitialized);
    int16_t *newSamples = reinterpret_cast<int16_t *>(newData.data());
    
    // Başlangıçta sıfırla
    memset(newSamples, 0, newSampleCount * sizeof(int16_t));

    // Orijinal sesi kopyala
    for (int i = 0; i < sampleCount; ++i) {
        newSamples[i] = samples[i];
    }

    // Çoklu yankı ekle (mağara efekti)
    for (int i = 0; i < sampleCount; ++i) {
        // İlk yankı
        if (i + delay1 < newSampleCount) {
            newSamples[i + delay1] += static_cast<int16_t>(samples[i] * decay1);
        }
        
        // İkinci yankı
        if (i + delay2 < newSampleCount) {
            newSamples[i + delay2] += static_cast<int16_t>(samples[i] * decay2);
        }
        
        // Üçüncü yankı
        if (i + delay3 < newSampleCount) {
            newSamples[i + delay3] += static_cast<int16_t>(samples[i] * decay3);
        }
    }

    // Sınırlama (clipping önlemek için)
    for (int i = 0; i < newSampleCount; ++i) {
        newSamples[i] = static_cast<int16_t>(qBound(-15000, newSamples[i], 15000));
    }

    // Yeni veriyi geri ata
    data = newData;
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

#include "effects.h"
#include "ui_mainwindow.h"
#include <QTime>

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
    int delaySamples = format->sampleRate() / 100; // 10ms gecikme

    // Echo için bir tampon oluşturun
    QVector<int16_t> echoBuffer(delaySamples, 0);

    for (int i = 0; i < sampleCount; ++i) {
        // Kare dalga üretimi (daha düşük genlik)
        samples[i] = samples[i] > 0 ? 8000 : -8000;

        // Modülasyon ekleme (daha hafif modülasyon)
        double modulator = 0.1 * sin(2.0 * M_PI * 50 * i / format->sampleRate());
        samples[i] = static_cast<int16_t>(samples[i] * (1.0 + modulator));

        // Echo efekti (çok hafif echo efekti)
        if (i >= delaySamples) {
            int16_t echoSample = static_cast<int16_t>(0.05 * echoBuffer[i % delaySamples]); // Echo genliği daha da düşürüldü
            samples[i] = static_cast<int16_t>(samples[i] + echoSample);
        }

        // Echo tamponunu güncelle
        echoBuffer[i % delaySamples] = samples[i];

        // Genliği çok sıkı bir şekilde sınırlandır
        samples[i] = static_cast<int16_t>(qBound(-12000, samples[i], 12000));
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
    double pitchFactor = 1.2; // Kadın sesi için frekans artırımı
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

    // Modülasyon ekleme (Kadın sesine hafif bir efekt için)
    double modFrequency = 150.0; // Kadın sesi için daha düşük bir modülasyon frekansı
    double modAmplitude = 0.1;   // Daha ince bir modülasyon genliği
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

    double decay = 0.5;  // Yankının azalması
    int delaySamples = 500;  // Gecikme (örnek sayısı)

    // Yeni boyut: Cubik interpolasyonla örnek sayısını artırıyoruz
    int newSampleCount = sampleCount + delaySamples * 2;  // Ek yankı alanı için genişletilmiş boyut
    QByteArray newData(newSampleCount * sizeof(int16_t), Qt::Uninitialized);
    int16_t *newSamples = reinterpret_cast<int16_t *>(newData.data());

    // İlk olarak orijinal örnekleri cubic interpolasyonla genişletiyoruz
    for (int i = 0; i < newSampleCount; ++i) {
        if (i < sampleCount) {
            double srcIndex = i;
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

            // Interpolasyon sonucu yeni örneğe atanır
            newSamples[i] = static_cast<int16_t>(a0 * t * t * t + a1 * t * t + a2 * t + a3);
        } else {
            newSamples[i] = 0;  // Boş kalan alanlar sıfırlanır
        }
    }

    // Orijinal sesin üzerine yankıyı ekliyoruz
    for (int i = 0; i < newSampleCount; ++i) {
        if (i >= delaySamples) {
            newSamples[i] += static_cast<int16_t>(newSamples[i - delaySamples] * decay);
        }
        if (i >= 2 * delaySamples) {
            newSamples[i] += static_cast<int16_t>(newSamples[i - 2 * delaySamples] * (decay / 2));
        }

        // Sınırlandırma
        if (newSamples[i] > INT16_MAX) newSamples[i] = INT16_MAX;
        if (newSamples[i] < INT16_MIN) newSamples[i] = INT16_MIN;
    }

    // Yeni veriyi geri ata
    data = newData;
}


void MainWindow::on_robotButton_clicked(bool checked)
{


    if(checked)
    {
        ui->robotButton->setText("Stop");

        if(testNotOpened)
        {
            audioInput->resume();
        }

        audioOutputVirtual->resume();

        if(audioInput && audioOutputVirtual)
        {
            connect(inputDevice, &QIODevice::readyRead, this, [=](){

                data = inputDevice->readAll();

                processToRobotVoice(data);

                if(outputDeviceVirtual)
                {
                    outputDeviceVirtual->write(data);
                }

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

        // İşlemi durdur
        if (inputDevice)
        {
        disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
        }
        if(testNotOpened)
        {
            audioInput->suspend();
        }
        if(audioOutputVirtual)
        {
          audioOutputVirtual->suspend();
        }

        usingEffects = true;
        qDebug() << "robot effect stopped.";
    }

}


void MainWindow::on_bananaButton_clicked(bool checked)
{


    if (checked) {
        ui->bananaButton->setText("Stop");

        if(testNotOpened)
        {
            audioInput->resume();
        }

        audioOutputVirtual->resume();

        if (audioInput && audioOutput) {
            connect(inputDevice, &QIODevice::readyRead, this, [=]() {
                data = inputDevice->readAll();

                // Çocuk sesi efektini uygula
                processToBananaVoice(data);


                // Sanal cihaza yaz
                if (outputDeviceVirtual) {
                    outputDeviceVirtual->write(data);
                }

            });

            usingEffects = false;

            qDebug() << "Child voice effect started.";

        }
        else
        {
            qWarning() << "Audio devices are not properly initialized.";
        }
    } else {
        ui->bananaButton->setText("Çocuk Sesi");

        // İşlemi durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
        }
        if(testNotOpened)
        {
            audioInput->suspend();
        }

        if(audioOutputVirtual)
        {
          audioOutputVirtual->suspend();
        }

        usingEffects = true;
        qDebug() << "Child voice effect stopped.";
    }

}

void MainWindow::on_devilButton_clicked(bool checked)
{


    if(checked)
    {
        ui->devilButton->setText("Stop");

        if(testNotOpened)
        {
            audioInput->resume();
        }

        audioOutputVirtual->resume();

        if(audioInput && audioOutputVirtual)
        {
            connect(inputDevice, &QIODevice::readyRead, this, [=] {

                data = inputDevice->readAll();

                processToDevilVoice(data);

                // Sanal cihaza yaz
                if (outputDeviceVirtual) {
                    outputDeviceVirtual->write(data);
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

        // İşlemi durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
        }
        if(testNotOpened)
        {
            audioInput->suspend();
        }

        if(audioOutputVirtual)
        {
            audioOutputVirtual->suspend();
        }

        usingEffects = true;
        qDebug() << "Devil effect stopped.";
    }
}

void MainWindow::on_ekoButton_clicked(bool checked)
{


    if(checked)
    {
        ui->ekoButton->setText("Stop");

        if(testNotOpened)
        {
            audioInput->resume();
        }

        audioOutputVirtual->resume();

        if(audioInput && audioOutputVirtual)
        {
            connect(inputDevice, &QIODevice::readyRead, this, [=] {

                data = inputDevice->readAll();

                processToEkoVoice(data);

                // Sanal cihaza yaz
                if (outputDeviceVirtual) {
                    outputDeviceVirtual->write(data);
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
        ui->ekoButton->setText("Eko sesi");

        // İşlemi durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
        }
        if(testNotOpened)
        {
            audioInput->suspend();
        }

        if(audioOutputVirtual)
        {
            audioOutputVirtual->suspend();
        }

        usingEffects = true;
        qDebug() << "eko effect stopped.";
    }
}


void MainWindow::on_boldButton_clicked(bool checked)
{


}

void MainWindow::on_femaleButton_clicked(bool checked)
{

    if(checked)
    {
        ui->femaleButton->setText("Stop");

        if(testNotOpened)
        {
            audioInput->resume();
        }

        audioOutputVirtual->resume();

        if(audioInput && audioOutputVirtual)
        {
            connect(inputDevice, &QIODevice::readyRead, this, [=] {

                data = inputDevice->readAll();

                processToFemaleVoice(data);

                // Sanal cihaza yaz
                if (outputDeviceVirtual) {
                    outputDeviceVirtual->write(data);
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

        // İşlemi durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
        }
        if(testNotOpened)
        {
            audioInput->suspend();
        }

        if(audioOutputVirtual)
        {
            audioOutputVirtual->suspend();
        }

        usingEffects = true;
        qDebug() << "female effect stopped.";
    }
}


void MainWindow::on_combineButton_clicked(bool checked)
{
    if(checked)
    {
        ui->combineButton->setText("Stop");

        if(testNotOpened)
        {
            audioInput->resume();
        }

        audioOutputVirtual->resume();

        if(audioInput && audioOutputVirtual)
        {
            connect(inputDevice, &QIODevice::readyRead, this, [=] {

                data = inputDevice->readAll();

                processToCombineVoice(data);

                // Sanal cihaza yaz
                if (outputDeviceVirtual) {
                    outputDeviceVirtual->write(data);
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
        ui->combineButton->setText("Combine hl");

        // İşlemi durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
        }
        if(testNotOpened)
        {
            audioInput->suspend();
        }

        if(audioOutputVirtual)
        {
            audioOutputVirtual->suspend();
        }

        usingEffects = true;
        qDebug() << "combine effect stopped.";
    }
}

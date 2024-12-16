#include "effects.h"
#include "ui_mainwindow.h"

effects::effects(QWidget *parent)
    : QMainWindow{parent}

{

}

void MainWindow::processToBananaVoice(QByteArray &data)
{
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());
    int sampleCount = data.size() / sizeof(int16_t);

    for (int i = 0; i < sampleCount; ++i) {
        // Ses frekansını yükselt
        samples[i] = static_cast<int16_t>(qBound(-30000, static_cast<int>(samples[i] * 3), 30000));
    }
}

void MainWindow::processToRobotVoice(QByteArray &data)
{
    // Ses verisini 16-bit PCM olarak yorumlayın
    int16_t *samples = reinterpret_cast<int16_t *>(data.data());

    // Örnek sayısını hesaplayın
    int sampleCount = data.size() / sizeof(int16_t);

    // İşleme döngüsü
    for (int i = 0; i < sampleCount; ++i) {
        // Kare dalga üretimi
        samples[i] = samples[i] > 0 ? 30000 : -30000;

        // Modülasyon ekleme
        double modulator = 0.5 * sin(2.0 * M_PI * 50 * i / format->sampleRate());
        samples[i] = static_cast<int16_t>(samples[i] * (1.0 + modulator));

        // Echo efekti
        int delaySamples = format->sampleRate() / 50; // 20ms gecikme
        if (i >= delaySamples) {
            samples[i] += static_cast<int16_t>(0.3 * samples[i - delaySamples]);
        }
    }
}


void MainWindow::on_robotButton_clicked(bool checked)
{
    if(checked)
    {
        ui->robotButton->setText("Stop");

        if(audioInput && audioOutput)
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



    else {
    ui->robotButton->setText("Robot sesi");

    // İşlemi durdur
    if (inputDevice) {
        disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
    }
        usingEffects = true;
        qDebug() << "robot effect stopped.";
    }

}

void MainWindow::on_bananaButton_clicked(bool checked)
{
    if (checked) {
        ui->bananaButton->setText("Stop");

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


        } else {
            qWarning() << "Audio devices are not properly initialized.";
        }
    } else {
        ui->bananaButton->setText("Çocuk Sesi");

        // İşlemi durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
        }
        usingEffects = true;
        qDebug() << "Child voice effect stopped.";
    }

}

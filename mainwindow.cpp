#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , format(new QAudioFormat)
    , audioOutput(nullptr)
    , audioOutputVirtual(nullptr)
    , audioInput(nullptr)
    , outputDeviceVirtual(nullptr)
    , inputDevice(nullptr)
    , outputDevice(nullptr)

{
    ui->setupUi(this);

    ui->frame_3->hide();
    searchInputDevice();
    searchOutputDevice();
    searchVirtualOutputDevice();

}

MainWindow::~MainWindow()
{

    if (audioInput) {
        audioInput->stop();
        delete audioInput;
    }

    if(audioOutput){
        audioOutput->stop();
        delete audioOutput;
    }

    disconnect(inputDevice, &QIODevice::readyRead,this,&MainWindow::progressBarOutput);


    delete format;


    delete ui;


}


void MainWindow::searchInputDevice()
{
    const auto devicesI = QMediaDevices::audioInputs();
    for(const QAudioDevice &device : devicesI)
    {
        ui->inputcombobox->addItem(device.description(), QVariant::fromValue(device) ); //bakılacak
    }
}

void MainWindow::searchOutputDevice()
{
    const auto devicesO = QMediaDevices::audioOutputs();
    for(const QAudioDevice &device : devicesO)
    {
        ui->outputcombobox->addItem(device.description(), QVariant::fromValue(device));
    }

}

void MainWindow::searchVirtualOutputDevice()
{
    const auto devicesO = QMediaDevices::audioInputs();
    for(const QAudioDevice &device : devicesO)
    {
        ui->VirtualOutputBox->addItem(device.description(), QVariant::fromValue(device));
    }
}


void MainWindow::on_inputslider_valueChanged(int value)
{
    ui->inputlabel->setText(QString::number(value).arg("%0"));

    if (audioInput) {
        ui->inputslider->setValue(static_cast<int>(audioInput->volume() * 100));
    }


    float volume = value/100.0f;
    audioInput->setVolume(volume);

}

void MainWindow::on_outputslider_valueChanged(int value)
{


    ui->outputlabel->setText(QString::number(value).arg("%0"));

    if (audioOutput) {
        ui->outputslider->setValue(static_cast<int>(audioOutput->volume() * 100));

    }

    float volume = value/100.0f;
    audioOutput->setVolume(volume);
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->labelVirtual->setText(QString::number(value).arg("%0"));

    if(audioOutputVirtual)
    {
        ui->horizontalSlider->setValue(static_cast<int>(audioOutputVirtual->volume()*100));

    }
    else
    {
        return;
    }

    float volume = value/100.0f;
    audioOutputVirtual->setVolume(volume);
}



void MainWindow::on_refreshInput_clicked()
{

    qDebug()<< ui->inputcombobox->count();

    for(int i= ui->inputcombobox->count(); i>-1 ; i--)
    {
        ui->inputcombobox->removeItem(i);
    }

    searchInputDevice();
}


void MainWindow::on_refreshOutput_clicked()
{
    qDebug()<< ui->outputcombobox->count();

    for(int i= ui->outputcombobox->count(); i>-1 ; i--)
    {
        ui->outputcombobox->removeItem(i);
    }

    searchOutputDevice();

}

void MainWindow::on_refresVirtualOutput_clicked()
{
    qDebug()<<ui->VirtualOutputBox->count();
    for(int i= ui->VirtualOutputBox->count(); i>-1; i--)
    {
        ui->VirtualOutputBox->removeItem(i);
    }

    searchVirtualOutputDevice();
}


void MainWindow::on_inputcombobox_currentIndexChanged(int index)
{
    // Varsayılan format ayarları
    format->setSampleRate(48000);
    format->setChannelCount(2);
    format->setSampleFormat(QAudioFormat::Int16);

    QVariant inputData = ui->inputcombobox->itemData(index);
    if (!inputData.isValid()) {
        qWarning() << "No valid input device selected.";
        return;
    }

    QAudioDevice infoInput = inputData.value<QAudioDevice>();

    // Cihazın format desteği kontrolü
    if (!infoInput.isFormatSupported(*format)) {
        qWarning() << "Selected device does not support the default format. Updating format.";
        *format = infoInput.preferredFormat();
        qDebug() << "Updated Format: SampleRate:" << format->sampleRate()
                 << ", Channels:" << format->channelCount()
                 << ", SampleFormat:" << format->sampleFormat();
    }

    // Önceki `audioInput` nesnesini temizleme
    if (audioInput) {
        audioInput->stop();
        delete audioInput;
    }

    // Yeni `audioInput` nesnesini oluşturma
    audioInput = new QAudioSource(infoInput,*format, this);

    // Sinyal bağlantısı
    connect(audioInput, &QAudioSource::stateChanged, this, [](QAudio::State state) {
        if (state == QAudio::IdleState) {
            qDebug() << "Audio input is idle.";
        } else if (state == QAudio::StoppedState) {
            qDebug() << "Audio input has stopped.";
        }
    });

    // Ses kaynağını başlatma
    inputDevice = audioInput->start();
}



void MainWindow::on_outputcombobox_currentIndexChanged(int index)
{
    // Varsayılan format ayarları
    format->setSampleRate(48000);
    format->setChannelCount(2);
    format->setSampleFormat(QAudioFormat::Int16);

    QVariant outputData = ui->outputcombobox->itemData(index);
    if (!outputData.isValid())
    {
        qWarning() << "No valid output device selected.";
        return;
    }

    QAudioDevice infoOutput = outputData.value<QAudioDevice>();

    // Cihazın format desteği kontrolü
    if (!infoOutput.isFormatSupported(*format))
    {
        qWarning() << "Selected device does not support the default format. Trying preferred format.";
        *format = infoOutput.preferredFormat();

        // `preferredFormat`'in geçerli olup olmadığını kontrol edin
        if (format->sampleRate() <= 0 || format->channelCount() <= 0 || format->sampleFormat() == QAudioFormat::Unknown)
        {
            qCritical() << "Preferred format is invalid. Falling back to default format.";
            format->setSampleRate(44100);  // Alternatif varsayılan değer
            format->setChannelCount(2);
            format->setSampleFormat(QAudioFormat::Int16);
        }

        qDebug() << "Updated Format: SampleRate:" << format->sampleRate()
                 << ", Channels:" << format->channelCount()
                 << ", SampleFormat:" << format->sampleFormat();
    }

    // Eski `audioOutput` nesnesini temizleme
    if (audioOutput)
    {
        audioOutput->stop();
        delete audioOutput;
    }

    // Yeni `audioOutput` nesnesini oluşturma
    audioOutput = new QAudioSink(infoOutput,*format,this);

    // Sinyal bağlantısı
    connect(audioOutput, &QAudioSink::stateChanged, this, [](QAudio::State state) {
        if (state == QAudio::IdleState) {
            qDebug() << "Audio output is idle.";
        } else if (state == QAudio::StoppedState) {
            qDebug() << "Audio output has stopped.";
        }
    });

    // Ses kaynağını başlatma
    outputDevice = audioOutput->start();

}


void MainWindow::on_VirtualOutputBox_currentIndexChanged(int index)
{
    // Varsayılan format ayarları
    format->setSampleRate(48000);
    format->setChannelCount(2);
    format->setSampleFormat(QAudioFormat::Int16);

    QVariant virtualOutputData = ui->VirtualOutputBox->currentData(index);

    if (!virtualOutputData.isValid())
    {
        qWarning() << "No valid virtual output device selected.";
        return;
    }

    QAudioDevice virtualOutputinfo = virtualOutputData.value<QAudioDevice>();

    // Cihazın format desteği kontrolü
    if (!virtualOutputinfo.isFormatSupported(*format)) {
        qWarning() << "Selected device does not support the default format. Updating format.";
        *format = virtualOutputinfo.preferredFormat();
        qDebug() << "Updated Format: SampleRate:" << format->sampleRate()
                 << ", Channels:" << format->channelCount()
                 << ", SampleFormat:" << format->sampleFormat();
    }

    // Eski `audioOutput` nesnesini temizleme
    if (audioOutputVirtual)
    {
        audioOutputVirtual->stop();
        delete audioOutputVirtual;
        audioOutputVirtual = nullptr;
    }

    // Yeni `audioOutput` nesnesini oluşturma
    audioOutputVirtual = new QAudioSource(virtualOutputinfo,*format,this);

    // Sinyal bağlantısı
    connect(audioOutputVirtual, &QAudioSource::stateChanged, this, [](QAudio::State state) {
        if (state == QAudio::IdleState) {
            qDebug() << "Audio virtual output is idle.";
        } else if (state == QAudio::StoppedState) {
            qDebug() << "Audio virtual output has stopped.";
        }
    });

    // Ses kaynağını başlatma
    outputDeviceVirtual = audioOutputVirtual->start();
    if (!outputDeviceVirtual) {
        qWarning() << "Failed to start audio output.";
    }

}

void MainWindow::processAudioInput()
{
    if (!inputDevice || !outputDevice) {
        qCritical() << "Input or output device is null.";
        return;
    }

    QByteArray data = inputDevice->readAll();
    if (data.isEmpty()) {
        qWarning() << "No data read from input device.";
        return;
    }

    qint64 written = outputDevice->write(data);
    if (written == -1) {
        qCritical() << "Failed to write data to output device.";
    }
}




void MainWindow::on_testButton_clicked(bool checked)
{
    if (checked) {
        ui->testButton->setText("Stop");
        testNotOpened= false; //test açılmadıysa inputu efektlerin içinde açar

            audioInput->resume();
            audioOutput->resume();

            qDebug() << "Listening started.";

            if(audioInput && audioOutput)
            {
                connect(inputDevice, &QIODevice::readyRead, this, [=]() {

                    if(usingEffects)
                    {
                        data = inputDevice->readAll();
                        qDebug() << "ab";
                    }

                    progressBarOutput(); // Progress bar işlemi

                    // Output ver
                    if (outputDevice) {
                        outputDevice->write(data);
                    }


                });
            }
             else
            {
            qWarning() << "Audio devices are not properly initialized.";
            }
    }
        else {
        ui->testButton->setText("Test Device");

        // Dinlemeyi durdur
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead,this,nullptr);
        }
        if (audioInput) {
            audioInput->suspend();
        }
        if (audioOutput) {
            audioOutput->suspend();
        }

        qDebug() << "Listening stopped.";
    }
}


void MainWindow::progressBarOutput()
{


    if(!inputDevice || !outputDevice)
    {
        qCritical() << "Input or output device is null.";
        return;
    }

    //data = inputDevice->readAll();

    if(data.isEmpty())
    {
        qWarning() << "No data read from input device.";
        return;
    }

    // Ses genliği hesaplama
    qint16 *samples = reinterpret_cast<qint16 *>(data.data());
    int numSamples = data.size() / sizeof(qint16);
    qint16 maxAmplitude = 0;

    for (int i = 0; i < numSamples; ++i) {
        maxAmplitude = qMax(maxAmplitude, qAbs(samples[i]));
    }

    // Normalize ve progress bar güncelle
    float normalizedAmplitude = static_cast<float>(maxAmplitude) / 32767.0f;
    int progressValue = static_cast<int>(normalizedAmplitude * 100); // 0-100 arası değer

    ui->progressBar->setValue(progressValue); // Progress bar güncelle
    qDebug() << "Volume Level:" << progressValue;

}

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , format(new QAudioFormat)
    , audioOutput(nullptr)
    , audioInput(nullptr)
    , inputDevice(nullptr)
    , outputDevice(nullptr)
    , isListening(false)
{
    ui->setupUi(this);

    ui->frame_3->hide();
    ui->stopButton->hide();
    searchInputDevice();
    searchOutputDevice();

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



void MainWindow::on_inputslider_valueChanged(int value)
{
    ui->inputlabel->setText(QString::number(value).arg("%0"));
}

void MainWindow::on_outputslider_valueChanged(int value)
{

    ui->outputlabel->setText(QString::number(value).arg("%0"));
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




void MainWindow::on_inputcombobox_currentIndexChanged(int index)
{
    // Varsayılan format ayarları
    format->setSampleRate(48000);
    format->setChannelCount(2);
    format->setSampleFormat(QAudioFormat::UInt8);

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
    format->setSampleFormat(QAudioFormat::UInt8);

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




void MainWindow::on_testButton_clicked()
{

    if (!audioInput || !audioOutput) {
        qCritical() << "Audio devices are not properly initialized.";
        //ui->testButton->setChecked(false);
        return;
    }

    inputDevice = audioInput->start();
    if (!inputDevice) {
        qCritical() << "Failed to start audio input device.";
        //ui->testButton->setChecked(false);
        return;
    }

    outputDevice = audioOutput->start();
    if (!outputDevice) {
        qCritical() << "Failed to start audio output device.";
        //ui->testButton->setChecked(false);
        return;
    }

    //connect(inputDevice, &QIODevice::readyRead, this, &MainWindow::processAudioInput);
    connect(inputDevice, &QIODevice::readyRead,this,&MainWindow::progressBarOutput);

    isListening = true;
    qDebug() << "Listening started.";
}


void MainWindow::on_stopButton_clicked()
{

        if (inputDevice) {
           // disconnect(inputDevice, &QIODevice::readyRead, this, &MainWindow::processAudioInput);
            disconnect(inputDevice, &QIODevice::readyRead,this,&MainWindow::progressBarOutput);
        }

        if (audioInput) {
            audioInput->stop();
            inputDevice = nullptr;
        }

        if (audioOutput) {
            audioOutput->stop();
            outputDevice = nullptr;
        }

        isListening = false;
        qDebug() << "Listening stopped.";
}


void MainWindow::progressBarOutput()
{


    if(!inputDevice || !outputDevice)
    {
        qCritical() << "Input or output device is null.";
        return;
    }

    QByteArray data = inputDevice->readAll();

    if(data.isEmpty())
    {
        qWarning() << "No data read from input device.";
        return;
    }

    // UInt8 formatında genlik hesaplama
    const unsigned char *samples = reinterpret_cast<const unsigned char *>(data.data());
    int numSamples = data.size();
    unsigned char maxAmplitude = 0;

    for(int i = 0; i<numSamples; i++)
    {
        maxAmplitude = qMax(maxAmplitude,static_cast<unsigned char>(qAbs(samples[i]-128)));
         // `samples[i] - 128`: Sessizlik seviyesi için ofset çıkarılır.

        float normalizedAmplitude = static_cast<float>(maxAmplitude)/127.0f; // Sessizlik seviyesi çevresinde normalize
        int progressValue = static_cast<int>(normalizedAmplitude*100); // 0-100 arası değer

        ui->progressBar->setValue(progressValue); //Progress bar güncelle
        qDebug() << "Volume Level:" << progressValue;

        qint64 written = outputDevice->write(data);
        if(written == -1)
        {
            qCritical() << "Failed to write data to output device.";
        }
    }

}

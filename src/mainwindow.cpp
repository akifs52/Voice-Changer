#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , format(new QAudioFormat)
    , audioOutput(nullptr)
    , audioInput(nullptr)
    , inputDevice(nullptr)
    , outputDevice(nullptr)

{
    ui->setupUi(this);
    ui->frame_3->hide();
    searchInputDevice();
    searchOutputDevice();

    // Setup hotkeys
    allinputKeys();
    connectAllHotkeys();

    // Populate hotkey dropdowns
    for (int i = 1; i <= 20; ++i) {
        QString name = QString("hotkey%1").arg(i);
        QComboBox *combo = findChild<QComboBox*>(name);
        if (combo) {
            populateComboBox(combo);
        }
    }
    
    // Install event filter for hotkey handling
    this->installEventFilter(this);
    
    // Load saved hotkeys
    loadHotkeys();
    
    // Preload default sound files to prevent first-click delay
    preloadTotal = 10;
    preloadCount = 0;
    preloadAudio(filename1);
    preloadAudio(filename2);
    preloadAudio(filename3);
    preloadAudio(filename4);
    preloadAudio(filename5);
    preloadAudio(filename6);
    preloadAudio(filename7);
    preloadAudio(filename8);
    preloadAudio(filename9);
    preloadAudio(filename10);
    qDebug() << "Default sound files preloading started";


}

MainWindow::~MainWindow()
{
    // Save hotkeys before cleanup
    saveHotkeys();
    
    // Clean up global hotkeys
#ifdef Q_OS_WIN
    for (auto it = m_globalHotkeyIds.begin(); it != m_globalHotkeyIds.end(); ++it) {
        UnregisterHotKey((HWND)this->winId(), it.value());
    }
    m_globalHotkeyIds.clear();
#endif

    if (audioInput) {
        audioInput->stop();
        delete audioInput;
    }

    if(audioOutput){
        audioOutput->stop();
        delete audioOutput;
    }

    delete format;

    on_stopRecord_clicked();
    
    // Clean up audio cache
    for (auto it = audioCache.begin(); it != audioCache.end(); ++it) {
        delete it.value();
    }
    audioCache.clear();

    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Event filter is now handled by QShortcut and global hotkey system
    // This can remain simple for any future custom event handling
    return QMainWindow::eventFilter(obj, event);
}

#ifdef Q_OS_WIN
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    if (eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG*>(message);
        if (msg->message == WM_HOTKEY) {
            int hotkeyId = msg->wParam;
            int soundIndex = hotkeyId - GLOBAL_HOTKEY_BASE_ID;
            
            if (soundIndex >= 1 && soundIndex <= 20) {
                // Trigger the corresponding sound button click
                switch(soundIndex) {
                    case 1: on_sound1_clicked(); break;
                    case 2: on_sound2_clicked(); break;
                    case 3: on_sound3_clicked(); break;
                    case 4: on_sound4_clicked(); break;
                    case 5: on_sound5_clicked(); break;
                    case 6: on_sound6_clicked(); break;
                    case 7: on_sound7_clicked(); break;
                    case 8: on_sound8_clicked(); break;
                    case 9: on_sound9_clicked(); break;
                    case 10: on_sound10_clicked(); break;
                    case 11: on_sound11_clicked(); break;
                    case 12: on_sound12_clicked(); break;
                    case 13: on_sound13_clicked(); break;
                    case 14: on_sound14_clicked(); break;
                    case 15: on_sound15_clicked(); break;
                    case 16: on_sound16_clicked(); break;
                    case 17: on_sound17_clicked(); break;
                    case 18: on_sound18_clicked(); break;
                    case 19: on_sound19_clicked(); break;
                    case 20: on_sound20_clicked(); break;
                }
                *result = 0;
                return true;
            }
        }
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}
#endif

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
    
    // Başlangıçta normal mod bağlantısı kur (progress bar için)
    if (inputDevice) {
        connect(inputDevice, &QIODevice::readyRead, this, [=](){
            data = inputDevice->readAll();
            progressBarOutput();
            // Emit signal for recording when recording is active
            if (isRecording) {
                qDebug() << "EMITTING SIGNAL (INITIAL): Audio size:" << data.size() << "bytes";
                emit audioDataReady(data);
            }
            // Normal modda output'a gönderme
        });
        qDebug() << "Initial audio connection established.";
    }
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

void MainWindow::on_testButton_clicked(bool checked)
{
    if (checked) {
        ui->testButton->setText("Stop");

        if(!audioInput)
        {
            qWarning() << "Audio input is not initialized";
            return;
        }

        audioInput->resume();
        
        if(audioOutput) {
            audioOutput->resume();
        }

        qDebug() << "Test mode started.";

        if(audioInput && audioOutput)
        {
            // Önce mevcut bağlantıları temizle
            if (inputDevice) {
                disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            }
            
            // Test modu bağlantısı kur - efekt durumunu kontrol et
            if (inputDevice) {
                connect(inputDevice, &QIODevice::readyRead, this, [=]() {
                    data = inputDevice->readAll();
                    
                    // Eğer bir efekt aktifse onu uygula
                    if (!usingEffects) {
                        // Hangi efekt aktif olduğunu kontrol et
                        if (ui->robotButton->isChecked()) {
                            processToRobotVoice(data);
                        } else if (ui->bananaButton->isChecked()) {
                            processToBananaVoice(data);
                        } else if (ui->devilButton->isChecked()) {
                            processToDevilVoice(data);
                        } else if (ui->femaleButton->isChecked()) {
                            processToFemaleVoice(data);
                        } else if (ui->combineButton->isChecked()) {
                            processToCombineVoice(data);
                        } else if (ui->ekoButton->isChecked()) {
                            processToEkoVoice(data);
                        }
                        // Efekt yoksa data değişmeden kalır
                    }
                    
                    // Progress bar'ı güncelle
                    progressBarOutput();
                    
                    // Emit signal for recording when recording is active (after effects)
                    if (isRecording) {
                        qDebug() << "EMITTING SIGNAL: Processed audio size:" << data.size() << "bytes";
                        emit audioDataReady(data);
                    }
                    
                    // SADECE test modunda output'a gönder
                    if (outputDevice && outputDevice->isOpen()) {
                        outputDevice->write(data);
                    }
                });
            }
        }
        else
        {
            qWarning() << "Audio devices are not properly initialized.";
        }
    } 
    else {
        ui->testButton->setText("Test Device");

        // Test bağlantısını kopar
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            if (usingEffects == false) {
                // Bir efekt aktifse, onun bağlantısını kur
                // Bu stopAllEffects içinde handled olur
                stopAllEffects();
            } else {
                // Normal mod için bağlantı kur - HİÇBİR ZAMAN output'a gönderme
                connect(inputDevice, &QIODevice::readyRead, this, [=]() {
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        qDebug() << "EMITTING SIGNAL (NORMAL): Audio size:" << data.size() << "bytes";
                        emit audioDataReady(data);
                    }
                });
            }
        }

        if (audioOutput) {
            audioOutput->suspend();
        }
        
        qDebug() << "Test mode stopped.";
    }
}


void MainWindow::progressBarOutput()
{
    if(!inputDevice || !outputDevice)
    {
        qCritical() << "Input or output device is null.";
        return;
    }

    // data değişkeni zaten efekt tarafından doldurulmuş olmalı
    if(data.isEmpty())
    {
        qWarning() << "No data available for progress bar.";
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


#include "mainwindow.h"
#include "audiopipeline.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QTimer>
#include <QMessageBox>
#include <QDesktopServices>
#include <QProcess>
#include <QIcon>
#include <QThread>

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
    , virtualAudioOutput(nullptr)
    , virtualOutputDevice(nullptr)
    , vbCableFound(false)
    , cableInputSelected(false)
    , testButtonActive(false)
    , soundpackBuffer(new CircularBuffer(32768))
    , effectsBuffer(new CircularBuffer(32768))
    , audioPipeline(new AudioPipeline(this))

{
    ui->setupUi(this);
    ui->frame_3->hide();
    searchInputDevice();
    searchOutputDevice();
    
    // Virtual ayarlar
    searchVirtualDevices();
    detectVBCable();
    updateVirtualStatusLabel();

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
    
    if(virtualAudioOutput){
        virtualAudioOutput->stop();
        delete virtualAudioOutput;
    }

    delete soundpackBuffer;
    delete effectsBuffer;
    delete soundpackBuffer2;

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
    ui->inputlabel->setText(QString::number(value));

    if (audioInput) {
        ui->inputslider->setValue(static_cast<int>(audioInput->volume() * 100));
    }


    float volume = value/100.0f;
    audioInput->setVolume(volume);

}

void MainWindow::on_outputslider_valueChanged(int value)
{
    ui->outputlabel->setText(QString::number(value));

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
    
    // Başlangıçta bağlantı kur - test butonundan bağımsız olarak cable input'a gönder
    if (inputDevice) {
        connect(inputDevice, &QIODevice::readyRead, this, [=](){
            data = inputDevice->readAll();
            progressBarOutput();
            
            // Emit signal for recording when recording is active
            if (isRecording) {
                qDebug() << "EMITTING SIGNAL (INITIAL): Audio size:" << data.size() << "bytes";
                emit audioDataReady(data);
            }
            
            // Her zaman virtual output'a gönder (Cable Input)
            if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                // Buffer doluluğunu kontrol et - overload önle
                if (audioPipeline) {
                    // Effects buffer'ın doluluk oranını kontrol et
                    if (audioPipeline->getInputBufferBytesAvailable() < 32768) { // 32KB'den azsa yaz
                        audioPipeline->writeInputAudio(data);
                    }
                }
            }
            
            // Fiziksel output'a gönderme (test butonu aktif değilse)
        });
        qDebug() << "Initial audio connection established with virtual output only.";
    }
    
    // Input değiştiğinde virtual output'u da yenile
    setupVirtualOutput();
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
    
    // Output değiştiğinde virtual output'u da yenile
    setupVirtualOutput();

}

void MainWindow::on_testButton_clicked(bool checked)
{
    if (checked) {
        ui->testButton->setText("Stop");
        testButtonActive = true;

        // AudioPipeline test modunu ayarla
        if (audioPipeline) {
            audioPipeline->setTestMode(true);
            audioPipeline->setNormalOutputDevice(outputDevice);
        }

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
            if (inputDevice && !isRecording) {
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
                    
                    // Test modunda hem normal output'a hem de virtual output'a gönder
                    // AudioPipeline üzerinden göndererek soundpack ile mix işlemini sağla
                    if (audioPipeline) {
                        if (usingEffects) {
                            // Efekt aktifse effects buffer'ına yaz
                            audioPipeline->writeEffectsAudio(data);
                        } else {
                            // Efekt yoksa input buffer'ına yaz
                            audioPipeline->writeInputAudio(data);
                        }
                        // AudioPipeline processBuffers tetikle
                        audioPipeline->processBuffers();
                    } else {
                        // Fallback: AudioPipeline yoksa doğrudan yaz
                        if (outputDevice && outputDevice->isOpen()) {
                            outputDevice->write(data);
                        }
                        if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                            virtualOutputDevice->write(data);
                        }
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

        // AudioPipeline test modunu kapat
        if (audioPipeline) {
            audioPipeline->setTestMode(false);
            audioPipeline->setNormalOutputDevice(nullptr);
        }

        // Test bağlantısını kopar
        if (inputDevice) {
            disconnect(inputDevice, &QIODevice::readyRead, this, nullptr);
            
            if (usingEffects == false) {
                // Bir efekt aktifse, onun bağlantısını kur
                // Bu stopAllEffects içinde handled olur
                stopAllEffects();
            } else if (!isRecording) {
                // Normal mod için bağlantı kur - virtual output'a her zaman gönder
                connect(inputDevice, &QIODevice::readyRead, this, [=]() {
                    data = inputDevice->readAll();
                    progressBarOutput();
                    // Emit signal for recording when recording is active
                    if (isRecording) {
                        qDebug() << "EMITTING SIGNAL (NORMAL): Audio size:" << data.size() << "bytes";
                        emit audioDataReady(data);
                    }
                    
                    // Normal modda sadece virtual output'a gönder - AudioPipeline üzerinden
                    if (audioPipeline) {
                        if (usingEffects) {
                            // Efekt aktifse effects buffer'ına yaz
                            audioPipeline->writeEffectsAudio(data);
                        } else {
                            // Efekt yoksa input buffer'ına yaz
                            audioPipeline->writeInputAudio(data);
                        }
                        // AudioPipeline processBuffers tetikle
                        audioPipeline->processBuffers();
                    } else {
                        // Fallback: AudioPipeline yoksa doğrudan virtual output'a yaz
                        if (virtualOutputDevice && virtualOutputDevice->isOpen()) {
                            virtualOutputDevice->write(data);
                        }
                    }
                    
                    // Normal modda fiziksel output'a gönderme
                });
            }
        }

        if (audioOutput) {
            audioOutput->suspend();
        }
        
        qDebug() << "Test mode stopped.";
        testButtonActive = false;
    }
}


void MainWindow::progressBarOutput()
{
    // During recording, inputDevice might be disconnected for effect switching
    // Handle this case gracefully
    if(!inputDevice || !outputDevice)
    {
        // Only show critical error when not recording
        if (!isRecording) {
            qCritical() << "Input or output device is null.";
        }
        return;
    }

    // data değişkeni zaten efekt tarafından doldurulmuş olmalı
    if(data.isEmpty())
    {
        // During recording, try to get data from AudioPipeline if input data is empty
        if (isRecording && audioPipeline) {
            QByteArray mixedData = audioPipeline->getMixedAudioData(1024);
            if (!mixedData.isEmpty()) {
                // Use mixed data for progress bar during recording
                qint16 *samples = reinterpret_cast<qint16 *>(mixedData.data());
                int numSamples = mixedData.size() / sizeof(qint16);
                qint16 maxAmplitude = 0;

                for (int i = 0; i < numSamples; ++i) {
                    maxAmplitude = qMax(maxAmplitude, qAbs(samples[i]));
                }

                // Progress bar'ı güncelle (0-100 arası)
                int progress = (maxAmplitude * 100) / 32768;
                ui->inputslider->setValue(progress);
                return;
            }
        }
        
        // During recording, data might be temporarily empty during effect switching
        if (!isRecording) {
            qWarning() << "No data available for progress bar.";
        }
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
    //qDebug() << "Volume Level:" << progressValue;
}

// Virtual Audio fonksiyonları
void MainWindow::searchVirtualDevices()
{
    // Virtual output cihazlarını ara
    ui->virtualcombobox->clear();
    
    QList<QAudioDevice> outputDevices = QMediaDevices::audioOutputs();
    for (const QAudioDevice &device : outputDevices) {
        QString deviceName = device.description();
        
        // VB-CABLE cihazlarını kontrol et
        if (deviceName.contains("VB-CABLE", Qt::CaseInsensitive) || 
            deviceName.contains("Virtual", Qt::CaseInsensitive)) {
            
            ui->virtualcombobox->addItem(deviceName);
            qDebug() << "Virtual device found:" << deviceName;
        }
    }
    
    // Eğer VB-CABLE bulunduysa otomatik seç
    if (ui->virtualcombobox->count() > 0) {
        ui->virtualcombobox->setCurrentIndex(0);
        vbCableFound = true;
    } else {
        vbCableFound = false;
    }
}

bool MainWindow::detectVBCable()
{
    QList<QAudioDevice> inputDevices = QMediaDevices::audioInputs();
    for (const QAudioDevice &device : inputDevices) {
        QString deviceName = device.description();
        
        if (deviceName.contains("VB-CABLE", Qt::CaseInsensitive)) {
            // VB-CABLE Input'ı normal input combobox'a ekle
            ui->inputcombobox->addItem(deviceName);
            
            // Otomatik seç ve kilitle
            int index = ui->inputcombobox->findText(deviceName);
            if (index >= 0) {
                ui->inputcombobox->setCurrentIndex(index);
                cableInputSelected = true;
                ui->inputcombobox->setEnabled(false); // Değiştirilemez yap
                return true;
            }
        }
    }
    
    cableInputSelected = false;
    ui->inputcombobox->setEnabled(true); // Normal kullanım için aktif

    return false;
}

void MainWindow::updateVirtualStatusLabel()
{
    if (vbCableFound) {
        ui->virtualStatusLabel->setText("VB-CABLE Bulundu - Aktif");

    } else {
        ui->virtualStatusLabel->setText("VB-CABLE Bulunamadı - İndirin");
        ui->virtualStatusLabel->setStyleSheet("color: red; ");


        // VB-CABLE bulunamadıysa indirme isteği göster
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "VB-CABLE Driver Gerekli",
            "VB-CABLE ses sürücüsü bulunamadı.\n\nVB-CABLE, sanal ses cihazları oluşturmak için gereklidir.\n\nŞimdi VB-CABLE kurulumunu başlatmak ister misiniz?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
            );

        if (reply == QMessageBox::Yes) {
            // VB-CABLE kurulum betiğini çalıştır
            QString batchPath = QCoreApplication::applicationDirPath() + "/install_vbcable.bat";
            qDebug() << "Running VB-CABLE installer:" << batchPath;

            // Batch dosyasını yönetici olarak çalıştır
            QStringList arguments;
            arguments << "/c" << QString("cmd /c \"%1\"").arg(batchPath);

            bool success = QProcess::startDetached("powershell", arguments);
            if (success) {
                qDebug() << "VB-CABLE installer started successfully";
                QMessageBox::information(this, "Kurulum Başlatıldı",
                                         "VB-CABLE kurulum betiği başlatıldı.\n\nKurulum penceresini takip edin ve talimatlara uyun.");
            } else {
                qWarning() << "Failed to start VB-CABLE installer";
                QMessageBox::warning(this, "Kurulum Hatası",
                                     "VB-CABLE kurulum betiği başlatılamadı.\n\nManuel olarak kurulum dosyasını çalıştırın:\n" + batchPath);
            }
        }


    }
}

void MainWindow::setupVirtualOutput()
{
    if (virtualAudioOutput) {
        virtualAudioOutput->stop();
        delete virtualAudioOutput;
        virtualAudioOutput = nullptr;
    }
    
    if (ui->virtualcombobox->count() == 0) {
        return;
    }
    
    QList<QAudioDevice> outputDevices = QMediaDevices::audioOutputs();
    for (const QAudioDevice &device : outputDevices) {
        if (device.description() == ui->virtualcombobox->currentText()) {
            format->setSampleRate(48000);
            format->setChannelCount(2);
            format->setSampleFormat(QAudioFormat::Int16);
            
            if (device.isFormatSupported(*format)) {
                virtualAudioOutput = new QAudioSink(device, *format);
                virtualOutputDevice = virtualAudioOutput->start();
                
                // Audio pipeline'ı virtual output device ile bağla
                audioPipeline->setVirtualOutputDevice(virtualOutputDevice);
                audioPipeline->start();
                
                // AudioPipeline recording sinyalini bağla
                connect(audioPipeline, &AudioPipeline::processedAudioReady, this, &MainWindow::audioDataReady);
                
                qDebug() << "Virtual output setup completed for:" << device.description();
            } else {
                qWarning() << "Format not supported for virtual device:" << device.description();
            }
            break;
        }
    }
}

void MainWindow::on_virtualcombobox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    setupVirtualOutput();
}

void MainWindow::on_virtualslider_valueChanged(int value)
{
    if (virtualAudioOutput) {
        float volume = static_cast<float>(value) / 100.0f;
        virtualAudioOutput->setVolume(volume);
    }
}



#include "mainwindow.h"
#include "audiopipeline.h"
#include "ui_mainwindow.h"
#include "gloweffekt.h"
#include <QSettings>
#include <QTimer>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QIcon>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#endif
#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#endif
#ifdef Q_OS_MACOS
#include <Carbon/Carbon.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VoiceChangerMainWindow)
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
    , voiceEffects(new VoiceEffects(this))

{
    ui->setupUi(this);

    // Initialize glow effects
    glowEffect = new GlowEffect(ui, this);
    
    // Initialize preset notification widget
    presetNotification = new PresetNotification(this);
    
    // Initialize animation widget for sidebar transitions
    animationWidget = new AnimationWidget(this);
    
    // Initialize recording animations
    setupRecordingAnimations();

    // Remove top default bar but keep in taskbar and side borders for resizing
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);

    // Initialize dragging variables
    isDragging = false;
    isMaximizing = false;

    ui->EffectsSideBarWidget->hide();
    ui->presetsSidebarWidget->hide();

    searchInputDevice();
    searchOutputDevice();
    
    // Auto-select first input device if available
    if (ui->inputDeviceCombobox->count() > 0) {
        ui->inputDeviceCombobox->setCurrentIndex(0);
    }
    
    // Auto-select first output device if available  
    if (ui->OutputDEviceCombobox->count() > 0) {
        ui->OutputDEviceCombobox->setCurrentIndex(0);
    }
    
    // Virtual ayarlar
    searchVirtualDevices();
    detectVBCable();
    updateVirtualStatusLabel();

    // Setup hotkeys
    allinputKeys();
    connectAllHotkeys();

    // Populate hotkey dropdowns
    for (int i = 1; i <= 20; ++i) {
        QString name = QString("slot%1Hotkey").arg(i);
        QComboBox *combo = findChild<QComboBox*>(name);
        if (combo) {
            populateComboBox(combo);
        }
    }
    
    // Install event filter for hotkey handling
    this->installEventFilter(this);
    
    // Install event filter for titlebar dragging
    ui->titleBar->installEventFilter(this);
    
    // Connect titlebar buttons
    connect(ui->minimizeButton, &QPushButton::clicked, this, &MainWindow::on_minimizeButton_clicked);
    connect(ui->maximizeButton, &QPushButton::clicked, this, &MainWindow::on_maximizeButton_clicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::on_closeButton_clicked);
    
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
#ifdef Q_OS_LINUX
    if (m_x11Display) {
        XCloseDisplay(m_x11Display);
        m_x11Display = nullptr;
    }
#endif
#ifdef Q_OS_MACOS
    for (auto it = m_macHotkeyRefs.begin(); it != m_macHotkeyRefs.end(); ++it) {
        UnregisterEventHotKey(it.value());
    }
    m_macHotkeyRefs.clear();
    if (m_macEventHandler) {
        RemoveEventHandler(m_macEventHandler);
        m_macEventHandler = nullptr;
    }
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
    delete voiceEffects;

    delete format;

    // Clean up audio cache
    for (auto it = audioCache.begin(); it != audioCache.end(); ++it) {
        delete it.value();
    }
    audioCache.clear();

    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Handle titlebar dragging
    if (obj == ui->titleBar) {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            if (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
                isDragging = true;
                dragPosition = static_cast<QMouseEvent*>(event)->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
            break;
        case QEvent::MouseMove:
            if (isDragging && (static_cast<QMouseEvent*>(event)->buttons() & Qt::LeftButton)) {
                move(static_cast<QMouseEvent*>(event)->globalPosition().toPoint() - dragPosition);
                return true;
            }
            break;
        case QEvent::MouseButtonRelease:
            if (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton) {
                isDragging = false;
                return true;
            }
            break;
        default:
            break;
        }
    }
    
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
                    case 1: on_slot1_clicked(); break;
                    case 2: on_slot2_clicked(); break;
                    case 3: on_slot3_clicked(); break;
                    case 4: on_slot4_clicked(); break;
                    case 5: on_slot5_clicked(); break;
                    case 6: on_slot6_clicked(); break;
                    case 7: on_slot7_clicked(); break;
                    case 8: on_slot8_clicked(); break;
                    case 9: on_slot9_clicked(); break;
                    case 10: on_slot10_clicked(); break;
                    case 11: on_slot11_clicked(); break;
                    case 12: on_slot12_clicked(); break;
                    case 13: on_slot13_clicked(); break;
                    case 14: on_slot14_clicked(); break;
                    case 15: on_slot15_clicked(); break;
                    case 16: on_slot16_clicked(); break;
                    case 17: on_slot17_clicked(); break;
                    case 18: on_slot18_clicked(); break;
                    case 19: on_slot19_clicked(); break;
                    case 20: on_slot20_clicked(); break;
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
        ui->inputDeviceCombobox->addItem(device.description(), QVariant::fromValue(device) ); //bakılacak
    }
}

void MainWindow::searchOutputDevice()
{
    const auto devicesO = QMediaDevices::audioOutputs();
    for(const QAudioDevice &device : devicesO)
    {
        ui->OutputDEviceCombobox->addItem(device.description(), QVariant::fromValue(device));
    }

}


void MainWindow::on_inputslider_valueChanged(int value)
{
    if (audioInput) {
        ui->inputSlider->setValue(static_cast<int>(audioInput->volume() * 100));
    }


    float volume = value/100.0f;
    audioInput->setVolume(volume);

}

void MainWindow::on_outputslider_valueChanged(int value)
{

    if (audioOutput) {
        ui->outputSlider->setValue(static_cast<int>(audioOutput->volume() * 100));

    }

    float volume = value/100.0f;
    audioOutput->setVolume(volume);
}


void MainWindow::on_refreshInputDevice_clicked()
{

    qDebug()<< ui->inputDeviceCombobox->count();

    for(int i= ui->inputDeviceCombobox->count(); i>-1 ; i--)
    {
        ui->inputDeviceCombobox->removeItem(i);
    }

    searchInputDevice();
    
    // Auto-select first input device if available
    if (ui->inputDeviceCombobox->count() > 0) {
        ui->inputDeviceCombobox->setCurrentIndex(0);
    }
}


void MainWindow::on_refreshOutputDevice_clicked()
{
    qDebug()<< ui->OutputDEviceCombobox->count();

    for(int i= ui->OutputDEviceCombobox->count(); i>-1 ; i--)
    {
        ui->OutputDEviceCombobox->removeItem(i);
    }

    searchOutputDevice();
    
    // Auto-select first output device if available
    if (ui->OutputDEviceCombobox->count() > 0) {
        ui->OutputDEviceCombobox->setCurrentIndex(0);
    }

}



void MainWindow::on_inputDeviceCombobox_currentIndexChanged(int index)
{
    // Varsayılan format ayarları
    format->setSampleRate(48000);
    format->setChannelCount(2);
    format->setSampleFormat(QAudioFormat::Int16);


    QVariant inputData = ui->inputDeviceCombobox->itemData(index);
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



void MainWindow::on_OutputDEviceCombobox_currentIndexChanged(int index)
{
    // Varsayılan format ayarları
    format->setSampleRate(48000);
    format->setChannelCount(2);
    format->setSampleFormat(QAudioFormat::Int16);


    QVariant outputData = ui->OutputDEviceCombobox->itemData(index);
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
                        } else if (ui->childButton->isChecked()) {
                            processToChildVoice(data);
                        } else if (ui->devilButton->isChecked()) {
                            processToDevilVoice(data);
                        } else if (ui->femaleButton->isChecked()) {
                            processToFemaleVoice(data);
                        } else if (ui->combineButton->isChecked()) {
                            processToCombineVoice(data);
                        } else if (ui->caveButton->isChecked()) {
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
        // AudioPipeline test modunu kapat
        if (audioPipeline) {
            audioPipeline->setTestMode(false);
            audioPipeline->setNormalOutputDevice(nullptr);
            // Buffer'ları temizle CPU kullanımını azaltmak için
            audioPipeline->clearBuffers();
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
                ui->inputSlider->setValue(progress);
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
    ui->virtualInputcombobox->clear();
    
    QList<QAudioDevice> outputDevices = QMediaDevices::audioOutputs();
    for (const QAudioDevice &device : outputDevices) {
        QString deviceName = device.description();
        
        // Check for virtual audio devices (cross-platform)
        if (deviceName.contains("VB-CABLE", Qt::CaseInsensitive) || 
            deviceName.contains("Virtual", Qt::CaseInsensitive) ||
            deviceName.contains("BlackHole", Qt::CaseInsensitive) ||
            deviceName.contains("Soundflower", Qt::CaseInsensitive) ||
            deviceName.contains("Loopback", Qt::CaseInsensitive) ||
            (deviceName.contains("Audio", Qt::CaseInsensitive) &&
            (deviceName.contains("Virtual", Qt::CaseInsensitive) || 
             deviceName.contains("Cable", Qt::CaseInsensitive)))) {
            
            ui->virtualInputcombobox->addItem(deviceName);
        }
    }
    
    // Auto-select first virtual device if found
    if (ui->virtualInputcombobox->count() > 0) {
        ui->virtualInputcombobox->setCurrentIndex(0);
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
        
        // Check for virtual audio input devices (cross-platform)
        if (deviceName.contains("VB-CABLE", Qt::CaseInsensitive) ||
            deviceName.contains("BlackHole", Qt::CaseInsensitive) ||
            deviceName.contains("Soundflower", Qt::CaseInsensitive) ||
            deviceName.contains("Loopback", Qt::CaseInsensitive) ||
            (deviceName.contains("Virtual", Qt::CaseInsensitive) && 
             deviceName.contains("Input", Qt::CaseInsensitive))) {
            
            // Add virtual input device to normal input combobox
            ui->inputDeviceCombobox->addItem(deviceName);
            
            // Auto-select and lock it
            int index = ui->inputDeviceCombobox->findText(deviceName);
            if (index >= 0) {
                ui->inputDeviceCombobox->setCurrentIndex(index);
                cableInputSelected = true;
                ui->inputDeviceCombobox->setEnabled(false); // Değiştirilemez yap
                return true;
            }
        }
    }
    
    cableInputSelected = false;
    ui->inputDeviceCombobox->setEnabled(true); // Normal kullanım için aktif

    return false;
}

void MainWindow::updateVirtualStatusLabel()
{
    if (vbCableFound) {
        // Find the virtual audio device name
        QString deviceName = "Unknown";
        if (ui->virtualInputcombobox->count() > 0) {
            deviceName = ui->virtualInputcombobox->currentText();
        }
        
        ui->VirtualDriverButton->setText("Driver Found: " + deviceName);
        ui->VirtualDriverButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #27ae60;"
            "	 font: 10px;"
            "    color: white;"
            "    border: none;"
            "    padding: 8px;"
            "    border-radius: 4px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: #229954;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #1e8449;"
            "}"
        );
    } else {
        ui->VirtualDriverButton->setText("Driver Not Found");
        ui->VirtualDriverButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #e74c3c;"
            "	 font:10px;"
            "    color: white;"
            "    border: none;"
            "    padding: 8px;"
            "    border-radius: 4px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: #c0392b;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #a93226;"
            "}"
        );
    }
}

void MainWindow::on_VirtualDriverButton_clicked()
{
    if (!vbCableFound) {
        // Open VB-CABLE download page
        QDesktopServices::openUrl(QUrl("https://vb-audio.com/Cable/"));
    }
    // If driver is found, do nothing (button is just for display)
}

void MainWindow::showVirtualDeviceInstructions()
{
    QString instructions;
    QString downloadUrl;
    
#ifdef Q_OS_WIN
    instructions = "Windows Virtual Audio Device Installation:\n\n"
                   "1. Download VB-CABLE A+B (free) from: https://vb-audio.com/Cable/\n"
                   "2. Run the installer as Administrator\n"
                   "3. Restart your computer\n"
                   "4. Launch VoiceChanger again\n\n"
                   "Alternative: BlackHole for Windows (if available)";
    downloadUrl = "https://vb-audio.com/Cable/";
#elif defined(Q_OS_MACOS)
    instructions = "macOS Virtual Audio Device Installation:\n\n"
                   "Option 1: BlackHole (Recommended)\n"
                   "1. Download BlackHole from: https://github.com/ExistentialAudio/BlackHole\n"
                   "2. Install the .pkg file\n"
                   "3. Restart your computer\n"
                   "4. Launch VoiceChanger again\n\n"
                   "Option 2: Soundflower (Legacy)\n"
                   "1. Download Soundflower from: https://github.com/mattingalls/Soundflower\n"
                   "2. Install the .pkg file\n"
                   "3. Restart your computer\n"
                   "4. Launch VoiceChanger again";
    downloadUrl = "https://github.com/ExistentialAudio/BlackHole";
#elif defined(Q_OS_LINUX)
    instructions = "Linux Virtual Audio Device Installation:\n\n"
                   "Option 1: Using PulseAudio\n"
                   "1. Install pavucontrol: sudo apt install pavucontrol\n"
                   "2. Create a null sink: pactl load-module module-null-sink sink_name=virtual\n"
                   "3. Use pavucontrol to route audio\n\n"
                   "Option 2: Using JACK Audio Connection Kit\n"
                   "1. Install JACK: sudo apt install jackd2\n"
                   "2. Configure JACK for virtual routing\n\n"
                   "Option 3: Using Loopback devices\n"
                   "1. Install ALSA loopback: sudo modprobe snd-aloop\n"
                   "2. Configure in your .asoundrc file";
    downloadUrl = "https://wiki.archlinux.org/title/PulseAudio/Examples";
#else
    instructions = "Virtual Audio Device Installation:\n\n"
                   "Please search for virtual audio software for your operating system.\n"
                   "Common options include:\n"
                   "- VB-CABLE (Windows)\n"
                   "- BlackHole (macOS)\n"
                   "- PulseAudio null sink (Linux)\n"
                   "- JACK Audio Connection Kit (Cross-platform)";
    downloadUrl = "https://vb-audio.com/Cable/";
#endif
    
    QMessageBox::information(this, "Virtual Audio Device Installation", instructions);
    
    // Open download page
    QDesktopServices::openUrl(QUrl(downloadUrl));
}

void MainWindow::setupVirtualOutput()
{
    if (virtualAudioOutput) {
        virtualAudioOutput->stop();
        delete virtualAudioOutput;
        virtualAudioOutput = nullptr;
    }
    
    if (ui->virtualInputcombobox->count() == 0) {
        return;
    }
    
    QList<QAudioDevice> outputDevices = QMediaDevices::audioOutputs();
    for (const QAudioDevice &device : outputDevices) {
        if (device.description() == ui->virtualInputcombobox->currentText()) {
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

// Mouse event functions for titlebar dragging
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    isDragging = false;
    event->accept();
}

void MainWindow::on_virtualInputcombobox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    setupVirtualOutput();
}

void MainWindow::on_VirtualSlider_valueChanged(int value)
{
    if (virtualAudioOutput) {
        float volume = static_cast<float>(value) / 100.0f;
        virtualAudioOutput->setVolume(volume);
    }
}

// Titlebar button functions
void MainWindow::on_minimizeButton_clicked()
{
    showMinimized();
}

void MainWindow::on_maximizeButton_clicked()
{
    // Prevent multiple rapid clicks
    if (isMaximizing) {
        qDebug() << "Maximize operation in progress, ignoring click";
        return;
    }
    
    isMaximizing = true;
    
    // Check current window state before changing
    Qt::WindowStates currentState = windowState();
    
    if (currentState & Qt::WindowMaximized) {
        // Window is currently maximized, restore it
        showNormal();
        ui->maximizeButton->setText("□");
        qDebug() << "Window restored to normal size";
    } else {
        // Window is not maximized, maximize it
        showMaximized();
        ui->maximizeButton->setText("❐");
        qDebug() << "Window maximized";
    }
    
    // Reset the flag after a short delay to prevent rapid clicking
    QTimer::singleShot(200, [this]() {
        isMaximizing = false;
    });
}

void MainWindow::on_closeButton_clicked()
{
    close();
}

void MainWindow::on_recordingButton_clicked(bool checked)
{
    if (checked) {
        // Start recording
        startRecording();
        // Animation is started in startRecording() function
    } else {
        // Stop recording
        stopRecording();
        // Animation is stopped in stopRecording() function
    }
}

// Animated sidebar transition functions
void MainWindow::on_miniSidebarToggleBtn_clicked()
{
    // Animate from mini sidebar to presets sidebar
    animationWidget->animateSidebarTransition(ui->presetsSidebarWidget, ui->miniSidebarWidget, "right", 350);
}

void MainWindow::on_presetsSidebarHideButton_clicked()
{
    // Animate from presets sidebar back to mini sidebar
    animationWidget->animateSidebarTransition(ui->miniSidebarWidget, ui->presetsSidebarWidget, "left", 350);
}

void MainWindow::on_toggleEffectsSidebar_clicked()
{
    // Animate from settings sidebar to effects sidebar
    animationWidget->animateSidebarTransition(ui->EffectsSideBarWidget, ui->settingsSidebarWidget, "left", 350);
}

void MainWindow::on_settingsHideButton_clicked()
{
    // Animate from effects sidebar back to settings sidebar
    animationWidget->animateSidebarTransition(ui->settingsSidebarWidget, ui->EffectsSideBarWidget, "right", 350);
}



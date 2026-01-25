#include "hotkeys.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h>
#endif
#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif
#ifdef Q_OS_MACOS
#include <Carbon/Carbon.h>
#endif

hotkeys::hotkeys(QWidget *parent)
    : QMainWindow{parent}
{}

QStringList allKeys;

void MainWindow::allinputKeys() {
    // Populate the list of available keys for the comboboxes
    for(int k = Qt::Key_A; k <= Qt::Key_Z; ++k) {
        allKeys << QKeySequence(k).toString();
    }
    for(int k = Qt::Key_F1; k <= Qt::Key_F12; ++k) {
        allKeys << QKeySequence(k).toString();
    }
    // Add numpad keys
    for(int k = Qt::Key_0; k <= Qt::Key_9; ++k) {
        allKeys << QKeySequence(k).toString();
    }
    allKeys << "Num+" << "Num-" << "Num*" << "Num/" << "Num.";
    allKeys << "NumEnter" << "NumLock" << "NumDelete";
    allKeys.prepend("None"); // Option to have no hotkey
}

void MainWindow::populateComboBox(QComboBox *combo) {
    combo->addItems(allKeys);
    combo->setCurrentIndex(0); // "None" is selected by default
}

void MainWindow::handleHotkeyChange(const QString &key, int soundIndex) {
    // Note: soundIndex is 1-based here for easier mapping to hotkey numbers
    if (key.isEmpty() || key == "None") {
        // Remove existing shortcuts if any
        if (m_soundIndexToKey.contains(soundIndex)) {
            QString oldKey = m_soundIndexToKey[soundIndex];
            
            // Remove QShortcut
            if (m_shortcuts.contains(oldKey)) {
                delete m_shortcuts[oldKey];
                m_shortcuts.remove(oldKey);
            }
            
            // Remove global hotkey
#ifdef Q_OS_WIN
            if (m_globalHotkeyIds.contains(oldKey)) {
                UnregisterHotKey((HWND)this->winId(), m_globalHotkeyIds[oldKey]);
                m_globalHotkeyIds.remove(oldKey);
            }
#endif
#ifdef Q_OS_LINUX
            // Linux cleanup handled in destructor
#endif
#ifdef Q_OS_MACOS
            if (m_macHotkeyRefs.contains(oldKey)) {
                UnregisterEventHotKey(m_macHotkeyRefs[oldKey]);
                m_macHotkeyRefs.remove(oldKey);
            }
#endif
            
            m_hotkeyAssignments.remove(oldKey);
            m_soundIndexToKey.remove(soundIndex);
        }
        return;
    }

    // Remove old shortcuts if this sound had one
    if (m_soundIndexToKey.contains(soundIndex)) {
        QString oldKey = m_soundIndexToKey[soundIndex];
        
        // Remove QShortcut
        if (m_shortcuts.contains(oldKey)) {
            delete m_shortcuts[oldKey];
            m_shortcuts.remove(oldKey);
        }
        
        // Remove global hotkey
#ifdef Q_OS_WIN
        if (m_globalHotkeyIds.contains(oldKey)) {
            UnregisterHotKey((HWND)this->winId(), m_globalHotkeyIds[oldKey]);
            m_globalHotkeyIds.remove(oldKey);
        }
#endif
#ifdef Q_OS_LINUX
        // Linux cleanup handled in destructor
#endif
#ifdef Q_OS_MACOS
        if (m_macHotkeyRefs.contains(oldKey)) {
            UnregisterEventHotKey(m_macHotkeyRefs[oldKey]);
            m_macHotkeyRefs.remove(oldKey);
        }
#endif
        
        m_hotkeyAssignments.remove(oldKey);
    }

    // Handle conflicting assignment
    if (m_hotkeyAssignments.contains(key) && m_hotkeyAssignments[key] != soundIndex) {
        int conflictingSoundIndex = m_hotkeyAssignments[key];
        
        QString comboName = QString("slot%1Hotkey").arg(conflictingSoundIndex);
        QComboBox *conflictingCombo = findChild<QComboBox*>(comboName);
        if (conflictingCombo) {
            conflictingCombo->blockSignals(true);
            conflictingCombo->setCurrentIndex(0); // Set to "None"
            conflictingCombo->blockSignals(false);
        }
        
        // Remove conflicting shortcuts
        if (m_shortcuts.contains(key)) {
            delete m_shortcuts[key];
            m_shortcuts.remove(key);
        }
        
#ifdef Q_OS_WIN
        if (m_globalHotkeyIds.contains(key)) {
            UnregisterHotKey((HWND)this->winId(), m_globalHotkeyIds[key]);
            m_globalHotkeyIds.remove(key);
        }
#endif
#ifdef Q_OS_LINUX
        // Linux cleanup handled in destructor
#endif
#ifdef Q_OS_MACOS
        if (m_macHotkeyRefs.contains(key)) {
            UnregisterEventHotKey(m_macHotkeyRefs[key]);
            m_macHotkeyRefs.remove(key);
        }
#endif
        
        m_soundIndexToKey.remove(conflictingSoundIndex);
    }

    // Create new QShortcut (for when app is focused)
    QKeySequence shortcutKey;
    
    // Handle special numpad keys
    if (key == "Num+") {
        shortcutKey = QKeySequence(Qt::Key_Plus);
    } else if (key == "Num-") {
        shortcutKey = QKeySequence(Qt::Key_Minus);
    } else if (key == "Num*") {
        shortcutKey = QKeySequence(Qt::Key_Asterisk);
    } else if (key == "Num/") {
        shortcutKey = QKeySequence(Qt::Key_Slash);
    } else if (key == "Num.") {
        shortcutKey = QKeySequence(Qt::Key_Period);
    } else if (key == "NumEnter") {
        shortcutKey = QKeySequence(Qt::Key_Enter);
    } else if (key == "NumLock") {
        shortcutKey = QKeySequence(Qt::Key_CapsLock);
    } else if (key == "NumDelete") {
        shortcutKey = QKeySequence(Qt::Key_Delete);
    } else {
        // Regular keys
        shortcutKey = QKeySequence::fromString(key);
    }

    QShortcut *shortcut = new QShortcut(shortcutKey, this);
    connect(shortcut, &QShortcut::activated, this, [this, soundIndex]() {
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
    });

    m_shortcuts[key] = shortcut;

    // Create global hotkey (for background operation)
#ifdef Q_OS_WIN
    UINT modifiers = 0;
    UINT vkCode = 0;
    
    // Convert key to Windows virtual key code
    if (key == "Num+") {
        vkCode = VK_ADD;
        modifiers = 0;
    } else if (key == "Num-") {
        vkCode = VK_SUBTRACT;
        modifiers = 0;
    } else if (key == "Num*") {
        vkCode = VK_MULTIPLY;
        modifiers = 0;
    } else if (key == "Num/") {
        vkCode = VK_DIVIDE;
        modifiers = 0;
    } else if (key == "Num.") {
        vkCode = VK_DECIMAL;
        modifiers = 0;
    } else if (key == "NumEnter") {
        vkCode = VK_RETURN;
        modifiers = 0;
    } else if (key.length() == 1 && key[0] >= '0' && key[0] <= '9') {
        vkCode = VK_NUMPAD0 + (key[0].unicode() - '0');
        modifiers = 0;
    } else if (key.length() == 1 && key[0] >= 'A' && key[0] <= 'Z') {
        vkCode = key[0].unicode();
    } else if (key.startsWith("F")) {
        int fNum = key.mid(1).toInt();
        if (fNum >= 1 && fNum <= 24) {
            vkCode = VK_F1 + fNum - 1;
        }
    }
    
    if (vkCode != 0) {
        int hotkeyId = GLOBAL_HOTKEY_BASE_ID + soundIndex;
        if (RegisterHotKey((HWND)this->winId(), hotkeyId, modifiers, vkCode)) {
            m_globalHotkeyIds[key] = hotkeyId;
            qDebug() << "Global hotkey registered for key:" << key << "ID:" << hotkeyId;
        } else {
            qDebug() << "Failed to register global hotkey for key:" << key << "Error:" << GetLastError();
        }
    }
#endif
#ifdef Q_OS_LINUX
    // Linux X11 global hotkey implementation
    if (!m_x11Display) {
        m_x11Display = XOpenDisplay(nullptr);
        if (!m_x11Display) {
            qWarning() << "Could not open X11 display for global hotkeys";
            return;
        }
        m_rootWindow = DefaultRootWindow(m_x11Display);
    }
    
    // Convert Qt key to X11 keysym
    KeySym keysym = 0;
    if (key == "Num+") keysym = XK_KP_Add;
    else if (key == "Num-") keysym = XK_KP_Subtract;
    else if (key == "Num*") keysym = XK_KP_Multiply;
    else if (key == "Num/") keysym = XK_KP_Divide;
    else if (key == "Num.") keysym = XK_KP_Decimal;
    else if (key == "NumEnter") keysym = XK_KP_Enter;
    else if (key.length() == 1 && key[0] >= '0' && key[0] <= '9') keysym = XK_KP_0 + (key[0].unicode() - '0');
    else if (key.length() == 1 && key[0] >= 'A' && key[0] <= 'Z') keysym = key[0].unicode();
    else if (key.startsWith("F")) {
        int fNum = key.mid(1).toInt();
        if (fNum >= 1 && fNum <= 35) keysym = XK_F1 + fNum - 1;
    }
    
    if (keysym != 0) {
        // For Linux, we'll use Qt's global shortcut mechanism as fallback
        // since X11 global hotkeys require complex event filtering
        qDebug() << "Global hotkey for Linux using Qt shortcut:" << key;
    }
#endif
#ifdef Q_OS_MACOS
    // macOS global hotkey implementation
    OSStatus err;
    EventHotKeyID hotKeyID;
    hotKeyID.id = GLOBAL_HOTKEY_BASE_ID + soundIndex;
    hotKeyID.signature = 'VCHG';
    
    // Convert Qt key to macOS key code
    UInt32 keyCode = 0;
    if (key == "Num+") keyCode = kVK_ANSI_Equal;
    else if (key == "Num-") keyCode = kVK_ANSI_Minus;
    else if (key == "Num*") keyCode = kVK_ANSI_8;
    else if (key == "Num/") keyCode = kVK_ANSI_Slash;
    else if (key == "Num.") keyCode = kVK_ANSI_Period;
    else if (key == "NumEnter") keyCode = kVK_Return;
    else if (key.length() == 1 && key[0] >= '0' && key[0] <= '9') keyCode = kVK_ANSI_0 + (key[0].unicode() - '0');
    else if (key.length() == 1 && key[0] >= 'A' && key[0] <= 'Z') keyCode = kVK_ANSI_A + (key[0].unicode() - 'A');
    else if (key.startsWith("F")) {
        int fNum = key.mid(1).toInt();
        if (fNum >= 1 && fNum <= 35) keyCode = kVK_F1 + fNum - 1;
    }
    
    if (keyCode != 0) {
        EventHotKeyRef hotKeyRef;
        err = RegisterEventHotKey(keyCode, 0, hotKeyID, GetApplicationEventTarget(), 0, &hotKeyRef);
        if (err == noErr) {
            m_macHotkeyRefs[key] = hotKeyRef;
            qDebug() << "Global hotkey registered for macOS key:" << key;
        } else {
            qDebug() << "Failed to register global hotkey for macOS key:" << key << "Error:" << err;
        }
    }
#endif

    m_hotkeyAssignments[key] = soundIndex;
    m_soundIndexToKey[soundIndex] = key;
}

void MainWindow::connectAllHotkeys()
{
    // Connect hotkey signals to slots
    connect(ui->slot1Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot1Hotkey_currentTextChanged);
    connect(ui->slot2Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot2Hotkey_currentTextChanged);
    connect(ui->slot3Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot3Hotkey_currentTextChanged);
    connect(ui->slot4Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot4Hotkey_currentTextChanged);
    connect(ui->slot5Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot5Hotkey_currentTextChanged);
    connect(ui->slot6Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot6Hotkey_currentTextChanged);
    connect(ui->slot7Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot7Hotkey_currentTextChanged);
    connect(ui->slot8Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot8Hotkey_currentTextChanged);
    connect(ui->slot9Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot9Hotkey_currentTextChanged);
    connect(ui->slot10Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot10Hotkey_currentTextChanged);
    connect(ui->slot11Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot11Hotkey_currentTextChanged);
    connect(ui->slot12Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot12Hotkey_currentTextChanged);
    connect(ui->slot13Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot13Hotkey_currentTextChanged);
    connect(ui->slot14Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot14Hotkey_currentTextChanged);
    connect(ui->slot15Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot15Hotkey_currentTextChanged);
    connect(ui->slot16Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot16Hotkey_currentTextChanged);
    connect(ui->slot17Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot17Hotkey_currentTextChanged);
    connect(ui->slot18Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot18Hotkey_currentTextChanged);
    connect(ui->slot19Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot19Hotkey_currentTextChanged);
    connect(ui->slot20Hotkey, &QComboBox::currentTextChanged, this, &MainWindow::on_slot20Hotkey_currentTextChanged);
}

// --- Slots for Hotkey ComboBoxes ---

void MainWindow::on_slot1Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 1); }
void MainWindow::on_slot2Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 2); }
void MainWindow::on_slot3Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 3); }
void MainWindow::on_slot4Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 4); }
void MainWindow::on_slot5Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 5); }
void MainWindow::on_slot6Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 6); }
void MainWindow::on_slot7Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 7); }
void MainWindow::on_slot8Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 8); }
void MainWindow::on_slot9Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 9); }
void MainWindow::on_slot10Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 10); }
void MainWindow::on_slot11Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 11); }
void MainWindow::on_slot12Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 12); }
void MainWindow::on_slot13Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 13); }
void MainWindow::on_slot14Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 14); }
void MainWindow::on_slot15Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 15); }
void MainWindow::on_slot16Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 16); }
void MainWindow::on_slot17Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 17); }
void MainWindow::on_slot18Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 18); }
void MainWindow::on_slot19Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 19); }
void MainWindow::on_slot20Hotkey_currentTextChanged(const QString &key) { handleHotkeyChange(key, 20); }

void MainWindow::saveHotkeys()
{
    QSettings settings("VoiceChanger", "Hotkeys");
    settings.clear();
    
    int savedCount = 0;
    for (auto it = m_soundIndexToKey.begin(); it != m_soundIndexToKey.end(); ++it) {
        int soundIndex = it.key();
        QString key = it.value();
        if (key != "None" && !key.isEmpty()) {
            settings.setValue(QString("slot%1Hotkey").arg(soundIndex), key);
            savedCount++;
            qDebug() << "Saved hotkey for slot" << soundIndex << ":" << key;
        }
    }
    
    qDebug() << "Hotkeys saved to settings. Total saved:" << savedCount;
}

void MainWindow::loadHotkeys()
{
    QSettings settings("VoiceChanger", "Hotkeys");
    
    for (int soundIndex = 1; soundIndex <= 20; ++soundIndex) {
        QString key = settings.value(QString("slot%1Hotkey").arg(soundIndex), "None").toString();
        
        // Find the corresponding combobox and set its value
        QString comboName = QString("slot%1Hotkey").arg(soundIndex);
        QComboBox *combo = findChild<QComboBox*>(comboName);
        if (combo) {
            int index = combo->findText(key);
            if (index >= 0) {
                combo->blockSignals(true);
                combo->setCurrentIndex(index);
                combo->blockSignals(false);
                
                // Trigger the hotkey change to register it
                handleHotkeyChange(key, soundIndex);
            }
        }
    }
    
    qDebug() << "Hotkeys loaded from settings";
}

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDebug>

class VBCableInstaller : public QObject
{
    Q_OBJECT

public:
    explicit VBCableInstaller(QObject *parent = nullptr) : QObject(parent) {}

    bool checkVBCableInstalled()
    {
        // Check system drivers folder
        QString system32Path = "C:/Windows/System32/drivers/";
        if (QFile::exists(system32Path + "vbaudio_cable64.sys") || 
            QFile::exists(system32Path + "vbaudio_cable.sys")) {
            return true;
        }

        // Check Program Files
        if (QDir("C:/Program Files/VB-CABLE").exists() || 
            QDir("C:/Program Files (x86)/VB-CABLE").exists()) {
            return true;
        }

        return false;
    }

    void showVBCableDialog()
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("VB-CABLE Required");
        msgBox.setText("VB-CABLE virtual audio cable is not installed.");
        msgBox.setInformativeText("VB-CABLE is required for virtual audio output. "
                                "Would you like to download and install it?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        
        if (msgBox.exec() == QMessageBox::Yes) {
            openDownloadPage();
        }
    }

    void openDownloadPage()
    {
        // Open VB-CABLE download page
        QString url = "https://vb-audio.com/Cable/";
        
#ifdef Q_OS_WIN
        // Use default browser on Windows
        QProcess::startDetached("cmd", QStringList() << "/c" << "start" << url);
#else
        // Generic way for other platforms
        QProcess::startDetached("xdg-open", QStringList() << url);
#endif
        
        QMessageBox::information(nullptr, "Installation Instructions",
            "1. Download VB-CABLE A+B (free version)\n"
            "2. Run the installer as Administrator\n"
            "3. Restart your computer\n"
            "4. Launch VoiceChanger again\n\n"
            "The download page has been opened in your browser.");
    }

    bool checkAndPrompt()
    {
        if (!checkVBCableInstalled()) {
            showVBCableDialog();
            return false;
        }
        return true;
    }
};
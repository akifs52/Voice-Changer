#include "loadouts.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>

loadouts::loadouts(QWidget *parent)
    : QMainWindow{parent}
{}


void MainWindow::saveLoadout(const QString &loadout)
{
    QSettings settings("save", "voiceChanger");

    settings.beginGroup(loadout);

    settings.setValue("filename1",filename1);
    settings.setValue("filename2",filename2);
    settings.setValue("filename3",filename3);
    settings.setValue("filename4",filename4);
    settings.setValue("filename5",filename5);
    settings.setValue("filename6",filename6);
    settings.setValue("filename7",filename7);
    settings.setValue("filename8",filename8);
    settings.setValue("filename9",filename9);
    settings.setValue("filename10",filename10);
    settings.setValue("filename11",filename11);
    settings.setValue("filename12",filename12);
    settings.setValue("filename13",filename13);
    settings.setValue("filename14",filename14);
    settings.setValue("filename15",filename15);
    settings.setValue("filename16",filename16);
    settings.setValue("filename17",filename17);
    settings.setValue("filename18",filename18);
    settings.setValue("filename19",filename19);
    settings.setValue("filename20",filename20);
    settings.setValue("pic1",pic1);
    settings.setValue("pic2",pic2);
    settings.setValue("pic3",pic3);
    settings.setValue("pic4",pic4);
    settings.setValue("pic5",pic5);
    settings.setValue("pic6",pic6);
    settings.setValue("pic7",pic7);
    settings.setValue("pic8",pic8);
    settings.setValue("pic9",pic9);
    settings.setValue("pic10",pic10);
    settings.setValue("pic11",pic11);
    settings.setValue("pic12",pic12);
    settings.setValue("pic13",pic13);
    settings.setValue("pic14",pic14);
    settings.setValue("pic15",pic15);
    settings.setValue("pic16",pic16);
    settings.setValue("pic17",pic17);
    settings.setValue("pic18",pic18);
    settings.setValue("pic19",pic19);
    settings.setValue("pic20",pic20);

    settings.endGroup();
}

void MainWindow::loadLoadout(const QString &loadout)
{

    QSettings settings ("save","voiceChanger");

    settings.beginGroup(loadout);


    filename1 = settings.value("filename1").toString();
    filename2 = settings.value("filename2").toString();
    filename3 = settings.value("filename3").toString();
    filename4 = settings.value("filename4").toString();
    filename5 = settings.value("filename5").toString();
    filename6 = settings.value("filename6").toString();
    filename7 = settings.value("filename7").toString();
    filename8 = settings.value("filename8").toString();
    filename9 = settings.value("filename9").toString();
    filename10 = settings.value("filename10").toString();
    filename11= settings.value("filename11").toString();
    filename12 = settings.value("filename12").toString();
    filename13 = settings.value("filename13").toString();
    filename14 = settings.value("filename14").toString();
    filename15 = settings.value("filename15").toString();
    filename16 = settings.value("filename16").toString();
    filename17 = settings.value("filename17").toString();
    filename18 = settings.value("filename18").toString();
    filename19 = settings.value("filename19").toString();
    filename20 = settings.value("filename20").toString();
    pic1 = settings.value("pic1").toString();
    pic2 = settings.value("pic2").toString();
    pic3 = settings.value("pic3").toString();
    pic4 = settings.value("pic4").toString();
    pic5 = settings.value("pic5").toString();
    pic6 = settings.value("pic6").toString();
    pic7 = settings.value("pic7").toString();
    pic8 = settings.value("pic8").toString();
    pic9 = settings.value("pic9").toString();
    pic10 = settings.value("pic10").toString();
    pic11 = settings.value("pic11").toString();
    pic12 = settings.value("pic12").toString();
    pic13 = settings.value("pic13").toString();
    pic14 = settings.value("pic14").toString();
    pic15 = settings.value("pic15").toString();
    pic16 = settings.value("pic16").toString();
    pic17 = settings.value("pic17").toString();
    pic18 = settings.value("pic18").toString();
    pic19 = settings.value("pic19").toString();
    pic20 = settings.value("pic20").toString();

    settings.endGroup();

    ui->slot1->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot2->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot3->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot4->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot5->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot6->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot7->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot8->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot9->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot10->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot11->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot12->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot13->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot14->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot15->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot16->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot17->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot18->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot19->setIcon(QIcon(":/app/img/app/add.png"));
    ui->slot20->setIcon(QIcon(":/app/img/app/add.png"));

    if(!pic1.isEmpty())
    {
        QIcon icon(pic1);

        ui->slot1->setIcon(icon);

    }

    if(!pic2.isEmpty())
    {
        QIcon icon(pic2);

        

        ui->slot2->setIcon(icon);

        
    }
    if(!pic3.isEmpty())
    {
        QIcon icon(pic3);

        

        ui->slot3->setIcon(icon);

       
    }

    if(!pic4.isEmpty())
    {
        QIcon icon(pic4);

    

        ui->slot4->setIcon(icon);

        
    }

    if(!pic5.isEmpty())
    {
        QIcon icon(pic5);

        

        ui->slot5->setIcon(icon);

    }

    if(!pic6.isEmpty())
    {
        QIcon icon(pic6);

      

        ui->slot6->setIcon(icon);

    }

    if(!pic7.isEmpty())
    {
        QIcon icon(pic7);


        ui->slot7->setIcon(icon);
    
    }

    if(!pic8.isEmpty())
    {
        QIcon icon(pic8);

        ui->slot8->setIcon(icon);

    }

    if(!pic9.isEmpty())
    {
        QIcon icon(pic9);

      

        ui->slot9->setIcon(icon);

    }

    if(!pic10.isEmpty())
    {
        QIcon icon(pic10);

     

        ui->slot10->setIcon(icon);

    }

    if(!pic11.isEmpty())
    {
        QIcon icon(pic11);


        ui->slot11->setIcon(icon);

    }

    if(!pic12.isEmpty())
    {
        QIcon icon(pic12);

      

        ui->slot12->setIcon(icon);


    }

    if(!pic14.isEmpty())
    {
        QIcon icon(pic14);

        ui->slot14->setIcon(icon);

    }

    if(!pic15.isEmpty())
    {
        QIcon icon(pic15);

        ui->slot15->setIcon(icon);

    }

    if(!pic16.isEmpty())
    {
        QIcon icon(pic16);


        ui->slot16->setIcon(icon);

  
    }

    if(!pic17.isEmpty())
    {
        QIcon icon(pic17);


        ui->slot17->setIcon(icon);

    }


    if(!pic18.isEmpty())
    {
        QIcon icon(pic18);

        

        ui->slot18->setIcon(icon);


    }


    if(!pic19.isEmpty())
    {
        QIcon icon(pic19);

        

        ui->slot19->setIcon(icon);

    
    }


    if(!pic13.isEmpty())
    {
        QIcon icon(pic13);


        ui->slot13->setIcon(icon);

    }

    if(!pic20.isEmpty())
    {
        QIcon icon(pic20);

        

        ui->slot20->setIcon(icon);

    }

}

// Preset Save Functions
void MainWindow::on_savePreset1_clicked()
{
    connect(presetNotification, &PresetNotification::confirmed, this, [this]() {
        saveLoadout("Preset1");
        presetNotification->showSuccessNotification("Preset 1 Saved Successfully!");
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    connect(presetNotification, &PresetNotification::cancelled, this, [this]() {
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    presetNotification->showConfirmationDialog("Save Preset 1");
}

void MainWindow::on_savePreset2_clicked()
{
    connect(presetNotification, &PresetNotification::confirmed, this, [this]() {
        saveLoadout("Preset2");
        presetNotification->showSuccessNotification("Preset 2 Saved Successfully!");
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    connect(presetNotification, &PresetNotification::cancelled, this, [this]() {
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    presetNotification->showConfirmationDialog("Save Preset 2");
}

void MainWindow::on_savePreset3_clicked()
{
    connect(presetNotification, &PresetNotification::confirmed, this, [this]() {
        saveLoadout("Preset3");
        presetNotification->showSuccessNotification("Preset 3 Saved Successfully!");
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    connect(presetNotification, &PresetNotification::cancelled, this, [this]() {
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    presetNotification->showConfirmationDialog("Save Preset 3");
}

void MainWindow::on_savePreset4_clicked()
{
    connect(presetNotification, &PresetNotification::confirmed, this, [this]() {
        saveLoadout("Preset4");
        presetNotification->showSuccessNotification("Preset 4 Saved Successfully!");
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    connect(presetNotification, &PresetNotification::cancelled, this, [this]() {
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    presetNotification->showConfirmationDialog("Save Preset 4");
}

void MainWindow::on_savePreset5_clicked()
{
    connect(presetNotification, &PresetNotification::confirmed, this, [this]() {
        saveLoadout("Preset5");
        presetNotification->showSuccessNotification("Preset 5 Saved Successfully!");
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    connect(presetNotification, &PresetNotification::cancelled, this, [this]() {
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    presetNotification->showConfirmationDialog("Save Preset 5");
}

void MainWindow::on_savePreset6_clicked()
{
    connect(presetNotification, &PresetNotification::confirmed, this, [this]() {
        saveLoadout("Preset6");
        presetNotification->showSuccessNotification("Preset 6 Saved Successfully!");
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    connect(presetNotification, &PresetNotification::cancelled, this, [this]() {
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    presetNotification->showConfirmationDialog("Save Preset 6");
}

void MainWindow::on_savePreset7_clicked()
{
    connect(presetNotification, &PresetNotification::confirmed, this, [this]() {
        saveLoadout("Preset7");
        presetNotification->showSuccessNotification("Preset 7 Saved Successfully!");
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    connect(presetNotification, &PresetNotification::cancelled, this, [this]() {
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    presetNotification->showConfirmationDialog("Save Preset 7");
}

void MainWindow::on_savePreset8_clicked()
{
    connect(presetNotification, &PresetNotification::confirmed, this, [this]() {
        saveLoadout("Preset8");
        presetNotification->showSuccessNotification("Preset 8 Saved Successfully!");
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    connect(presetNotification, &PresetNotification::cancelled, this, [this]() {
        disconnect(presetNotification, &PresetNotification::confirmed, nullptr, nullptr);
        disconnect(presetNotification, &PresetNotification::cancelled, nullptr, nullptr);
    });
    
    presetNotification->showConfirmationDialog("Save Preset 8");
}

// Preset Load Functions
void MainWindow::on_loadPreset1_clicked()
{
    loadLoadout("Preset1");
    presetNotification->showSuccessNotification("Preset 1 Loaded Successfully!");
}

void MainWindow::on_loadPreset2_clicked()
{
    loadLoadout("Preset2");
    presetNotification->showSuccessNotification("Preset 2 Loaded Successfully!");
}

void MainWindow::on_loadPreset3_clicked()
{
    loadLoadout("Preset3");
    presetNotification->showSuccessNotification("Preset 3 Loaded Successfully!");
}

void MainWindow::on_loadPreset4_clicked()
{
    loadLoadout("Preset4");
    presetNotification->showSuccessNotification("Preset 4 Loaded Successfully!");
}

void MainWindow::on_loadPreset5_clicked()
{
    loadLoadout("Preset5");
    presetNotification->showSuccessNotification("Preset 5 Loaded Successfully!");
}

void MainWindow::on_loadPreset6_clicked()
{
    loadLoadout("Preset6");
    presetNotification->showSuccessNotification("Preset 6 Loaded Successfully!");
}

void MainWindow::on_loadPreset7_clicked()
{
    loadLoadout("Preset7");
    presetNotification->showSuccessNotification("Preset 7 Loaded Successfully!");
}

void MainWindow::on_loadPreset8_clicked()
{
    loadLoadout("Preset8");
    presetNotification->showSuccessNotification("Preset 8 Loaded Successfully!");
}


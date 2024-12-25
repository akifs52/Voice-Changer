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

    ui->sound1->setIcon(QIcon());
    ui->sound2->setIcon(QIcon());
    ui->sound3->setIcon(QIcon());
    ui->sound4->setIcon(QIcon());
    ui->sound5->setIcon(QIcon());
    ui->sound6->setIcon(QIcon());
    ui->sound7->setIcon(QIcon());
    ui->sound8->setIcon(QIcon());
    ui->sound9->setIcon(QIcon());
    ui->sound10->setIcon(QIcon());
    ui->sound11->setIcon(QIcon());
    ui->sound12->setIcon(QIcon());
    ui->sound13->setIcon(QIcon());
    ui->sound14->setIcon(QIcon());
    ui->sound15->setIcon(QIcon());
    ui->sound16->setIcon(QIcon());
    ui->sound17->setIcon(QIcon());
    ui->sound18->setIcon(QIcon());
    ui->sound19->setIcon(QIcon());
    ui->sound20->setIcon(QIcon());

    if(!pic1.isEmpty())
    {
        QIcon icon(pic1);

        QSize size(75,75);

        ui->sound1->setIcon(icon);

        ui->sound1->setIconSize(size);
    }

    if(!pic2.isEmpty())
    {
        QIcon icon(pic2);

        QSize size(75,75);

        ui->sound2->setIcon(icon);

        ui->sound2->setIconSize(size);
    }
    if(!pic3.isEmpty())
    {
        QIcon icon(pic3);

        QSize size(75,75);

        ui->sound3->setIcon(icon);

        ui->sound3->setIconSize(size);
    }

    if(!pic4.isEmpty())
    {
        QIcon icon(pic4);

        QSize size(75,75);

        ui->sound4->setIcon(icon);

        ui->sound4->setIconSize(size);
    }

    if(!pic5.isEmpty())
    {
        QIcon icon(pic5);

        QSize size(75,75);

        ui->sound5->setIcon(icon);

        ui->sound5->setIconSize(size);
    }

    if(!pic6.isEmpty())
    {
        QIcon icon(pic6);

        QSize size(75,75);

        ui->sound6->setIcon(icon);

        ui->sound6->setIconSize(size);
    }

    if(!pic7.isEmpty())
    {
        QIcon icon(pic7);

        QSize size(75,75);

        ui->sound7->setIcon(icon);

        ui->sound7->setIconSize(size);
    }

    if(!pic8.isEmpty())
    {
        QIcon icon(pic8);

        QSize size(75,75);

        ui->sound8->setIcon(icon);

        ui->sound8->setIconSize(size);
    }

    if(!pic9.isEmpty())
    {
        QIcon icon(pic9);

        QSize size(75,75);

        ui->sound9->setIcon(icon);

        ui->sound9->setIconSize(size);
    }

    if(!pic10.isEmpty())
    {
        QIcon icon(pic10);

        QSize size(75,75);

        ui->sound10->setIcon(icon);

        ui->sound10->setIconSize(size);
    }

    if(!pic11.isEmpty())
    {
        QIcon icon(pic11);

        QSize size(75,75);

        ui->sound11->setIcon(icon);

        ui->sound11->setIconSize(size);
    }

    if(!pic12.isEmpty())
    {
        QIcon icon(pic12);

        QSize size(75,75);

        ui->sound12->setIcon(icon);

        ui->sound12->setIconSize(size);
    }

    if(!pic14.isEmpty())
    {
        QIcon icon(pic14);

        QSize size(75,75);

        ui->sound14->setIcon(icon);

        ui->sound14->setIconSize(size);
    }

    if(!pic15.isEmpty())
    {
        QIcon icon(pic15);

        QSize size(75,75);

        ui->sound15->setIcon(icon);

        ui->sound15->setIconSize(size);
    }

    if(!pic16.isEmpty())
    {
        QIcon icon(pic16);

        QSize size(75,75);

        ui->sound16->setIcon(icon);

        ui->sound16->setIconSize(size);
    }

    if(!pic17.isEmpty())
    {
        QIcon icon(pic17);

        QSize size(75,75);

        ui->sound17->setIcon(icon);

        ui->sound17->setIconSize(size);
    }


    if(!pic18.isEmpty())
    {
        QIcon icon(pic18);

        QSize size(75,75);

        ui->sound18->setIcon(icon);

        ui->sound18->setIconSize(size);
    }


    if(!pic19.isEmpty())
    {
        QIcon icon(pic19);

        QSize size(75,75);

        ui->sound19->setIcon(icon);

        ui->sound19->setIconSize(size);
    }


    if(!pic13.isEmpty())
    {
        QIcon icon(pic13);

        QSize size(75,75);

        ui->sound13->setIcon(icon);

        ui->sound13->setIconSize(size);
    }

    if(!pic20.isEmpty())
    {
        QIcon icon(pic20);

        QSize size(75,75);

        ui->sound20->setIcon(icon);

        ui->sound20->setIconSize(size);
    }

}

void MainWindow::on_load1_clicked()
{
    loadLoadout("Loadout1");
}


void MainWindow::on_load2_clicked()
{
    loadLoadout("Loadout2");
}


void MainWindow::on_load3_clicked()
{
    loadLoadout("Loadout3");
}


void MainWindow::on_load4_clicked()
{
    loadLoadout("Loadout4");
}


void MainWindow::on_load5_clicked()
{
    loadLoadout("Loadout5");
}


void MainWindow::on_save1_clicked()
{
    saveLoadout("Loadout1");
}


void MainWindow::on_save2_clicked()
{
     saveLoadout("Loadout2");
}


void MainWindow::on_savee3_clicked()
{
     saveLoadout("Loadout3");
}


void MainWindow::on_save4_clicked()
{
     saveLoadout("Loadout4");
}


void MainWindow::on_save5_clicked()
{
     saveLoadout("Loadout5");
}


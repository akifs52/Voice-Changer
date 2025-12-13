#include "mainwindow.h"

#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Yükleniyor ekranı oluştur
    QWidget *splashWidget = new QWidget(nullptr, Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    splashWidget->setFixedSize(300, 100);
    splashWidget->setStyleSheet("background-color: #2d2d2d; border-radius: 10px; border: 2px solid #555;");

    QVBoxLayout *layout = new QVBoxLayout(splashWidget);
    QLabel *label = new QLabel("Yükleniyor...", splashWidget);
    label->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    // Ekranın ortasına konumlandır
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - splashWidget->width()) / 2;
    int y = (screenGeometry.height() - splashWidget->height()) / 2;
    splashWidget->move(x, y);

    splashWidget->show();
    a.processEvents();

    MainWindow w;
    
    // Preload bitince splash'i kapat ve pencereyi göster
    QObject::connect(&w, &MainWindow::preloadFinished, splashWidget, [splashWidget, &w]() {
        splashWidget->close();
        w.show();
    });

    return a.exec();
}

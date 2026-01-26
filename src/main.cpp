#include "mainwindow.h"
#include "loading.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Create modern loading widget
    LoadingWidget loadingWidget;
    
    // Show loading screen
    loadingWidget.showLoading();
    
    // Initial progress animation
    QTimer *progressTimer = new QTimer(&loadingWidget);
    int progressValue = 0;
    
    QObject::connect(progressTimer, &QTimer::timeout, [&]() {
        progressValue += 2;
        if (progressValue > 90) progressValue = 90; // Stop at 90%, complete when app loads
        
        loadingWidget.updateProgress(progressValue, 100);
        
        // Update loading text based on progress
        if (progressValue < 30) {
            loadingWidget.updateStatus("Initializing audio system...");
        } else if (progressValue < 60) {
            loadingWidget.updateStatus("Loading voice effects...");
        } else if (progressValue < 90) {
            loadingWidget.updateStatus("Preparing interface...");
        }
    });
    
    progressTimer->start(50); // Update every 50ms

    MainWindow w;
    
    // Connect to preload progress for real-time updates
    QObject::connect(&w, &MainWindow::preloadProgress, [&](int current, int total) {
        int progress = (current * 80) / 100; // Use 80% of progress bar for preload
        loadingWidget.updateProgress(progress, 100);
        
        if (current < total / 3) {
            loadingWidget.updateStatus(QString("Loading soundpacks... %1/%2").arg(current).arg(total));
        } else if (current < (total * 2) / 3) {
            loadingWidget.updateStatus(QString("Processing audio files... %1/%2").arg(current).arg(total));
        } else {
            loadingWidget.updateStatus(QString("Finalizing... %1/%2").arg(current).arg(total));
        }
    });
    
    // Preload bitince splash'i kapat ve pencereyi göster
    QObject::connect(&w, &MainWindow::preloadFinished, [&]() {
        progressTimer->stop();
        
        // Complete progress to 100%
        loadingWidget.updateProgress(100, 100);
        loadingWidget.updateStatus("Ready!");
        
        // Hide loading screen and show main window
        QTimer::singleShot(500, [&]() {
            loadingWidget.hideLoading();
            w.show();
        });
    });

    return a.exec();
}

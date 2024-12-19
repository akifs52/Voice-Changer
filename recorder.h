#ifndef RECORDER_H
#define RECORDER_H

#include <QMainWindow>
#include "mainwindow.h"

class recorder : public QMainWindow
{
    Q_OBJECT
public:
    explicit recorder(QWidget *parent = nullptr);

signals:
};

#endif // RECORDER_H


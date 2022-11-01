#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "workthread.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(int argc, char** argv, QWidget *parent = 0);
    ~MainWindow();
    
//    const static int NEXT = 1;
//    const static int PREVIOUS = -1;
    
private:
    Ui::MainWindow *ui;
    WorkThread *workThread;

signals:
    void killWorkThread();
    void textToWorkThread(QString, int);
    
public slots:
    void updateImageA(QImage frame);
    void updateEventLabel(QString text);
    void updatePlusButton(int frameSpread);
    void previousButtonPressed();
    void nextButtonPressed();

};

#endif // MAINWINDOW_H

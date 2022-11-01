#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "workthread.h"

MainWindow::MainWindow(int argc, char** argv, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    workThread = new WorkThread(argc, argv, this);

    connect(workThread, SIGNAL( newFrame(QImage /* , QImage, QImage, ... */ ) ), this, SLOT( updateImageA(QImage /*,QImage,...*/) ));
    connect(workThread, SIGNAL( eventLabelChanged(QString) ), this, SLOT( updateEventLabel(QString) ));
    connect(workThread, SIGNAL( eventPlusTime(int) ), this, SLOT( updatePlusButton(int) ));
    
    connect(this, SIGNAL( killWorkThread() ), workThread, SLOT( killMe() ));

    connect(ui->previousButton, SIGNAL( clicked() ), this, SLOT( previousButtonPressed() ));
    
    connect(ui->nextButton, SIGNAL( clicked() ), this, SLOT( nextButtonPressed() ));
    connect(this, SIGNAL( textToWorkThread(QString, int) ), workThread, SLOT( directionButtonClicked(QString, int) ));
    // connect(ui->nextButton, SIGNAL( clicked() ), workThread, SLOT( nextButtonClicked() ));
    // connect(ui->nextButton, SIGNAL( clicked() ), ui->lineEdit, SLOT( clear() ));
    
    connect(ui->plusButton, SIGNAL( clicked() ), workThread, SLOT( plusTime() ));
    connect(ui->pauseButton, SIGNAL( clicked() ), workThread, SLOT( togglePause() ));
    connect(ui->replayButton, SIGNAL( clicked() ), workThread, SLOT( replay() ));


    // run
    workThread->start();
}

MainWindow::~MainWindow() {
    // zabij WT
    emit killWorkThread();
    if (!workThread->wait(1000)) workThread->terminate();
    delete workThread;

    delete ui;
}


void MainWindow::updateImageA(QImage frame /* QImage frame2, ... */) {
    ui->imageArea->setPixmap(QPixmap::fromImage(frame));
//  ...
}

void MainWindow::previousButtonPressed() {
  emit textToWorkThread(ui->lineEdit->text(), -1);
  ui->lineEdit->clear();
}

void MainWindow::nextButtonPressed() {
  emit textToWorkThread(ui->lineEdit->text(), 1);
  ui->lineEdit->clear();
}

void MainWindow::updateEventLabel(QString text) {
  ui->eventLabel->setText(text);
}

void MainWindow::updatePlusButton(int frameSpread = NULL) {;
  if (frameSpread) {
    ui->plusButton->setText(QString("+ %1").arg(frameSpread));
  }
}

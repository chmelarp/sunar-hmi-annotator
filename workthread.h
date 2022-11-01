#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H

// Qt header files
#include <QThread>
#include <QtGui>

// OpenCV header files
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

// MatToQImage
#include "cvwidget.h"

// VTApi
#include <include/vtapi.h>



using namespace cv;

class WorkThread : public QThread
{
    Q_OBJECT
public:
    explicit WorkThread(int argc, char** argv, QObject *parent = 0);
    ~WorkThread();

protected:
    void run();     // this is the (only) working function

private:
    VTApi* vtapi;
    Dataset* dataset;
    Sequence* sequence;
    Interval* annotation;

    VideoCapture videoCapture;
    // ...
    Mat mat;
    QImage image;

    bool previousClicked;
    bool nextClicked;
    QString text;

    bool plusClicked;
    bool replayClicked;

    QTimer timer;
    bool timerTick;

    bool doom;  // the doom makes the exit clean
    
    int spreadShot(int t1, int t2);
    
signals:
    void newFrame(const QImage &frame);
    void eventLabelChanged(QString text);
    void eventPlusTime(int frameSpread);
    
public slots:
    void directionButtonClicked(const QString& str, const int direction);
    void timerUpdate();
    void killMe();
    void togglePause();
    void plusTime();
    void replay();
    
};

#endif // PROCESSINGTHREAD_H

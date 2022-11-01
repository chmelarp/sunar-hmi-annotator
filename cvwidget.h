#ifndef CVWIDGET_H
#define CVWIDGET_H

// Qt header files
#include <QWidget>
#include <QPixmap>
#include <QDebug>
#include <QtGui>

// OpenCV header files
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
QImage MatToQImage(const Mat&);


class CVWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CVWidget(QWidget *parent = 0);

    void setPixmap(QPixmap _pixmap) { pixmap = _pixmap; emit pixmapChanged(); update(); }
    QPixmap getPixmap() { return pixmap; }

    void paintEvent(QPaintEvent *);

    void mousePressEvent (QMouseEvent * event);

    void wheelEvent(QWheelEvent * event);

//  void keyPressEvent(QKeyEvent *);
// QDragLeaveEvent Enter, dragMoveEvent (QWidget)
// mouseDoubleClickEvent

private:
    QPixmap pixmap;
    
signals:
    void pixmapChanged();
    
public slots:
    
};

#endif // CVWIDGET_H

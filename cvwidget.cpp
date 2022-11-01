#include "cvwidget.h"
#include <QPainter>
#include <QDebug>
#include <QtGui>

CVWidget::CVWidget(QWidget *parent) :
    QWidget(parent)
{
    // no focus to the widget to enabble key presssssssssssss
    setFocusPolicy(Qt::NoFocus);
}


void CVWidget::paintEvent(QPaintEvent *) {
    QSize s = this->size();
    // qDebug() << s;
    QPainter painter(this);

    // painter.save(); // Saves the current painter state ???

    // this makes the pixmap to fill the available space (doesn't keep the ratio)
    // painter.drawPixmap(0,0 ,s.width(), s.height(), pixmap);

    // this makes the pixmap to draw 1:1 (no zoom)
    // painter.drawPixmap(0,0 ,m_pixmap.width(), m_pixmap.height(), m_pixmap);

    // this makes the pixmap to fill the space but keep the original ratio
    // FIXME: optimize: use onResize property to change this!
    if (pixmap.width() > 0 && pixmap.height() > 0 && s.width() > 0 && s.height() > 0)
    if ((float)pixmap.width() / (float)pixmap.height() <
        (float)s.width()      / (float)s.height()) {  // s.setHeight() is OK
        s.setWidth((float)pixmap.width() * ((float)s.height() / (float)pixmap.height()));
    } else {                                             // s.setWidth() is OK
        s.setHeight((float)pixmap.height() * ((float)s.width() / (float)pixmap.width()));
    }
    // qDebug() << s << " pixmap:" << pixmap.width() << pixmap.height();
    painter.drawPixmap(0,0 ,s.width(), s.height(), pixmap);


//    if (hasFocus()) {
//        painter.setPen(QColor(0,0,128));
//        painter.drawRect(0,0, s.width()-1, s.height()-1);
//    }

    painter.end();

}


void CVWidget::mousePressEvent (QMouseEvent * event) {
    QSize s = size();
    qDebug() << "click" << event->x() << " " << event->y() << " " << s.width() << " " << s.height();

    // emit clicked (event->x(), event->y());

    // buttons ()  ->  Qt::LeftButton, Qt::RightButton, Qt::MidButton
}


void CVWidget::wheelEvent(QWheelEvent * event) {
    qDebug() << "wheeeeeeee";
}


// unnecessary
//void CVWidget::keyPressEvent(QKeyEvent *ev) {
//    qDebug() << "keyboard" << ev->key();
//}




/***********************************************************************
 * qt-opencv-multithreaded:
 * A multithreaded OpenCV application using the Qt framework.
 *
 * MatToQImage.cpp
 *
 * Nick D'Ademo <nickdademo@gmail.com>
 *
 * Copyright (c) 2012 Nick D'Ademo
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ***********************************************************************/
QImage MatToQImage(const Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS=1
    if(mat.type()==CV_8UC1)
    {
        // Set the color table (used to translate colour indexes to qRgb values)
        QVector<QRgb> colorTable;
        for (int i=0; i<256; i++)
            colorTable.push_back(qRgb(i,i,i));
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        img.setColorTable(colorTable);
        return img;
    }
    // 8-bits unsigned, NO. OF CHANNELS=3
    if(mat.type()==CV_8UC3)
    {
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return img.rgbSwapped();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
} // MatToQImage()

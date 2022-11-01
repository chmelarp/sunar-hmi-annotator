/*
 * File:   main.cpp
 * Author: chmelarp
 *
 * Created on September 26, 2012, 3:00 PM
 */

#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    // initialize resources, if needed
    // Q_INIT_RESOURCE(resfile);

    QApplication app(argc, argv);

    // create and show your widgets here
    MainWindow w(argc, argv);
    w.show();

    return app.exec();
}

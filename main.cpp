/***
 * RadioViz - main.cpp
 * Author: Matthew Ribbins, 2015
 * Description: Main file for RadioViz application
 *
 */

#include <QApplication>
#include <QtWidgets>
#include <opencv2/opencv.hpp>
#include <assert.h>

#include "mainwindow.h"

int main(int argc, char **argv) {


    QApplication app(argc, argv);

    // Open up the main window
    MainWindow *window = new MainWindow();
    window->show();

    int retval = app.exec();
     
    return retval;
}


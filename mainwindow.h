#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>
#include <QDesktopWidget>
#include <opencv2/opencv.hpp>
#include "camerawidget.h"
#include "radioviz.h"

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

private:
    CameraWidget *cameraWidget;
    cv::VideoCapture camera[MAX_CAMERAS_AVAILABLE];
    int currentCamera;
    int availableCameras;

protected:
    void timerEvent(QTimerEvent*);
    int countAvailableCameras(void);
    void refreshCameraImage(void);

public slots:
    void changeCamera(void);

};

#endif // MAINWINDOW_H

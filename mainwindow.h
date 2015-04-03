#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>
#include <QDesktopWidget>
#include <QDir>

#include <opencv2/opencv.hpp>
#include <portaudiocpp/PortAudioCpp.hxx>

#include "camerawidget.h"
#include "camera.h"
#include "radioviz.h"

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

private:
    CameraWidget *cameraWidget;
    Camera *camera[MAX_CAMERAS_AVAILABLE];
    int currentCamera;
    int availableCameras;
    QLabel *debugLabel;

protected:
    void timerEvent(QTimerEvent*);
    int CountAvailableCameras(void);
    void RefreshCameraImage(void);
    void PrintSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
    double GetHighestAudioSampleRate(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
    void DebugUpdateAudioLevel(int currentDeviceNum);
    int GetAudioLevelFromDevice(int devNum);
    void SelectCameraBasedOnAudio();

public slots:
    void ChangeCamera(void);
    void ChangeCamera(int cameraToChange);


};

#endif // MAINWINDOW_H

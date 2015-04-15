#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>
#include <QDesktopWidget>
#include <QDir>
#include <QSettings>
#include <QThread>
#include <QKeyEvent>

#include <opencv2/opencv.hpp>
#include <portaudiocpp/PortAudioCpp.hxx>

extern "C" {
#include <libavdevice/avdevice.h>
}

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
    int timerCount;
    QThread audioThread;
    QLabel *debugLabel;
    int mode;

protected:
    void timerEvent(QTimerEvent *);
    void keyPressEvent(QKeyEvent *);
    int CountAvailableCameras(void);
    int GetAvailableCamerasList(AVDeviceInfoList **deviceList);
    void RefreshCameraImage(void);
    void PrintSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
    double GetHighestAudioSampleRate(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
    int GetAudioLevelFromDevice(int devNum);
    void SelectCameraBasedOnAudio();
    void SelectCameraBasedOnVideo();
    void SelectCameraBasedOnAudioVideo();

    void SelectCameraBasedOnInput(int input);

public slots:
    void ChangeCamera(void);
    void ChangeCamera(int cameraToChange);


};

#endif // MAINWINDOW_H

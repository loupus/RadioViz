/***
 * RadioViz - mainwindow.cpp
 * Author: Matthew Ribbins, 2015
 * Description: Main window for RadioViz application, displays camera feed on screen/window
 *
 */

#include "mainwindow.h"
//#include "ui_mainwindow.h"
#include "camera.h"
#include <QMainWindow>

/***
 * Main Window Constructor
 * Author: Matthew Ribbins
 */
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    PaError err;
    //int result;
    QSettings settings("settings.ini", QSettings::IniFormat, parent);

    // Set up window. Full screen
    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->screenGeometry(0));

    currentCamera = 0;
    availableCameras = 0;

    cameraWidget = new CameraWidget(this);
    cameraWidget->resize(this->width(), this->height());

    QVBoxLayout *layout = new QVBoxLayout;
    debugLabel = new QLabel;
    QPushButton *button = new QPushButton("Change Camera");

    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(debugLabel);
    layout->addWidget(cameraWidget);
    layout->addWidget(button);

    setLayout(layout);
    setWindowTitle("Camera");
    setStyleSheet("background-color: black;");
    setWindowState(Qt::WindowFullScreen);

    // Portaudio Test


    // Get available camera devices by using FFMpeg
    av_register_all();
    avdevice_register_all();
    avcodec_register_all();



    AVDeviceInfoList* deviceList;
    GetAvailableCamerasList(&deviceList);
    availableCameras = deviceList->nb_devices;

    err = Pa_Initialize();
    if(err != paNoError)
        qDebug() << "Error: PortAudio did not initialise";

    int numAudioDevices = Pa_GetDeviceCount();
    if(numAudioDevices < 0)
       qDebug() << "Error: PortAudio did not find any audio devices";

    qDebug() << "Number of available devices:"  << numAudioDevices - PORTAUDIO_TO_CAMERA_DEVICE_OFFSET << "\n";
    debugLabel->setText(QString("Number of available audio devices: %1").arg(numAudioDevices));

    for(int i = 0; i < numAudioDevices; i++)
    {
        const PaDeviceInfo *deviceInfo;
        deviceInfo = Pa_GetDeviceInfo(i);
        qDebug() << "Device " << i << ": " << deviceInfo->name;
    }

    // Initialise Cameras
    for(int i=0; i < availableCameras; i++) {
        camera[i] = new Camera(i,i);
    }

    // Settings
    for(int i=0; i < availableCameras; i++) {
        settings.setValue(QString("Devices/").append(deviceList->devices[i]->device_name).append("/name"),
                          QString(deviceList->devices[i]->device_description));
    }

    connect(button, SIGNAL(pressed()), this, SLOT(ChangeCamera()));

    startTimer(50);  // 50 = 30fps
 }

/***
 * Main Window Deconstructor
 * Author: Matthew Ribbins
 */
MainWindow::~MainWindow()
{

}



void MainWindow::SelectCameraBasedOnAudio()
{
    int levels[availableCameras];
    QString *debugString = new QString();


    for(int i = 0; i < availableCameras; i++) {
        levels[i] = camera[i]->GetAudioLevelFromDevice();
        debugString->append(QString("%1").arg(levels[i]));
        if(i == currentCamera)
            debugString->append(QString("*"));
        debugString->append(QString(" "));

    }
    debugLabel->setText(*debugString);

    // Logic
    for(int i = 0; i < availableCameras; i++) {
        if(i != currentCamera) {
            if( (levels[i] > CAMERA_AUDIO_THRESHOLD) && (levels[i] > (levels[currentCamera])) ) {
                qDebug() << "Changing to camera " << i;
                ChangeCamera(i);
            }
        }
    }
}

#if 0
void MainWindow::DebugUpdateAudioLevel(int currentDeviceNum)
{
    PaError err;
    QString *debugString = new QString();
    float tempBuffer[FRAMES_PER_BUFFER];
    int volume;

    for(int mic = 0; mic < availableCameras; mic++) {


        Pa_StartStream(cameraMic[mic]);

        err = Pa_ReadStream(cameraMic[mic], tempBuffer, FRAMES_PER_BUFFER);
        if(err == paNoError) {
            // Get a dB

            float sum = 0;
            for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
                sum += pow(tempBuffer[i], 2);
            }
            volume = 20 * log10(sqrt(sum / FRAMES_PER_BUFFER));
            debugString->append(QString("%1").arg(volume));
            if(mic == currentDeviceNum)
                debugString->append(QString("*"));

        } else
            qDebug() << "Error: " << Pa_GetErrorText(err);

        Pa_StopStream(cameraMic[mic]);
        debugString->append(QString(" "));
    }

    //qDebug(debugString->toLatin1());
    debugLabel->setText(*debugString);
}
#endif
/***
 * Timer Event Handler
 * Author: Matthew Ribbins
 * Description: Every time the timer handler is called, we need to refresh the current image on screen
 */
void MainWindow::timerEvent(QTimerEvent*)
{
    //DebugUpdateAudioLevel(currentCamera);
    SelectCameraBasedOnAudio();
    RefreshCameraImage();

}

/***
 * Refresh Camera Image
 * Author: Matthew Ribbins
 * Description: Acquire the image from the current camera using OpenCV. We then display the image using CameraWidget
 */
void MainWindow::RefreshCameraImage(void)
{
    QPixmap frame = camera[currentCamera]->GetVideoFrame();
    cameraWidget->putFrame(frame);
}

/***
 * Count Number of camera devicess
 * Author: Matthew Ribbins
 * Description: Count the number of available cameras attached.
 *
 * Return: (int) Number of cameras available
 */

int MainWindow::CountAvailableCameras(void)
{
    AVDeviceInfoList *deviceList;
    AVInputFormat *iformat = av_find_input_format("video4linux2");
    avdevice_list_input_sources(iformat, NULL, NULL, &deviceList);
    return deviceList->nb_devices;
}

/***
 * Get list of camera devices
 * Author: Matthew Ribbins
 * Description: Get list of camera devices.
 *
 * Return: (AVDeviceList) Number of cameras available
 */

int MainWindow::GetAvailableCamerasList(AVDeviceInfoList** deviceList)
{
    AVInputFormat *iformat = av_find_input_format("video4linux2");
    return avdevice_list_input_sources(iformat, NULL, NULL, deviceList);
}

/***
 * Count Number of OpenCV Cameras
 * Author: Matthew Ribbins
 * Description: Count the number of available cameras attached. OpenCV does not do this natively, so we will poll all
 *              the cameras available until we either hit MAX_CAMERAS_AVAILABLE or hit a camera that doesn't exist.
 *
 * Return: (int) Number of cameras available
 */
#if 0

int MainWindow::CountAvailableCamerasOpenCV(void)
{
    int fileCount = 0
    QDir dir(QString("/sys/class/video4linux/"));
    fileCount = dir.count() - 2;
    qDebug() << "OpenCV detects >=" << fileCount << " cameras.";
    return fileCount;
}
#endif


#if 0
int MainWindow::countAvailableCameras(void) {
    cv::VideoCapture tempCamera;
    bool isOpen = false;
    for (int i = 0; i < MAX_CAMERAS_AVAILABLE; i++) {
        // Attempt to open up camera, get the result
        tempCamera.open(i);
        isOpen = (tempCamera.isOpened());
        tempCamera.release();

        // If camera does not exist, return number of cameras detected
        if (!isOpen) {
            qDebug() << "OpenCV detects " << i << " cameras.";
            return i;
        }
        isOpen = false;
    }
    // There are >=MAX_CAMERAS_AVAILABLE
    qDebug() << "OpenCV detects >=" << MAX_CAMERAS_AVAILABLE << " cameras.";
    return MAX_CAMERAS_AVAILABLE;
}
#endif

/***
 * Change Camera Feed
 * Author: Matthew Ribbins
 * Description: Switch camera feed
 */
void MainWindow::ChangeCamera(void)
{
    // Turn off the current camera to save USB bandwidth
    //camera[currentCamera].release();

    // If no camera number provided, switch to the next available camera
    (currentCamera+1 >= availableCameras) ? currentCamera = 0 : currentCamera++;

    // Open the new camera
    //camera[currentCamera].open(currentCamera);
    //camera[currentCamera].set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    //camera[currentCamera].set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    //camera[currentCamera].set(CV_CAP_PROP_FPS, 30);

    // Refresh image on screen
    MainWindow::RefreshCameraImage();
}

void MainWindow::ChangeCamera(int cameraToChange)
{
    // Turn off the current camera to save USB bandwidth
    //camera[currentCamera].release();

    // If no camera number provided, switch to the next available camera
    currentCamera = cameraToChange;

    // Open the new camera
    //camera[currentCamera].open(currentCamera);
    //camera[currentCamera].set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    //camera[currentCamera].set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    //camera[currentCamera].set(CV_CAP_PROP_FPS, 30);

    // Refresh image on screen
    MainWindow::RefreshCameraImage();
}


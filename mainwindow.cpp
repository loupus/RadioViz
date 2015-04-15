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
    timerCount = 0;
    mode = 0;

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

    // Get available camera devices by using FFMpeg
    av_register_all();
    avdevice_register_all();
    avcodec_register_all();

    AVDeviceInfoList* deviceList;
    GetAvailableCamerasList(&deviceList);
    availableCameras = deviceList->nb_devices;

    // Initialise PortAudio
    err = Pa_Initialize();
    if(err != paNoError)
        qDebug() << "Error: PortAudio did not initialise";

    int numAudioDevices = Pa_GetDeviceCount();
    if(numAudioDevices < 0)
       qDebug() << "Error: PortAudio did not find any audio devices";

    qDebug() << "Number of available devices:"  << numAudioDevices - PORTAUDIO_TO_CAMERA_DEVICE_OFFSET << "\n";

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
        //For some reason there's a reversal
        int ffmpegi = availableCameras - i - 1;
        QString settingKey(QString("Devices/").append(deviceList->devices[ffmpegi]->device_description));

        int result = QString::compare(settings.value(QString(settingKey).append("/enabled")).toString(), "true", Qt::CaseInsensitive);

        if(!result) {
            // We have settings to load
            qDebug() << "Load " << settingKey << "to Camera " << ffmpegi;
            float gain = (settings.value(QString(settingKey).append("/gain"))).toFloat();
            qDebug() << "Gain " << gain;
            camera[i]->SetAudioGain(gain);

        } else {
            // We have a new device to save settings about
            settings.setValue(QString(settingKey).append("/enabled"), true);

            // Set current settings
            settings.setValue(QString(settingKey).append("/gain"), camera[i]->GetAudioGain());
        }

    }

    mode = settings.value(QString("mode")).toInt();
    if(!mode) {
        // Initialise Mode
        settings.setValue(QString("mode"), "1");
        mode = 1;
    }

    connect(button, SIGNAL(pressed()), this, SLOT(ChangeCamera()));
    for(int i = 0; i < availableCameras; i++) {
        camera[i]->FlushBuffers();
    }

    startTimer(40); // 30fps
 }

/***
 * Main Window Deconstructor
 * Author: Matthew Ribbins
 */
MainWindow::~MainWindow()
{
    for(int i = 0; i < availableCameras; i++) {
        delete camera[i];
    }
}

void MainWindow::SelectCameraBasedOnInput(int input)
{
    if(input > availableCameras) return;
    ChangeCamera(--input);
}

/***
 * Select suitable camera based on audio
 * Author: Matthew Ribbins
 */
void MainWindow::SelectCameraBasedOnAudio()
{
    float levels[availableCameras];
    bool active[availableCameras];
    int activeCount = 0;
    int loudestCamera = 0;
    float loudestCameraLevel = -50;
    QString *debugString = new QString();

    memset((void *)&active, 0, sizeof(bool)*availableCameras);

    // Get current values
    for(int i = 0; i < availableCameras; i++) {
        levels[i] = camera[i]->GetAudioLevelFromDevice();
        debugString->append(QString("%1").arg(levels[i]));
        qDebug() << "Camera " << i << ": " << levels[i];

        if(i == currentCamera)
            debugString->append(QString("*"));
        debugString->append(QString(" "));

        if(levels[i] > loudestCameraLevel) {
            loudestCamera = i;
            loudestCameraLevel = levels[i];
        }

        if(levels[i] > CAMERA_AUDIO_THRESHOLD) {
            active[i] = true;
            activeCount++;
        }
    }
    debugLabel->setText(*debugString);

    if(activeCount) {
        if(activeCount > 2) {
            qDebug() << "Two or more cameras loud.";
            ChangeCamera(loudestCamera);
        } else {
            ChangeCamera(loudestCamera);
        }
    }
}

/***
 * Select suitable camera based on video
 * Author: Matthew Ribbins
 */
void MainWindow::SelectCameraBasedOnVideo()
{
    int movement[availableCameras];
    bool active[availableCameras];
    int numOfActive = 0;
    int highestActive = -1;
    int highestActiveLevel = 0;

    memset((void *)&active, 0, sizeof(bool)*availableCameras);

    for(int i = 0; i < availableCameras; i++) {
        camera[i]->GetVideoFrame();
        movement[i] = camera[i]->GetMovementDetection();
        qDebug() << "Camera " << i << ": " << movement[i];
        if(movement[i] > CAMERA_MOVEMENT_THRESHOLD) {
            numOfActive++;
            active[i] = true;
            if(movement[i] > highestActiveLevel) {
                highestActive = i;
                highestActiveLevel = movement[i];
            }
        }
    }
    if(numOfActive) {
        if(numOfActive > 2) {
            qDebug() << ">2 cameras with movement above threshold";
        } else {
            qDebug() << "Camera " << highestActive << " active";
            ChangeCamera(highestActive);
            //for(int i = 0; i < 2; i++) {
            //    cameraWidget->putFrame(camera[highestActive]->GetProcessedFrame(i));
            //    cameraWidget->repaint();
            //}
        }
    }
    qDebug() << "----";
}

/***
 * Select suitable camera based on audio and video
 * Author: Matthew Ribbins
 */
void MainWindow::SelectCameraBasedOnAudioVideo()
{
    float levels[availableCameras];
    bool activeAudio[availableCameras];
    int activeCount = 0;
    int loudestCamera = 0;
    float loudestCameraLevel = -50;

    int movement[availableCameras];
    bool activeVideo[availableCameras];
    int numOfActive = 0;
    int highestActive = -1;
    int highestActiveLevel = 0;
    QString *debugString = new QString();

    memset((void *)&activeAudio, 0, sizeof(bool)*availableCameras);
    memset((void *)&activeVideo, 0, sizeof(bool)*availableCameras);


    // Get current values
    for(int i = 0; i < availableCameras; i++) {
        levels[i] = camera[i]->GetAudioLevelFromDevice();
        movement[i] = camera[i]->GetMovementDetection();
        debugString->append(QString("%1").arg(levels[i]));
        qDebug() << "Camera " << i << ": " << levels[i];

        if(i == currentCamera)
            debugString->append(QString("*"));
        debugString->append(QString(" "));

        if(levels[i] > loudestCameraLevel) {
            loudestCamera = i;
            loudestCameraLevel = levels[i];
        }

        if(levels[i] > CAMERA_AUDIO_THRESHOLD) {
            activeAudio[i] = true;
            activeCount++;
        }
    }
    debugLabel->setText(*debugString);

    if(activeCount) {
        if(activeCount > 2) {
            qDebug() << "Two or more cameras loud.";
            ChangeCamera(loudestCamera);
        } else {
            ChangeCamera(loudestCamera);
        }
    }
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
 * Parameters
 * - AVDeviceInfoList** deviceList (out): List of available camera devices
 * Return: int: AVError
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
    qDebug() << "Chaning from " << currentCamera << " to " << cameraToChange;
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


/***
 * Timer Event Handler
 * Author: Matthew Ribbins
 * Description: Every time the timer handler is called, we need to refresh the current image on screen
 */
void MainWindow::timerEvent(QTimerEvent*)
{
    RefreshCameraImage();
    timerCount++;

    switch(mode) {
        case MODE_DISABLED:
            break;
        case MODE_AUTO_AUDIO:
            if(10 <= timerCount) {
                SelectCameraBasedOnAudio();
                timerCount = 0;
            }
            break;
        case MODE_AUTO_MOVEMENT:
            if(floor(timerCount/3)) {
                SelectCameraBasedOnVideo();
            }
            break;
        case MODE_AUTO_MULTI:
            // Not implemented
            break;
        case MODE_MANUAL:
            break;
    }

}

/***
 * Key Press Event Handler
 * Author: Matthew Ribbins
 * Description: Look out for numeric keypad presses, or hotkeys to change settings
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    switch(key) {
        case Qt::Key_0:
            mode = MODE_AUTO_AUDIO; break;
        case Qt::Key_M:
            mode = MODE_AUTO_MOVEMENT; break;
        case Qt::Key_N:
            mode = MODE_AUTO_AUDIO; break;
        case Qt::Key_B:
            mode = MODE_AUTO_MULTI; break;
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            mode = MODE_MANUAL;
            SelectCameraBasedOnInput(key - 0x30);
            break;
    }
}


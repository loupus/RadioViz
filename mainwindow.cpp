/***
 * RadioViz - mainwindow.cpp
 * Author: Matthew Ribbins, 2015
 * Description: Main window for RadioViz application, displays camera feed on screen/window
 *
 */

#include "mainwindow.h"
//#include "ui_mainwindow.h"
#include <QMainWindow>

/***
 * Main Window Constructor
 * Author: Matthew Ribbins
 */
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    currentCamera = 0;
    availableCameras = 0;

    availableCameras = countAvailableCameras();
    for(int i=0; i < availableCameras; i++) {
        cv::VideoCapture initCamera(i);
        camera[i] = initCamera;
        camera[i].set(CV_CAP_PROP_FRAME_WIDTH, 1280);
        camera[i].set(CV_CAP_PROP_FRAME_HEIGHT, 720);
        camera[i].set(CV_CAP_PROP_FPS, 30);
    }

    // Set up window. Full screen
    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->screenGeometry(0));

    cameraWidget = new CameraWidget(this);
    cameraWidget->resize(this->width(), this->height());

    QVBoxLayout *layout = new QVBoxLayout;
    QPushButton *button = new QPushButton("Change Camera");

    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(cameraWidget);
    layout->addWidget(button);

    setLayout(layout);
    setWindowTitle("Camera");
    setStyleSheet("background-color: black;");
    setWindowState(Qt::WindowFullScreen);




    connect(button, SIGNAL(pressed()), this, SLOT(changeCamera()));

    startTimer(50);  // 50 = 30fps
 }

/***
 * Main Window Deconstructor
 * Author: Matthew Ribbins
 */
MainWindow::~MainWindow()
{

}

/***
 * Timer Event Handler
 * Author: Matthew Ribbins
 * Description: Every time the timer handler is called, we need to refresh the current image on screen
 */
void MainWindow::timerEvent(QTimerEvent*)
{
    refreshCameraImage();
}

/***
 * Refresh Camera Image
 * Author: Matthew Ribbins
 * Description: Acquire the image from the current camera using OpenCV. We then display the image using CameraWidget
 */
void MainWindow::refreshCameraImage(void)
{
    cv::Mat image;
    camera[currentCamera] >> image;
    if(!image.empty())
        cameraWidget->putFrame(image);
}

/***
 * Refresh Camera Image
 * Author: Matthew Ribbins
 * Description: Count the number of available cameras attached. OpenCV does not do this natively, so we will poll all
 *              the cameras available until we either hit MAX_CAMERAS_AVAILABLE or hit a camera that doesn't exist.
 *
 * Return: (int) Number of cameras available
 */
int MainWindow::countAvailableCameras(void)
{
    cv::VideoCapture tempCamera;

    for (int i = 0; i < MAX_CAMERAS_AVAILABLE; i++) {
        // Attempt to open up camera, get the result
        cv::VideoCapture tempCamera(i);
        bool res = (tempCamera.isOpened());
        tempCamera.release();

        // If camera does not exist, return number of cameras detected
        if (!res)
            return i;
    }
    // There are >=MAX_CAMERAS_AVAILABLE
    return MAX_CAMERAS_AVAILABLE;
}

/***
 * Change Camera Feed
 * Author: Matthew Ribbins
 * Description: Switch camera feed
 */
void MainWindow::changeCamera(void)
{
    // Turn off the current camera to save USB bandwidth
    camera[currentCamera].release();

    // If no camera number provided, switch to the next available camera
    (currentCamera+1 >= availableCameras) ? currentCamera = 0 : currentCamera++;

    // Open the new camera
    camera[currentCamera].open(currentCamera);
    camera[currentCamera].set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    camera[currentCamera].set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    camera[currentCamera].set(CV_CAP_PROP_FPS, 30);

    // Refresh image on screen
    MainWindow::refreshCameraImage();
}

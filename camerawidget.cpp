/***
 * RadioViz - camerawidget.cpp
 * Author: Matthew Ribbins, 2015
 * Description: Camera widget to display camera video feed in a QLabel image
 *
 */
#include "camerawidget.h"

/***
 * Main Window Constructor
 * Author: Matthew Ribbins
 */
CameraWidget::CameraWidget(QWidget *parent)
    : QWidget(parent)
{
    cameraLayout = new QVBoxLayout;
    cameraLabel = new QLabel;

    cameraLayout->setSpacing(0);
    cameraLayout->setMargin(0);
    cameraLayout->addWidget(cameraLabel);

    setLayout(cameraLayout);

    windowWidth = parent->width();
    windowHeight = parent->height();
    cameraLabel->setAlignment(Qt::AlignHCenter);
}

/***
 * Main Window Deconstructor
 * Author: Matthew Ribbins
 */
CameraWidget::~CameraWidget(void)
{

}

/***
 * Update Frame on display
 * Author: Matthew Ribbins
 */
void CameraWidget::putFrame(cv::Mat image)
{
    QPixmap convertedImage = matToPixmap(image).scaled(windowWidth, windowHeight, Qt::KeepAspectRatio);
    cameraLabel->setPixmap(convertedImage);

}

/***
 * Convert Mat to QPixmap
 * Author: Matthew Ribbins
 * Description: Convert OpenCV Mat image format to QImage format and then to QPixmap for display on screen
 */
QPixmap CameraWidget::matToPixmap(cv::Mat matImage) {
    cv::Mat tempMat;

    // Colour correction
    cv::cvtColor(matImage, tempMat, CV_BGR2RGB);

    // Convert to QImage
    // QImage(uchar *data, int width, int height, Format format);
    QImage tempImage((uchar*)tempMat.data, tempMat.cols, tempMat.rows, QImage::Format_RGB888);

    // Return as QPixmap
    return QPixmap::fromImage(tempImage);
}

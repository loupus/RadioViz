#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QPixmap>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QImage>
#include <QDebug>
#include <opencv/cv.h>

class CameraWidget : public QWidget
{

public:
    CameraWidget(QWidget *parent = 0);
    ~CameraWidget(void);
    QPixmap matToPixmap(cv::Mat);
    void putFrame(cv::Mat);

private:
    QLabel *cameraLabel;
    QVBoxLayout *cameraLayout;
};

#endif // CAMERAWIDGET_H

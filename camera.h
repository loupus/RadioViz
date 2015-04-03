#ifndef CAMERA_H
#define CAMERA_H


#include <opencv2/opencv.hpp>
#include <portaudiocpp/PortAudioCpp.hxx>

#include "radioviz.h"


class Camera
{
public:
    Camera();
    Camera(int cameraId, int audioId);
    ~Camera();
    QPixmap GetVideoFrame(void);
    int GetAudioLevelFromDevice(void);



private:
    cv::VideoCapture video;
    PaStream *audio;
    bool isActive;
    Camera *parentCamera;

protected:
    bool IsVideoValid(void);
    void InitialiseVideo(int cameraId);
    void InitialiseAudio(int audioId);
    double GetHighestAudioSampleRate(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
    void PrintSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
    QPixmap MatToPixmap(cv::Mat matImage);

};

#endif // CAMERA_H

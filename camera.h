#ifndef CAMERA_H
#define CAMERA_H


#include <opencv2/opencv.hpp>
#include <portaudiocpp/PortAudioCpp.hxx>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/fifo.h>
#include <libavutil/rational.h>
}


#include "radioviz.h"

typedef struct _FFmpegDevice {
    AVCodecContext *pCodecCtx;
    AVFormatContext *pFormatCtx;
    AVCodec *pCodec;
    AVInputFormat *iformat;
    AVFrame *pFrame;
    AVFrame *pFrameRGB;
    AVDeviceInfoList *pDeviceList;
    AVPixelFormat pFormat;
    int videoStream;

} FFmpegDevice;

class Camera
{
public:
    Camera();
    Camera(int cameraId, int audioId);
    ~Camera();
    QPixmap GetVideoFrame(void);
    int GetAudioLevelFromDevice(void);
    void SetAudioGain(float gain);



private:
    //cv::VideoCapture video;
    FFmpegDevice video;
    PaStream *audio;
    float audioGain;
    bool isActive;
    Camera *parentCamera;

protected:
    void DebugFFmpegError(int errno);
    bool IsVideoValid(void);
    void InitialiseVideo(int cameraId);
    void DeinitialiseVideo();
    void InitialiseAudio(int audioId);
    double GetHighestAudioSampleRate(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
    void PrintSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
    QPixmap MatToPixmap(cv::Mat matImage);
    QPixmap AVPictureToPixmap(int height, int width, void* data);

};

#endif // CAMERA_H

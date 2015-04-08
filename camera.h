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

#define CAMERA_MODE_FFMPEG 0
#define CAMERA_MODE_OPENCV 1

typedef struct _FFmpegDevice {
    AVCodecContext *pCodecCtx;
    AVFormatContext *pFormatCtx;
    AVCodec *pCodec;
    AVFrame *pFrame;
    AVFrame *pFrameRGB;
    AVDeviceInfoList *pDeviceList;
    int streamId;

} FFmpegDevice;

class Camera
{
public:
    Camera();
    Camera(int cameraId, int audioId);
    ~Camera();
    QPixmap GetVideoFrame(void);
    QPixmap GetVideoFrameOpenCV(void);
    QPixmap GetVideoFrameFFmpeg(void);
    int GetAudioLevelFromDevice(void);
    void FlushBuffers(void);

    double GetAudioGain();
    void SetAudioGain(double gain);
    int GetMovementDetection();

private:
    cv::VideoCapture cvvideo;
    FFmpegDevice video;
    int videoMode;
    PaStream *audio;
    float audioGain;
    bool isActive;
    Camera *parentCamera;
    cv::Mat storedFrames[3];

protected:
    void DebugFFmpegError(int errno);
    void InitialiseVideo(int cameraId);
    void InitialiseVideoFFmpeg(int cameraId);
    void InitialiseVideoOpenCV(int cameraId);
    void DeinitialiseVideo();
    bool IsVideoValid(void);

    void InitialiseAudio(int audioId);
    double GetHighestAudioSampleRate(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
    double GetFirstAudioSampleRate(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);
    void PrintSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters);

    void SaveStoredFrame(cv::Mat frame);

    QPixmap MatToPixmap(cv::Mat matImage);
    QPixmap AVPictureToPixmap(int height, int width, void* data);

};

#endif // CAMERA_H

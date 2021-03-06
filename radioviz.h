#ifndef RADIOVIZ_H
#define RADIOVIZ_H

#include <opencv2/opencv.hpp>
#include <portaudiocpp/PortAudioCpp.hxx>

#include "camerawidget.h"
#include "radioviz.h"

// Maximum number of cameras available for use
#define MAX_CAMERAS_AVAILABLE 5

// Maximum number of audio devices for each camera device
#define MAX_AUDIO_DEVICES_PER_CAMERA 2

// Temporary PortAudio camera offset
#define PORTAUDIO_TO_CAMERA_DEVICE_OFFSET 5

// PortAudio Defaults
#define SAMPLE_RATE  (44100)
#define PA_SAMPLE_TYPE paFloat32
#define SAMPLE_SIZE (2)
#define SAMPLE_SILENCE (0)
#define FRAMES_PER_BUFFER (1024)
#define NUM_CHANNELS    (1)
#define NUM_SECONDS     (15)
#define DITHER_FLAG     (0)

// OpenCV Defaults
#define CAMERA_DEFAULT_RES_WIDTH 960
#define CAMERA_DEFAULT_RES_HEIGHT 544
#define CAMERA_DEFAULT_FPS 15

#define MOTION_DETECTION_PIXEL_THRESHOLD 42
#define MOTION_DETECTION_PIXEL_MAX 255
#define MOTION_DETECTION_JUMP 2


#define CAMERA_AUDIO_THRESHOLD (-29)
#define CAMERA_MOVEMENT_THRESHOLD 3

// Switching Modes
#define MODE_DISABLED 0
#define MODE_AUTO_AUDIO 1
#define MODE_AUTO_MOVEMENT 2
#define MODE_AUTO_MULTI 3
#define MODE_MANUAL 4


// Classes
class VizCamera
{
public:
    VizCamera();
    ~VizCamera();
private:
    cv::VideoCapture videoObject;
    PaStream *audioObject[MAX_AUDIO_DEVICES_PER_CAMERA];
    short numberOfAudioObjects;
    bool isActive;
    VizCamera *parentCamera;
};


#endif // RADIOVIZ_H

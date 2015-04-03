/***
 * RadioViz - camera.cpp
 * Author: Matthew Ribbins, 2015
 * Description: Camera class, handle camera
 *
 */
#include "camera.h"

Camera::Camera()
{

}

Camera::Camera(int cameraId, int audioId)
{
    InitialiseVideo(cameraId);
    InitialiseAudio(audioId);
}

Camera::~Camera()
{

}


/***
 * Initialise Camera (OpenCV)
 * Author: Matthew Ribbins
 * Description:
 */
void Camera::InitialiseVideo(int cameraId)
{
    cv::VideoCapture temp(cameraId);
    video = temp;
    video.set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_DEFAULT_RES_WIDTH);
    video.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_DEFAULT_RES_HEIGHT);
    video.set(CV_CAP_PROP_FPS, CAMERA_DEFAULT_FPS);
}
void Camera::InitaliseVideoFFmpeg(int cameraId)
{

}

void Camera::InitialiseAudio(int audioId)
{
    PaStreamParameters inputParameters;
    PaError err;

    inputParameters.device = audioId + PORTAUDIO_TO_CAMERA_DEVICE_OFFSET;
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
                &audio,
                &inputParameters,
                NULL,
                GetHighestAudioSampleRate(&inputParameters, NULL),
                FRAMES_PER_BUFFER,
                paClipOff,
                NULL,
                NULL);
    if(err != paNoError)
        qDebug() << "Error: Audio Device " << audioId << "failed to open.";
}

double Camera::GetHighestAudioSampleRate(
        const PaStreamParameters *inputParameters,
        const PaStreamParameters *outputParameters)
{
    static double standardSampleRates[] = {
        8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
        44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
    };
    int i;
    double highestSampleRate = 0.0;
    PaError err;

    for( i=0; standardSampleRates[i] > 0; i++ )
    {
        qDebug() << "Device querying rate " << standardSampleRates[i];
        err = Pa_IsFormatSupported( inputParameters, outputParameters, standardSampleRates[i] );
        if( err == paFormatIsSupported )
        {
            qDebug() << ">> Supported";
            highestSampleRate = standardSampleRates[i];
        }
    }
    if( !highestSampleRate )
      qDebug() << ( "No suitable sample rate detected. Something's pear shaped here..." );

    return highestSampleRate;
}


void Camera::PrintSupportedStandardSampleRates(
        const PaStreamParameters *inputParameters,
        const PaStreamParameters *outputParameters )
{
    static double standardSampleRates[] = {
        8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
        44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
    };
    int     i, printCount;
    PaError err;

    printCount = 0;
    for( i=0; standardSampleRates[i] > 0; i++ )
    {
        err = Pa_IsFormatSupported( inputParameters, outputParameters, standardSampleRates[i] );
        if( err == paFormatIsSupported )
        {
            if( printCount == 0 )
            {
                qDebug() << ( "\t%8.2f", standardSampleRates[i] );
                printCount = 1;
            }
            else if( printCount == 4 )
            {
                qDebug() << ( ",\n\t%8.2f", standardSampleRates[i] );
                printCount = 1;
            }
            else
            {
                qDebug() << ( ", %8.2f", standardSampleRates[i] );
                ++printCount;
         }
        }
    }
    if( !printCount )
      qDebug() << ( "None\n" );
}


/***
 * Convert Mat to QPixmap
 * Author: Matthew Ribbins
 * Description: Convert OpenCV Mat image format to QImage format and then to QPixmap for display on screen
 */
QPixmap Camera::MatToPixmap(cv::Mat matImage) {
    cv::Mat tempMat;

    // Colour correction
    cv::cvtColor(matImage, tempMat, CV_BGR2RGB);

    // Convert to QImage
    // QImage(uchar *data, int width, int height, Format format);
    QImage tempImage((uchar*)tempMat.data, tempMat.cols, tempMat.rows, QImage::Format_RGB888);

    // Return as QPixmap
    return QPixmap::fromImage(tempImage);
}



/***
 * Get Audio Level from Device
 * Author: Matthew Ribbins
 */
int Camera::GetAudioLevelFromDevice()
{
    PaError err;
    QString *debugString = new QString();
    float tempBuffer[FRAMES_PER_BUFFER];
    int volume;

    Pa_StartStream(audio);

    err = Pa_ReadStream(audio, tempBuffer, FRAMES_PER_BUFFER);
    if(err == paNoError) {
        // Get a dB
        float sum = 0;
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            sum += pow(tempBuffer[i], 2);
        }
        volume = 20 * log10(sqrt(sum / FRAMES_PER_BUFFER));
        debugString->append(QString("%1").arg(volume));

    } else {
     qDebug() << "Error: " << Pa_GetErrorText(err);
    }

    Pa_StopStream(audio);

    return volume;
}

/***
 * Get Frame
 * Author: Matthew Ribbins
 */
QPixmap Camera::GetVideoFrame(void)
{
    cv::Mat frame;
    QPixmap convertedFrame;

    // Get the frame
    video >> frame;
    if(!frame.empty())
        convertedFrame = MatToPixmap(frame);
    return convertedFrame;
}

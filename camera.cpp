/***
 * RadioViz - camera.cpp
 * Author: Matthew Ribbins, 2015
 * Description: Camera class, handle camera
 *
 */
#include "camera.h"

Camera::Camera()
{
    DeinitialiseVideo();
}

Camera::Camera(int cameraId, int audioId)
{
    InitialiseAudio(audioId);
    InitialiseVideo(cameraId);
}

Camera::~Camera()
{

}

void Camera::DebugFFmpegError(int errno)
{
    char msg[AV_ERROR_MAX_STRING_SIZE];
    av_make_error_string(&msg[0], AV_ERROR_MAX_STRING_SIZE, errno);
    qDebug() << "Error:" << msg;
}

/***
 * Initialise Camera (OpenCV)
 * Author: Matthew Ribbins
 * Description:
 */
void Camera::InitialiseVideo(int cameraId)
{
    char filenameSrc[12];
    int result = 0;
    int numBytes;
    uint8_t *buffer;

    av_register_all();
    avdevice_register_all();
    avcodec_register_all();

    video.pFormatCtx = avformat_alloc_context();
    video.pFormatCtx->video_codec_id = AV_CODEC_ID_MJPEG;
    video.pFormatCtx->iformat = av_find_input_format("video4linux2");
    //video.pFormatCtx->iformat->codec_tag = (const AVCodecTag* const*) avcodec_find_decoder(AV_CODEC_ID_MJPEG);
    //video.pDeviceList;// = (AVDeviceInfoList *) malloc(sizeof(AVDeviceInfoList));
    video.videoStream = 0;


    sprintf(filenameSrc, "/dev/video%d", cameraId);
    sprintf(video.pFormatCtx->filename, "/dev/video%d", cameraId);
    qDebug() << "Initialising " << filenameSrc << ".";

    //result = avdevice_list_devices(video.pFormatCtx, &video.pDeviceList);
    result = avdevice_list_input_sources(video.pFormatCtx->iformat, NULL, NULL, &video.pDeviceList);


    if(avformat_open_input(&video.pFormatCtx, filenameSrc, NULL, NULL) != 0)
        return;

    if(avformat_find_stream_info(video.pFormatCtx, NULL) < 0)
        return;
    av_dump_format(video.pFormatCtx, 0, filenameSrc, 0);

    for(unsigned int i=0; i < video.pFormatCtx->nb_streams; i++) {
        if(video.pFormatCtx->streams[i]->codec->coder_type==AVMEDIA_TYPE_VIDEO) {
            video.videoStream = i;
            break;
        }
    }
    if(video.videoStream == -1)
        return; // Videostream not found
    video.pCodecCtx = video.pFormatCtx->streams[video.videoStream]->codec;


    if(av_find_best_stream(video.pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &video.pCodec, 0) < 0)
        return;

    //video.pCodec = avcodec_find_decoder(video.pCodecCtx->codec_id);
    //if(video.pCodec == NULL)
        //return; // Codec not found

    //avcodec_set_dimensions(video.pCodecCtx, 1280, 720);


    if(avcodec_open2(video.pCodecCtx, video.pCodec, NULL) < 0)
        return; // Unable to open

    video.pFrame = av_frame_alloc();
    video.pFrameRGB = av_frame_alloc();

    //video.pFormat = AV_PIX_FMT_BGR24; //AV_PIX_FMT_YUV422P; //
    video.pCodecCtx->pix_fmt = AV_PIX_FMT_RGB24;
    numBytes = avpicture_get_size(video.pCodecCtx->pix_fmt, video.pCodecCtx->width, video.pCodecCtx->height);
    buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) video.pFrameRGB, buffer, video.pCodecCtx->pix_fmt, video.pCodecCtx->width, video.pCodecCtx->height);

}

#if 0
{
    cv::VideoCapture temp(cameraId);
    video = temp;
    video.set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_DEFAULT_RES_WIDTH);
    video.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_DEFAULT_RES_HEIGHT);
    video.set(CV_CAP_PROP_FPS, CAMERA_DEFAULT_FPS);
}
#endif

void Camera::DeinitialiseVideo()
{
    //close(f_desw);
    avcodec_close(video.pCodecCtx);
    av_free(video.pFrame);
    av_free(video.pFrameRGB);

    avformat_close_input(&video.pFormatCtx);
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
        8000.0, 16000.0, 44100.0, 48000.0, -1 /* negative terminated  list */
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
    cv::Mat tempMat = matImage;

    // Colour correction
    cv::cvtColor(matImage, tempMat, CV_BGR2RGB);

    // Convert to QImage
    // QImage(uchar *data, int width, int height, Format format);
    QImage tempImage((uchar*)tempMat.data, tempMat.cols, tempMat.rows, QImage::Format_RGB888);

    // Return as QPixmap
    return QPixmap::fromImage(tempImage);
}

/***
 * Convert AVPicture to QPixmap
 * Author: Matthew Ribbins
 * Description: Convert FFMpeg AVPicture from a frame to Qimage format and then to QPixmap for display
 */
QPixmap Camera::AVPictureToPixmap(int height, int width, void* data)
{
    // Convert to QImage
    // QImage(uchar *data, int width, int height, Format format);
    QImage tempImage((uchar*)data, width, height, QImage::Format_RGB888);

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
    QPixmap convertedFrame;
    AVPacket packet;
    int res;
    int frameFinished;

    avcodec_flush_buffers(video.pCodecCtx);

    if((res = av_read_frame(video.pFormatCtx, &packet)) >= 0) {
        if(packet.stream_index == video.videoStream) {
            // Decode
            avcodec_decode_video2(video.pCodecCtx, video.pFrame, &frameFinished, &packet);

            // Fix deprecation errors
            switch (video.pCodecCtx->pix_fmt) {
                case AV_PIX_FMT_YUVJ420P :
                    video.pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
                    break;
                case AV_PIX_FMT_YUVJ422P  :
                    video.pCodecCtx->pix_fmt = AV_PIX_FMT_YUV422P;
                    break;
                case AV_PIX_FMT_YUVJ444P   :
                    video.pCodecCtx->pix_fmt = AV_PIX_FMT_YUV444P;
                    break;
                case AV_PIX_FMT_YUVJ440P :
                    video.pCodecCtx->pix_fmt = AV_PIX_FMT_YUV440P;
                default:
                    break;
            }

            if(frameFinished) {
                struct SwsContext *img_convert_ctx;
                img_convert_ctx = sws_getCachedContext(NULL, video.pCodecCtx->width, video.pCodecCtx->height, video.pCodecCtx->pix_fmt, video.pCodecCtx->width, video.pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
                sws_scale(img_convert_ctx, ((AVPicture*)video.pFrame)->data, ((AVPicture*)video.pFrame)->linesize, 0, video.pCodecCtx->height, ((AVPicture *)video.pFrameRGB)->data, ((AVPicture *)video.pFrameRGB)->linesize);
                convertedFrame = AVPictureToPixmap(video.pFrame->height, video.pFrame->width, video.pFrameRGB->data[0]);
                //cv::Mat frame(video.pFrame->height,video.pFrame->width,CV_8UC3,video.pFrameRGB->data[0]);
                //if(!frame.empty())
                //    convertedFrame = MatToPixmap(frame);

                av_free_packet(&packet);
                sws_freeContext(img_convert_ctx);

            }
        }
    }

    // Get the frame OpenCV
    //video >> frame;
    //if(!frame.empty())
    //    convertedFrame = MatToPixmap(frame);
    return convertedFrame;
}

/***
 * Set Audio Gain
 * Author: Matthew Ribbins
 * Description: Set gain for audio device
 */
void Camera::SetAudioGain(float gain)
{
    this->audioGain = gain;
}

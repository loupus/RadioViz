#if 0
// Audio detection QThread
class AudioWorker : public QObject {
    Q_OBJECT

public:
    AudioWorker(char *cameraArr, int availableCamera);
    ~AudioWorker();

public slots:
    void process();

signals:
    void finished();
    void error(QString err);

private:
    Camera *camera[MAX_CAMERAS_AVAILABLE];
    int availableCameras;
    int changeCamera;
};

/***
 * AudioWorker Thread
 */
AudioWorker::AudioWorker(char *cameraArr, int availableCameras) {
   // &this->camera = (Camera **)&cameraArr;
    *this->camera = (Camera *)&cameraArr;
    this->availableCameras = availableCameras;
}

AudioWorker::~AudioWorker() {
    // free resources
}

void AudioWorker::process() {
    int levels[availableCameras];
    bool active[availableCameras];
    int activeCount = 0;
    int loudestCamera = 0;
    float loudestCameraLevel = 0;

    memset((void *)&active, 0, sizeof(bool)*availableCameras);

    // Get current values
    for(int i = 0; i < availableCameras; i++) {
        levels[i] = camera[i]->GetAudioLevelFromDevice();

        if(levels[i] > loudestCameraLevel) {
            loudestCamera = i;
            loudestCameraLevel = levels[i];
        }

        if(levels[i] > CAMERA_AUDIO_THRESHOLD) {
            active[i] = true;
            activeCount++;
        }
    }

    if(activeCount) {
        if(activeCount > 2) {
            changeCamera = (loudestCamera);
        } else {
            changeCamera = (loudestCamera);
        }
    }
}

/****
        QThread *thread = new QThread;
        AudioWorker *audioWorker = new AudioWorker((char *)&camera, availableCameras);

        audioWorker->moveToThread(thread);
        connect(audioWorker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
        connect(thread, SIGNAL(started()), audioWorker, SLOT(process()));
        connect(audioWorker, SIGNAL(finished()), thread, SLOT(quit()));
        connect(audioWorker, SIGNAL(finished()), audioWorker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
*/
#endif



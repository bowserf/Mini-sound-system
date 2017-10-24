#ifdef AAUDIO

#include "AAudioManager.h"

static Mutex *mutex;

extern "C" void *restartStream(void *arg) {
    if (!arg) {
        return nullptr;
    }

    AAudioManager *audioManager = (AAudioManager *) arg;

    if (mutex->tryLock()) {
        audioManager->closeOutputStream();
        audioManager->createPlaybackStream();
        mutex->unlock();
    } else {
        LOGI("Mutex try lock failed");
    }

    pthread_exit(nullptr);
}

aaudio_data_callback_result_t dataCallback(
        AAudioStream *stream,
        void *userData,
        void *audioData,
        int32_t numFrames) {

    assert(userData && audioData);
    AAudioEngine *eng = reinterpret_cast<AAudioEngine *>(userData);
    assert(stream == eng->playStream);

    // Tuning the buffer size for low latency...
    int32_t underRun = AAudioStream_getXRunCount(eng->playStream);
    if (underRun > eng->underRunCount) {
        /*
         * Underrun happened since last callback:
         * try to increase the buffer size.
         */
        eng->underRunCount = underRun;

        aaudio_result_t actSize = AAudioStream_setBufferSizeInFrames(
                stream, eng->bufSizeInFrames + eng->framesPerBurst);
        if (actSize > 0) {
            eng->bufSizeInFrames = actSize;
        } else {
            LOGE("[AAudioManager] Output stream buffer tuning error: %s",
                 AAudio_convertResultToText(actSize));
        }
    }

    int32_t samplesPerFrame = eng->sampleChannels;
    AUDIO_HARDWARE_SAMPLE_TYPE *bufferDest = static_cast<AUDIO_HARDWARE_SAMPLE_TYPE *>(audioData);

    if (!eng->playAudio) {
        memset(bufferDest, 0, sizeof(AUDIO_HARDWARE_SAMPLE_TYPE) * samplesPerFrame * numFrames);
        return AAUDIO_CALLBACK_RESULT_CONTINUE;
    }

    int32_t channelCount = AAudioStream_getChannelCount(eng->playStream);
    int numberElements = numFrames * channelCount;
    size_t sizeBuffer = numberElements * sizeof(AUDIO_HARDWARE_SAMPLE_TYPE);

    if (eng->playPosition < eng->soundSystem->getTotalNumberFrames()) {
        // check if we are near of the end
        if (eng->playPosition > eng->soundSystem->getTotalNumberFrames() - numberElements) {
            sizeBuffer = (eng->soundSystem->getTotalNumberFrames() - eng->playPosition) *
                         sizeof(AUDIO_HARDWARE_SAMPLE_TYPE);
        }
        memmove(bufferDest,
                eng->soundSystem->getExtractedData() + eng->playPosition,
                sizeBuffer);
        eng->playPosition += numberElements;
    } else {
        // track is finished
        memset(bufferDest, 0, sizeBuffer);
    }

    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void errorCallback(AAudioStream *stream,
                   void *userData,
                   aaudio_result_t error) {
    AAudioManager *audioManager = reinterpret_cast<AAudioManager *>(userData);
    audioManager->errorCallback(stream, error);
}

AAudioManager::AAudioManager(SoundSystem *soundSystem) {
    mutex = new Mutex();

    memset(&engine, 0, sizeof(engine));

    engine.soundSystem = soundSystem;
    engine.sampleChannels = AUDIO_SAMPLE_CHANNELS;
#ifdef FLOAT_PLAYER
    engine.sampleFormat = AAUDIO_FORMAT_PCM_FLOAT;
#else
    engine.sampleFormat = AAUDIO_FORMAT_PCM_I16;
#endif

    createPlaybackStream();
}

AAudioManager::~AAudioManager() {
    delete (mutex);
    closeOutputStream();
}

/*
 * Create sample engine and put application into started state:
 * audio is already rendering -- rendering silent audio.
 */
bool AAudioManager::createPlaybackStream() {
    AAudioStreamBuilder *builder = createStreamBuilder();
    if (builder == nullptr) {
        LOGE("Unable to obtain an AAudioStreamBuilder object");
    }

    engine.playStream = createStream(
            builder,
            engine.sampleFormat,
            engine.sampleChannels,
            AAUDIO_SHARING_MODE_SHARED,
            AAUDIO_DIRECTION_OUTPUT,
            ::dataCallback,
            ::errorCallback,
            &engine);

    // ::errorCallback
    // Score resolution operator ::
    // https://msdn.microsoft.com/en-us/library/b451xz31(v=vs.80).aspx
    // reference of the errorCallback outside of the AAudioManager scope

    assert(engine.playStream);

    PrintAudioStreamInfo(engine.playStream);

    engine.framesPerBurst = AAudioStream_getFramesPerBurst(engine.playStream);
    AAudioStream_setBufferSizeInFrames(engine.playStream, engine.framesPerBurst);

    engine.bufSizeInFrames = engine.framesPerBurst;

    aaudio_result_t result = AAudioStream_requestStart(engine.playStream);
    if (result != AAUDIO_OK) {
        assert(result == AAUDIO_OK);
        return false;
    }

    // remove unused builder
    AAudioStreamBuilder_delete(builder);

    engine.underRunCount = AAudioStream_getXRunCount(engine.playStream);
    return !(result != AAUDIO_OK);
}

AAudioStreamBuilder *AAudioManager::createStreamBuilder() {
    AAudioStreamBuilder *builder = nullptr;
    aaudio_result_t result = AAudio_createStreamBuilder(&builder);
    if (result != AAUDIO_OK && !builder) {
        LOGE("Error creating stream builder: %s", AAudio_convertResultToText(result));
    }
    return builder;
}

AAudioStream *AAudioManager::createStream(
        AAudioStreamBuilder *builder,
        aaudio_format_t format,
        int32_t sampleChannels,
        aaudio_sharing_mode_t sharing,
        aaudio_direction_t dir,
        AAudioStream_dataCallback callback,
        AAudioStream_errorCallback errorCallback,
        void *userData) {

    AAudioStreamBuilder_setFormat(builder, format);
    AAudioStreamBuilder_setChannelCount(builder, sampleChannels);

    AAudioStreamBuilder_setSharingMode(builder, sharing);
    AAudioStreamBuilder_setDirection(builder, dir);
    AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);

    AAudioStreamBuilder_setDataCallback(builder, callback, userData);
    AAudioStreamBuilder_setErrorCallback(builder, errorCallback, userData);

    AAudioStream *stream;
    aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &stream);
    if (result != AAUDIO_OK) {
        LOGE("Error opening stream: %s", AAudio_convertResultToText(result));
        assert(false);
        stream = nullptr;
    }
    return stream;
}

bool AAudioManager::start() {
    if (!engine.playStream) {
        return false;
    }

    engine.playAudio = true;
    return true;
}

bool AAudioManager::stop() {
    if (!engine.playStream) {
        return true;
    }

    engine.playAudio = false;
    return true;
}

void AAudioManager::errorCallback(AAudioStream *stream, aaudio_result_t error) {
    assert(stream == engine.playStream);
    LOGD("errorCallback result: %s", AAudio_convertResultToText(error));

    aaudio_stream_state_t streamState = AAudioStream_getState(engine.playStream);
    if (streamState == AAUDIO_STREAM_STATE_DISCONNECTED) {
        // We must handle stream restart on a separate thread
        pthread_t threadId;
        pthread_create(&threadId, nullptr, restartStream, this);
    }
}

void AAudioManager::closeOutputStream() {
    if (engine.playStream != nullptr) {
        aaudio_result_t result = AAudioStream_requestStop(engine.playStream);
        if (result != AAUDIO_OK) {
            LOGE("Error stopping output stream. %s", AAudio_convertResultToText(result));
            assert(false);
        }

        result = AAudioStream_close(engine.playStream);
        if (result != AAUDIO_OK) {
            LOGE("Error closing output stream. %s", AAudio_convertResultToText(result));
            assert(false);
        }

        engine.playStream = nullptr;
    }
}

#endif

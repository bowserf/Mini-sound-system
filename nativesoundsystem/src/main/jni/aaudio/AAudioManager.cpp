#ifdef AAUDIO

#include "AAudioManager.h"

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

    if (eng->playAudio) {
        // there is currently a bug in AAudio where currentState always equal to
        // AAUDIO_STREAM_STATE_STARTING instead of AAUDIO_STREAM_STATE_STARTED so we can't apply
        // this check for now.
        //
        //aaudio_stream_state_t aaudioStreamState = AAudioStream_getState(eng->playStream);
        //LOGV("[AAudioManager] currentState=%d %s", aaudioStreamState,
        //     AAudio_convertStreamStateToText(aaudioStreamState));
        //if (aaudioStreamState == AAUDIO_STREAM_STATE_STARTING) {
        //    memset(static_cast<AUDIO_HARDWARE_SAMPLE_TYPE *>(audioData), 0,
        //           sizeof(AUDIO_HARDWARE_SAMPLE_TYPE) * samplesPerFrame * numFrames);
        //    return AAUDIO_CALLBACK_RESULT_CONTINUE;
        //}

        int32_t channelCount = AAudioStream_getSamplesPerFrame(eng->playStream);
        int numberElements = numFrames * channelCount;
        size_t sizeBuffer = numberElements * sizeof(AUDIO_HARDWARE_SAMPLE_TYPE);

        if (eng->playAudio && eng->playPosition < eng->soundSystem->getTotalNumberFrames()) {
            // End of track
            if (eng->playPosition > eng->soundSystem->getTotalNumberFrames() - numberElements) {
                sizeBuffer = (eng->soundSystem->getTotalNumberFrames() - eng->playPosition) *
                             sizeof(AUDIO_HARDWARE_SAMPLE_TYPE);
            }
            memmove(bufferDest, eng->soundSystem->getExtractedData() + eng->playPosition,
                    sizeBuffer);
            eng->playPosition += numberElements;
        } else {
            memset(bufferDest, 0, sizeBuffer);
        }
    } else {
        memset(static_cast<AUDIO_HARDWARE_SAMPLE_TYPE *>(audioData), 0,
               sizeof(AUDIO_HARDWARE_SAMPLE_TYPE) * samplesPerFrame * numFrames);
    }

    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

/*
 * Create sample engine and put application into started state:
 * audio is already rendering -- rendering silent audio.
 */
bool AAudioManager::createEngine(SoundSystem *soundSystem) {
    memset(&engine, 0, sizeof(engine));

    engine.soundSystem = soundSystem;
    engine.sampleChannels = AUDIO_SAMPLE_CHANNELS;
#ifdef FLOAT_PLAYER
    engine.sampleFormat_ = AAUDIO_FORMAT_PCM_FLOAT;
#else
    engine.sampleFormat = AAUDIO_FORMAT_PCM_I16;
#endif

    StreamBuilder builder;
    engine.playStream = builder.CreateStream(
            engine.sampleFormat,
            engine.sampleChannels,
            AAUDIO_SHARING_MODE_SHARED,
            AAUDIO_PERFORMANCE_MODE_LOW_LATENCY,
            AAUDIO_DIRECTION_OUTPUT,
            INVALID_AUDIO_PARAM,
            dataCallback,
            &engine);

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
    engine.underRunCount = AAudioStream_getXRunCount(engine.playStream);
    return !(result != AAUDIO_OK);
}

/*
 * start():
 *   start to render sine wave audio.
 */
bool AAudioManager::start() {
    if (!engine.playStream) {
        return false;
    }

    engine.playAudio = true;
    return true;
}

/*
 * stop():
 *   stop rendering sine wave audio ( resume rendering silent audio )
 */
bool AAudioManager::stop() {
    if (!engine.playStream) {
        return true;
    }

    engine.playAudio = false;
    return true;
}

/*
 * delete()
 *   clean-up sample: application is going away. Simply setup stop request
 *   flag and rendering thread will see it and perform clean-up
 */
void AAudioManager::deleteEngine() {
    if (!engine.playStream) {
        return;
    }
}

#endif

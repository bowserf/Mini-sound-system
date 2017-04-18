/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef AAUDIO

#include "AAudioManager.h"

/*
 * PlayAudioThreadProc()
 *   Rendering audio frames continuously; if user asks to play audio, render
 *   sine wave; if user asks to stop, renders silent audio (all 0s)
 *
 */
void* PlayAudioThreadProc(void *ctx) {
    AAudioEngine *eng = reinterpret_cast<AAudioEngine *>(ctx);

    bool status = TunePlayerForLowLatency(engine.playStream_);
    if (!status) {
        // if tune up is failed, audio could still play
        LOGW("Failed to tune up the audio buffer size, low latency audio may not be guaranteed");
    }
    // double check the tuning result: not necessary
    PrintAudioStreamInfo(engine.playStream_);

    LOGV("=====: currentState=%d", AAudioStream_getState(eng->playStream_));

    int32_t framesPerBurst = AAudioStream_getFramesPerBurst(eng->playStream_);
    int32_t channelCount = AAudioStream_getSamplesPerFrame(eng->playStream_);

    LOGI("channel count %d", channelCount);
    LOGI("framesPerBurst %d", framesPerBurst);

    int numberElements = framesPerBurst * channelCount;
    int sizeBuffer = numberElements * sizeof(AUDIO_HARDWARE_SAMPLE_TYPE);

    AUDIO_HARDWARE_SAMPLE_TYPE *buf = new AUDIO_HARDWARE_SAMPLE_TYPE[sizeBuffer];
    assert(buf);

    aaudio_result_t result;

    while (!eng->requestStop_) {
        if (eng->playAudio_ && eng->playPosition_ < eng->soundSystem_->getTotalNumberFrames()) {
            if (eng->playPosition_ > eng->soundSystem_->getTotalNumberFrames() - numberElements) {
                sizeBuffer = (eng->soundSystem_->getTotalNumberFrames() - eng->playPosition_) * sizeof(AUDIO_HARDWARE_SAMPLE_TYPE);
            }
            memmove(buf, eng->soundSystem_->getExtractedData() + eng->playPosition_,
                    sizeBuffer);
            eng->playPosition_ += numberElements;
        } else {
            memset(buf, 0, sizeBuffer);
        }
        result = AAudioStream_write(eng->playStream_,
                                    buf,
                                    framesPerBurst,
                                    100000000);
        assert(result > 0);
    }

    delete[] buf;
    eng->requestStop_ = false;

    AAudioStream_requestStop(eng->playStream_);

    AAudioStream_close(eng->playStream_);
    eng->playStream_ = nullptr;

    LOGV("====Player is done");
}

/*
 * Create sample engine and put application into started state:
 * audio is already rendering -- rendering silent audio.
 */
bool AAudioManager::createEngine(SoundSystem *soundSystem) {

    memset(&engine, 0, sizeof(engine));

    engine.soundSystem_ = soundSystem;
    engine.sampleChannels_ = AUDIO_SAMPLE_CHANNELS;
#ifdef FLOAT_PLAYER
    engine.sampleFormat_ = AAUDIO_FORMAT_PCM_FLOAT;
#else
    engine.sampleFormat_ = AAUDIO_FORMAT_PCM_I16;
#endif
    engine.bitsPerSample_ = SampleFormatToBpp(engine.sampleFormat_);

    StreamBuilder builder(engine.sampleChannels_,
                          engine.sampleFormat_,
                          AAUDIO_SHARING_MODE_SHARED,
                          AAUDIO_DIRECTION_OUTPUT);

    engine.playStream_ = builder.Stream();
    assert(engine.playStream_);

    PrintAudioStreamInfo(engine.playStream_);

    engine.sampleRate_ = AAudioStream_getSampleRate(engine.playStream_);
    aaudio_result_t result = AAudioStream_requestStart(engine.playStream_);
    if (result != AAUDIO_OK) {
        assert(result == AAUDIO_OK);
        return false;
    }

    /*int64_t nanosPerWakeup = AAUDIO_NANOS_PER_SECOND * burstsPerWakeup * framesPerBurst / _sampleRate;*/
    int64_t nanosPerWakeup = 100;
    result = AAudioStream_createThread(engine.playStream_,
                                       nanosPerWakeup,
                                       PlayAudioThreadProc,
                                       &engine);
    return !(result != AAUDIO_OK);
}

/*
 * start():
 *   start to render sine wave audio.
 */
bool AAudioManager::start() {
    if (!engine.playStream_) {
        return false;
    }

    engine.playAudio_ = true;
    return true;
}

/*
 * stop():
 *   stop rendering sine wave audio ( resume rendering silent audio )
 */
bool AAudioManager::stop() {
    if (!engine.playStream_) {
        return true;
    }

    engine.playAudio_ = false;
    return true;
}

/*
 * delete()
 *   clean-up sample: application is going away. Simply setup stop request
 *   flag and rendering thread will see it and perform clean-up
 */
void AAudioManager::deleteEngine() {
    if (!engine.playStream_) {
        return;
    }
    engine.requestStop_ = true;
}

/*
 * TunePlayerForLowLatency()
 *   start from the framesPerBurst, find out the smallest size that has no
 *   underRan for buffer between Application and AAudio
 *  If tune-up failed, we still let it continue by restoring the value
 *  upon entering the function; the failure of the tuning is notified to
 *  caller with false return value.
 * Return:
 *   true:  tune-up is completed, AAudio is at its best
 *   false: tune-up is not complete, AAudio is at its default condition
 */
bool TunePlayerForLowLatency(AAudioStream *stream) {
    aaudio_stream_state_t state = AAudioStream_getState(stream);

    if (state == AAUDIO_STREAM_STATE_STARTING) {
        aaudio_result_t result;
        aaudio_stream_state_t nextState = AAUDIO_STREAM_STATE_UNINITIALIZED;
        do {
            result = AAudioStream_waitForStateChange(stream,
                                                     state,
                                                     &nextState,
                                                     100 * 1000000);
        } while ((result == AAUDIO_OK || result == AAUDIO_ERROR_TIMEOUT)
                 && nextState == AAUDIO_STREAM_STATE_UNINITIALIZED);
        state = AAudioStream_getState(stream);
    }

    if (state != AAUDIO_STREAM_STATE_STARTED) {
        LOGE("stream(%p) is not in started state when tuning", stream);
        return false;
    }

    int32_t framesPerBurst = AAudioStream_getFramesPerBurst(stream);
    int32_t orgSize = AAudioStream_getBufferSizeInFrames(
            stream);//Query the maximum number of frames that can be filled without blocking.

    int32_t bufSize = framesPerBurst;
    int32_t bufCap = AAudioStream_getBufferCapacityInFrames(
            stream);//Query maximum buffer capacity in frames.

    uint8_t *buf = new uint8_t[bufCap * engine.bitsPerSample_ / 8];
    assert(buf);
    memset(buf, 0, bufCap * engine.bitsPerSample_ / 8);

    int32_t prevXRun = AAudioStream_getXRunCount(stream);
    int32_t prevBufSize = 0;
    bool trainingError = false;
    while (bufSize <= bufCap) {
        aaudio_result_t result = AAudioStream_setBufferSizeInFrames(stream, bufSize);
        if (result <= AAUDIO_OK) {
            trainingError = true;
            break;
        }

        // check whether we are really setting to our value
        // AAudio might already reached its optimized state
        // so we set-get-compare, then act accordingly
        bufSize = AAudioStream_getBufferSizeInFrames(stream);
        if (bufSize == prevBufSize) {
            // AAudio refuses to go up, tuning is complete
            break;
        }
        // remember the current buf size so we could continue for next round tuning up
        prevBufSize = bufSize;
        result = AAudioStream_write(stream, buf, bufCap, 1000000000);

        if (result < 0) {
            assert(result >= 0);
            trainingError = true;
            break;
        }
        int32_t curXRun = AAudioStream_getXRunCount(stream);
        if (curXRun <= prevXRun) {
            // no more errors, we are done
            break;
        }
        prevXRun = curXRun;
        bufSize += framesPerBurst;
    }

    delete[] buf;
    if (trainingError) {
        // we are playing conservative here: if anything wrong, we restore to default
        // size WHEN engine was created
        AAudioStream_setBufferSizeInFrames(stream, orgSize);
        return false;
    }
    bufSize = AAudioStream_getBufferSizeInFrames(stream);
    LOGI("TunePlayerForLowLatency Bufsize : %d", bufSize);
    return true;
}

#endif

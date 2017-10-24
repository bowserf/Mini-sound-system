#ifdef AAUDIO

#ifndef MINI_SOUND_SYSTEM_AAUDIOMANAGER_H
#define MINI_SOUND_SYSTEM_AAUDIOMANAGER_H

#include <cassert>
#include <audio/SoundSystem.h>
#include <pthread.h>
#include "AAudioCommon.h"
#include "Mutex.h"

struct AAudioEngine {
    uint16_t sampleChannels;
    aaudio_format_t sampleFormat;

    SoundSystem *soundSystem;

    AAudioStream *playStream;
    bool playAudio;

    int playPosition;

    int32_t underRunCount;
    int32_t bufSizeInFrames;
    int32_t framesPerBurst;
};
static AAudioEngine engine;

class AAudioManager {

public:

    AAudioManager(SoundSystem *soundSystem);

    ~AAudioManager();

    bool createPlaybackStream();

    bool start();

    bool stop();

    void closeOutputStream();

    void errorCallback(AAudioStream *stream, aaudio_result_t error);

private:

    AAudioStreamBuilder *createStreamBuilder();

    AAudioStream *createStream(
            AAudioStreamBuilder *builder,
            aaudio_format_t format,
            int32_t sampleChannels,
            aaudio_sharing_mode_t sharing,
            aaudio_direction_t dir,
            AAudioStream_dataCallback callback,
            AAudioStream_errorCallback errorCallback,
            void *userData);

};

#endif //MINI_SOUND_SYSTEM_AAUDIOMANAGER_H

#endif

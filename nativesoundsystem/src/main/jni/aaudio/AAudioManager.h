#ifdef AAUDIO

#ifndef MINI_SOUND_SYSTEM_AAUDIOMANAGER_H
#define MINI_SOUND_SYSTEM_AAUDIOMANAGER_H

#include <cassert>
#include <audio/SoundSystem.h>

#include "AAudioCommon.h"
#include "aaudio/headers/stream_builder.h"

/*
 * This Sample's Engine Structure
 */
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

    bool createEngine(SoundSystem *soundSystem);

    bool start();

    bool stop();

    void deleteEngine();

};


#endif //MINI_SOUND_SYSTEM_AAUDIOMANAGER_H

#endif

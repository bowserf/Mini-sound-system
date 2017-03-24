//
// Created by Frederic on 23/03/2017.
//

#ifdef AAUDIO

#ifndef MINI_SOUND_SYSTEM_AAUDIOMANAGER_H
#define MINI_SOUND_SYSTEM_AAUDIOMANAGER_H

#include <thread>
#include <cassert>
#include <audio/SoundSystem.h>

#include "AAudioCommon.h"
#include "aaudio/headers/stream_builder.h"

/*
 * This Sample's Engine Structure
 */
struct AAudioEngine {
    uint32_t     sampleRate_;
    uint32_t     framesPerBuf_;
    uint16_t     sampleChannels_;
    uint16_t     bitsPerSample_;
    aaudio_audio_format_t sampleFormat_;

    SoundSystem* soundSystem_;

    AAudioStream *playStream_;
    bool   requestStop_;
    bool   playAudio_;

    int playPosition_;

};
static AAudioEngine engine;

bool TunePlayerForLowLatency(AAudioStream* stream);

class AAudioManager{
public:

    AAudioManager(int sampleRate, int framesPerBuf);

    bool createEngine(SoundSystem* soundSystem);
    bool start();
    bool stop();
    void deleteEngine();

private:

    int _sampleRate;
    int _framesPerBuf;
};



#endif //MINI_SOUND_SYSTEM_AAUDIOMANAGER_H

#endif

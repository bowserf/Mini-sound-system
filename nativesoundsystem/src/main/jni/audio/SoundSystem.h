#ifndef TEST_SOUNDSYSTEM_SOUNDSYSTEM_H
#define TEST_SOUNDSYSTEM_SOUNDSYSTEM_H

#define SLASSERT(x) assert(x == SL_RESULT_SUCCESS)

// Include OpenSLES
#include <SLES/OpenSLES.h>

// Include OpenSL ES android extensions
#include <SLES/OpenSLES_Android.h>

// C++ header
#include <assert.h>

// Androiddebug
#include <utils/android_debug.h>

// Access to malloc, free etc...
#include <malloc.h>

// NULL
#include <cstring>

#include "listener/SoundSystemCallback.h"

static void extractionEndCallback(SLPlayItf caller, void *pContext, SLuint32 event);
static void queueExtractorCallback(SLAndroidSimpleBufferQueueItf aSoundQueue, void *aContext);
static void queuePlayerCallback(SLAndroidSimpleBufferQueueItf aSoundQueue, void *aContext);

class SoundSystem {

public:
    SoundSystem(SoundSystemCallback *callback,
                int sampleRate,
                int bufSize);

    ~SoundSystem();

    void extractMusic(SLDataLocator_URI *fileLoc);

    void extractAndPlayDirectly(void *sourceFile);

    void initAudioPlayer();

    void sendSoundBufferExtract();

    void sendSoundBufferPlay();

    void stopSoundPlayer();

    void play(bool play);

    bool isPlaying();

    void stop();

    int getPlayerState();

    void fillDataBuffer();

    void getData();

    void endTrack();

    void release();

    void releasePlayer();

    short* getExtractedDataMono();

    inline short* getExtractedData(){
        return _extractedData;
    }

    inline unsigned int getTotalNumberFrames(){
        return _totalFrames;
    }

    //------------------------
    // - Extraction methods -
    //------------------------
    void notifyExtractionEnded();

    void notifyExtractionStarted();

    void notifyPlayPause(bool play);

    void notifyStopTrack();

    void notifyEndOfTrack();

private :
    void extractMetaData();

    int _sampleRate;
    int _bufferSize;
    unsigned int _totalFrames;
    int _positionExtract;
    int _positionPlay;
    bool _needExtractInitialisation;
    SLmillisecond  _musicDuration = NULL;

    SoundSystemCallback *_soundSystemCallback = NULL;

    // engine
    SLObjectItf mEngineObj = NULL;
    SLEngineItf mEngine = NULL;

    // output
    SLObjectItf mOutPutMixObj = NULL;

    //extract
    SLObjectItf _extractPlayerObject = NULL;
    SLPlayItf _extractPlayerPlay = NULL;
    SLAndroidSimpleBufferQueueItf _extractPlayerBufferQueue = NULL;
    SLMetadataExtractionItf _extractPlayerMetadata = NULL;

    //play
    SLObjectItf _playerObject = NULL;
    SLPlayItf _playerPlay = NULL;
    SLAndroidSimpleBufferQueueItf _playerQueue = NULL;

    //buffer
    short*_soundBuffer = NULL;

    //extracted music
    short* _extractedData = NULL;
};

#endif //TEST_SOUNDSYSTEM_SOUNDSYSTEM_H

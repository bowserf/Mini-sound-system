#include "SoundSystem.h"

static void extractionEndCallback(SLPlayItf caller, void *pContext, SLuint32 event) {
    if (event & SL_PLAYEVENT_HEADATEND) {
        SoundSystem *self = static_cast<SoundSystem *>(pContext);
        self->notifyExtractionEnded();
    }
}

static void extractionAndPlayEndCallback(SLPlayItf caller, void *pContext, SLuint32 event) {
    if (event & SL_PLAYEVENT_HEADATEND) {
        SoundSystem *self = static_cast<SoundSystem *>(pContext);
        self->releasePlayer();
    }
}

static void queueExtractorCallback(SLAndroidSimpleBufferQueueItf aSoundQueue, void *aContext) {
    SoundSystem *self = static_cast<SoundSystem *>(aContext);
    self->fillDataBuffer();

    // send new buffer in the queue
    self->sendSoundBufferExtract();
}

static void queuePlayerCallback(SLAndroidSimpleBufferQueueItf aSoundQueue, void *aContext) {
    SoundSystem *self = static_cast<SoundSystem *>(aContext);
    self->getData();

    // send filled buffer in the queue
    self->sendSoundBufferPlay();
}

void SoundSystem::fillDataBuffer() {
    if (_needExtractInitialisation) {
        extractMetaData();
        _needExtractInitialisation = false;
        _data = (SLuint16 *) calloc(_totalFrames * 4, sizeof(SLuint16));
    }

    int sizeBuffer = _bufferSize * sizeof(SLuint16);
    memmove(_data + _positionExtract, _soundBuffer, sizeBuffer);
    _positionExtract += sizeBuffer;
}

void SoundSystem::getData() {
    if (_positionPlay > _totalFrames * 2 * sizeof(SLuint16)) {
        endTrack();
        return;
    }

    int sizeBuffer = _bufferSize * sizeof(SLuint16);
    memmove(_soundBuffer, _data + _positionPlay, sizeBuffer);
    _positionPlay += sizeBuffer;
}

SoundSystem::SoundSystem(SoundSystemCallback *callback,
                         int sampleRate,
                         int bufSize) :
        _soundSystemCallback(callback),
        _needExtractInitialisation(true),
        _positionExtract(0),
        _positionPlay(0),
        _totalFrames(0) {

    this->_sampleRate = sampleRate;
    this->_bufferSize = bufSize;

    /*
     * A type for standard OpenSL ES errors that all functions defined in the API return.
     * Can have some of these values :
     * SL_RESULT_SUCCESS
     * SL_RESULT_PARAMETER_INVALID
     * SL_RESULT_MEMORY_FAILURE
     * SL_RESULT_FEATURE_UNSUPPORTED
     * SL_RESULT_RESOURCE_ERROR
     */
    SLresult result;

    // engine
    const SLuint32 engineMixIIDCount = 1;
    const SLInterfaceID engineMixIIDs[] = {SL_IID_ENGINE};
    const SLboolean engineMixReqs[] = {SL_BOOLEAN_TRUE};

    /*object : an abstraction of a set of resources, assigned for a well-defined set of tasks,
     and the state of these resources. To allocate the resources the object must be Realized.*/

    // create engine
    result = slCreateEngine(&mEngineObj, 0, NULL, engineMixIIDCount, engineMixIIDs, engineMixReqs);
    SLASSERT(result);

    // 2nd parameter : True if it's asynchronous and false to be synchronous
    result = (*mEngineObj)->Realize(mEngineObj, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // interface : an abstraction of a set of related features that a certain object provides.

    // get interfaces
    result = (*mEngineObj)->GetInterface(mEngineObj, SL_IID_ENGINE, &mEngine);
    SLASSERT(result);

    // creation of objects used to send data to HW device

    // mixed output
    const SLuint32 outputMixIIDCount = 0;
    const SLInterfaceID outputMixIIDs[] = {};
    const SLboolean outputMixReqs[] = {};

    result = (*mEngine)->CreateOutputMix(mEngine, &mOutPutMixObj, outputMixIIDCount, outputMixIIDs,
                                         outputMixReqs);
    SLASSERT(result);

    result = (*mOutPutMixObj)->Realize(mOutPutMixObj, SL_BOOLEAN_FALSE);
    SLASSERT(result);
}

SoundSystem::~SoundSystem() {
    release();
}

void SoundSystem::extractMusic(SLDataLocator_URI *fileLoc) {
    SLresult result;

    SLDataFormat_MIME format_mime;
    format_mime.formatType = SL_DATAFORMAT_MIME;
    format_mime.mimeType = NULL;
    format_mime.containerType = SL_CONTAINERTYPE_UNSPECIFIED;

    SLDataSource audioSrc;
    audioSrc.pLocator = fileLoc;
    audioSrc.pFormat = &format_mime;

    //Struct representing a data locator for a buffer queue
    //We say that the data will be in memory buffer and that we have two buffers
    SLDataLocator_AndroidSimpleBufferQueue dataLocatorInput;
    dataLocatorInput.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    dataLocatorInput.numBuffers = 2;

    // format of data
    SLDataFormat_PCM dataFormat;
    dataFormat.formatType = SL_DATAFORMAT_PCM;
    dataFormat.numChannels = 2; // Stereo sound.
    dataFormat.samplesPerSec = (SLuint32) _sampleRate * 1000;
    dataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    dataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSink audioSnk = {&dataLocatorInput, &dataFormat};

    // SL_IID_ANDROIDSIMPLEBUFFERQUEUE will allow us to control the queue with buffers
    // (we have two of them) create sound player
    const SLuint32 soundPlayerIIDCount = 2;
    const SLInterfaceID soundPlayerIIDs[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                             SL_IID_METADATAEXTRACTION};
    const SLboolean soundPlayerReqs[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*mEngine)->CreateAudioPlayer(mEngine, &_extractPlayerObject, &audioSrc, &audioSnk,
                                           soundPlayerIIDCount, soundPlayerIIDs, soundPlayerReqs);
    SLASSERT(result);

    result = (*_extractPlayerObject)->Realize(_extractPlayerObject, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // get the buffer queue interface
    result = (*_extractPlayerObject)->GetInterface(_extractPlayerObject,
                                                   SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                   &_extractPlayerBufferQueue);
    SLASSERT(result);

    // get the play interface
    result = (*_extractPlayerObject)->GetInterface(_extractPlayerObject, SL_IID_PLAY,
                                                   &_extractPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the player event
    result = (*_extractPlayerPlay)->RegisterCallback(_extractPlayerPlay, extractionEndCallback,
                                                     this);
    // enables/disables notification of playback events.
    result = (*_extractPlayerPlay)->SetCallbackEventsMask(_extractPlayerPlay,
                                                          SL_PLAYEVENT_HEADATEND);
    assert(SL_RESULT_SUCCESS == result);

    result = (*_extractPlayerObject)->GetInterface(_extractPlayerObject, SL_IID_METADATAEXTRACTION,
                                                   &_extractPlayerMetadata);
    assert(SL_RESULT_SUCCESS == result);

    // register callback for queue
    result = (*_extractPlayerBufferQueue)->RegisterCallback(_extractPlayerBufferQueue,
                                                            queueExtractorCallback, this);
    SLASSERT(result);

    // allocate space for the buffer
    _soundBuffer = (SLuint16 *) calloc(_bufferSize, sizeof(SLuint16));

    // send two buffers
    sendSoundBufferExtract();
    sendSoundBufferExtract();

    // start the extraction
    result = (*_extractPlayerPlay)->SetPlayState(_extractPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);

    notifyExtractionStarted();
}

void SoundSystem::initAudioPlayer() {
    SLresult result;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    // format of data
    SLDataFormat_PCM dataFormat;
    dataFormat.formatType = SL_DATAFORMAT_PCM;
    dataFormat.numChannels = 2; // Stereo sound.
    dataFormat.samplesPerSec = (SLuint32) _sampleRate * 1000;
    dataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    dataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audioSrc;
    audioSrc.pLocator = &loc_bufq;
    audioSrc.pFormat = &dataFormat;

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, mOutPutMixObj};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    const SLInterfaceID ids[] = {SL_IID_VOLUME, SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*mEngine)->CreateAudioPlayer(mEngine, &_playerObject, &audioSrc, &audioSnk,
                                           2, ids, req);
    // note that an invalid URI is not detected here, but during prepare/prefetch on Android,
    // or possibly during Realize on other platforms
    assert(result);

    // realize the player
    result = (*_playerObject)->Realize(_playerObject, SL_BOOLEAN_FALSE);

    // get the play interface
    result = (*_playerObject)->GetInterface(_playerObject, SL_IID_PLAY, &_playerPlay);
    SLASSERT(result);

    // get the buffer queue interface
    result = (*_playerObject)->GetInterface(_playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                            &_playerQueue);
    assert(SL_RESULT_SUCCESS == result);
    // register callback for queue
    result = (*_playerQueue)->RegisterCallback(_playerQueue, queuePlayerCallback, this);
    SLASSERT(result);
}

void SoundSystem::extractAndPlayDirectly(void *sourceFile) {
    if(_playerPlay != NULL && getPlayerState() == SL_PLAYSTATE_PLAYING){
        return;
    }

    SLresult result;

    SLDataFormat_MIME format_mime;
    format_mime.formatType = SL_DATAFORMAT_MIME;
    format_mime.mimeType = NULL;
    format_mime.containerType = SL_CONTAINERTYPE_UNSPECIFIED;

    SLDataSource audioSrc;
    audioSrc.pLocator = sourceFile;
    audioSrc.pFormat = &format_mime;

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, mOutPutMixObj};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    result = (*mEngine)->CreateAudioPlayer(mEngine, &_playerObject, &audioSrc, &audioSnk,
                                           0, NULL, NULL);
    SLASSERT(result);

    // realize the player
    result = (*_playerObject)->Realize(_playerObject, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // get the play interface
    result = (*_playerObject)->GetInterface(_playerObject, SL_IID_PLAY, &_playerPlay);
    SLASSERT(result);
    // register callback on the player event
    result = (*_playerPlay)->RegisterCallback(_playerPlay, extractionAndPlayEndCallback, this);
    // enables/disables notification of playback events.
    result = (*_playerPlay)->SetCallbackEventsMask(_playerPlay, SL_PLAYEVENT_HEADATEND);
    SLASSERT(result);

    result = (*_playerPlay)->SetPlayState(_playerPlay, SL_PLAYSTATE_PLAYING);
    SLASSERT(result);
}

void SoundSystem::extractMetaData() {
    (*_extractPlayerPlay)->GetDuration(_extractPlayerPlay, &_musicDuration);
    _totalFrames = (int) (((double) _musicDuration * (double) _sampleRate / 1000.0));
}

void SoundSystem::play(bool play) {
    SLresult result;
    if (NULL != _playerPlay) {
        SLuint32 currentState;
        (*_playerPlay)->GetPlayState(_playerPlay, &currentState);
        if (play
            && (currentState == SL_PLAYSTATE_PAUSED || currentState == SL_PLAYSTATE_STOPPED)) {
            sendSoundBufferPlay();
            sendSoundBufferPlay();
            result = (*_playerPlay)->SetPlayState(_playerPlay, SL_PLAYSTATE_PLAYING);
            SLASSERT(result);

            notifyPlayPause(true);
        } else if (currentState == SL_PLAYSTATE_PLAYING) {
            result = (*_playerPlay)->SetPlayState(_playerPlay, SL_PLAYSTATE_PAUSED);
            SLASSERT(result);

            notifyPlayPause(false);
        }
        SLASSERT(result);
    }
}

void SoundSystem::stop() {
    _positionPlay = 0;
    (*_playerPlay)->SetPlayState(_playerPlay, SL_PLAYSTATE_STOPPED);
    notifyStopTrack();
};

int SoundSystem::getPlayerState() {
    SLuint32 currentState;
    (*_playerPlay)->GetPlayState(_playerPlay, &currentState);
    return currentState;
}

void SoundSystem::sendSoundBufferExtract() {
    SLuint32 result = (*_extractPlayerBufferQueue)->Enqueue(_extractPlayerBufferQueue,
                                                            _soundBuffer,
                                                            sizeof(SLuint16) * _bufferSize);
    assert(result == SL_RESULT_SUCCESS);
}

void SoundSystem::sendSoundBufferPlay() {
    assert(_soundBuffer != NULL);
    SLuint32 result = (*_playerQueue)->Enqueue(_playerQueue, _soundBuffer,
                                               sizeof(SLuint16) * _bufferSize);
    assert(result == SL_RESULT_SUCCESS);
}

void SoundSystem::notifyExtractionEnded() {
    _soundSystemCallback->notifyExtractionCompleted();
}

void SoundSystem::notifyStopTrack() {
    _soundSystemCallback->notifyStopTrack();
}

void SoundSystem::notifyEndOfTrack() {
    _soundSystemCallback->notifyEndOfTrack();
}

void SoundSystem::notifyExtractionStarted() {
    _soundSystemCallback->notifyExtractionStarted();
}

void SoundSystem::notifyPlayPause(bool play) {
    _soundSystemCallback->notifyPlayPause(play);
}

void SoundSystem::endTrack() {
    _positionPlay = 0;
    (*_playerPlay)->SetPlayState(_playerPlay, SL_PLAYSTATE_STOPPED);
    notifyEndOfTrack();
}

void SoundSystem::release() {
    // destroy sound player
    stopSoundPlayer();

    if (mOutPutMixObj != NULL) {
        (*mOutPutMixObj)->Destroy(mOutPutMixObj);
        mOutPutMixObj = NULL;
    }

    if (mEngineObj != NULL) {
        (*mEngineObj)->Destroy(mEngineObj);
        mEngineObj = NULL;
        mEngine = NULL;
    }
}

void SoundSystem::stopSoundPlayer() {
    if (_playerObject != NULL) {
        SLuint32 soundPlayerState;
        (*_playerObject)->GetState(_playerObject, &soundPlayerState);
        if (soundPlayerState == SL_OBJECT_STATE_REALIZED) {
            if (_extractPlayerBufferQueue != NULL) {
                (*_extractPlayerBufferQueue)->Clear(_extractPlayerBufferQueue);
                _extractPlayerBufferQueue = NULL;
            }

            if (_playerQueue != NULL) {
                (*_playerQueue)->Clear(_playerQueue);
                _playerQueue = NULL;
            }

            if (_extractPlayerObject != NULL) {
                (*_extractPlayerObject)->AbortAsyncOperation(_extractPlayerObject);
                (*_extractPlayerObject)->Destroy(_extractPlayerObject);
                _extractPlayerObject = NULL;
                _extractPlayerPlay = NULL;
            }

            releasePlayer();

            if(_soundBuffer != NULL){
                free(_soundBuffer);
                _soundBuffer = NULL;
            }
        }
    }
}

void SoundSystem::releasePlayer() {
    if (_playerObject != NULL) {
        (*_playerObject)->AbortAsyncOperation(_playerObject);
        (*_playerObject)->Destroy(_playerObject);
        _playerObject = NULL;
        _playerPlay = NULL;
    }
}


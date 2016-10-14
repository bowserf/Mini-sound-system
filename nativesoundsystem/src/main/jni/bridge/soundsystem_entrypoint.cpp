#include "soundsystem_entrypoint.h"

void Java_fr_bowserf_soundsystem_SoundSystem_native_1init_1soundsystem(JNIEnv *env,
                                                               jclass jclass1,
                                                               jint sample_rate,
                                                               jint frames_per_buf) {
    _soundSystemCallback = new SoundSystemCallback(env, jclass1);
    LOGI("sound system callback init");
    _soundSystem = new SoundSystem(_soundSystemCallback, sample_rate, frames_per_buf);
}

void Java_fr_bowserf_soundsystem_SoundSystem_native_1load_1file(JNIEnv *env,
                                                               jclass jclass1,
                                                               jstring filePath) {
    if(!checkSoundSystemInit()){
        return;
    }
    _soundSystem->extractMusic(dataLocatorFromURLString(env, filePath));
    _soundSystem->initAudioPlayer();
}

void Java_fr_bowserf_soundsystem_SoundSystem_native_1play(JNIEnv *env, jclass jclass1, jboolean play){
    if(!checkSoundSystemInit()){
        return;
    }
    _soundSystem->play(play);
}

void Java_fr_bowserf_soundsystem_SoundSystem_native_1stop(JNIEnv *env, jclass jclass1) {
    if(!checkSoundSystemInit()){
        return;
    }
    _soundSystem->stop();
}

void Java_fr_bowserf_soundsystem_SoundSystem_native_1extract_1and_1play(JNIEnv *env, jobject obj, jstring filePath){
    if(!checkSoundSystemInit()){
        return;
    }
    _soundSystem->extractAndPlayDirectly(dataLocatorFromURLString(env, filePath));
}

void Java_fr_bowserf_soundsystem_SoundSystem_native_1extract_1from_1assets_1and_1play(JNIEnv *env, jobject obj, jobject assetManager, jstring filename){
    if(!checkSoundSystemInit()){
        return;
    }
    SLDataLocator_AndroidFD locator = getTrackFromAsset(env, assetManager, filename);
    _soundSystem->extractAndPlayDirectly(&locator);
}

void Java_fr_bowserf_soundsystem_SoundSystem_native_1release_1soundsystem(JNIEnv *env, jclass jclass1) {
    if (_soundSystem != nullptr) {
        delete _soundSystem;
        _soundSystem = nullptr;
    }
}

SLDataLocator_AndroidFD getTrackFromAsset(JNIEnv *env, jobject assetManager, jstring filename){
    // convert Java string to UTF-8
    const char *utf8 = env->GetStringUTFChars(filename, NULL);
    assert(NULL != utf8);

    // use asset manager to open asset by filename
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    assert(NULL != mgr);
    AAsset* asset = AAssetManager_open(mgr, utf8, AASSET_MODE_UNKNOWN);

    // release the Java string and UTF-8
    env->ReleaseStringUTFChars(filename, utf8);

    // open asset as file descriptor
    off_t start, length;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
    assert(0 <= fd);
    AAsset_close(asset);

    // configure audio source
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
    return loc_fd;
}

bool checkSoundSystemInit(){
    if(_soundSystem == NULL){
        LOGE("_soundSystem is not initialize");
        return false;
    }
    return true;
}

// Convert Java string to UTF-8
SLDataLocator_URI *dataLocatorFromURLString(JNIEnv *env, jstring fileURLString) {
    const char *urf8FileURLString = env->GetStringUTFChars(fileURLString, NULL);
    assert(NULL != urf8FileURLString);
    SLDataLocator_URI *fileLoc = (SLDataLocator_URI *) malloc(sizeof(SLDataLocator_URI));
    fileLoc->locatorType = SL_DATALOCATOR_URI;
    fileLoc->URI = (SLchar *) urf8FileURLString;
    return fileLoc;
}
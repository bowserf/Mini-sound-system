#ifndef TEST_SOUNDSYSTEM_SOUNDSYSTEM_ENTRYPOINT_H
#define TEST_SOUNDSYSTEM_SOUNDSYSTEM_ENTRYPOINT_H

// JNI
#include <jni.h>

// access to NULL
#include <cstring>

// android
#include <utils/android_debug.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

// throw exception
#include <iostream>
#include <stdexcept>

#include <assert.h>

#include "audio/SoundSystem.h"

#include "listener/SoundSystemCallback.h"

static SoundSystem *_soundSystem;

static SoundSystemCallback *_soundSystemCallback;

extern "C" {
    void Java_fr_bowserf_soundsystem_SoundSystem_native_1init_1soundsystem(JNIEnv *env,
                                                                           jclass jclass1,
                                                                           jint sample_rate,
                                                                           jint frames_per_buf);

    void Java_fr_bowserf_soundsystem_SoundSystem_native_1load_1file(JNIEnv *env,
                                                            jclass jclass1,
                                                            jstring filePath);

    void Java_fr_bowserf_soundsystem_SoundSystem_native_1play(JNIEnv *env, jclass jclass1, jboolean play);

    jboolean Java_fr_bowserf_soundsystem_SoundSystem_native_1is_1playing(JNIEnv *env, jclass jclass1);

    jboolean Java_fr_bowserf_soundsystem_SoundSystem_native_1is_1soundsystem_1init(JNIEnv *env, jclass jclass1);

    void Java_fr_bowserf_soundsystem_SoundSystem_native_1stop(JNIEnv *env, jclass jclass1);

    void Java_fr_bowserf_soundsystem_SoundSystem_native_1extract_1and_1play(JNIEnv *env, jobject obj, jstring filePath);

    void Java_fr_bowserf_soundsystem_SoundSystem_native_1release_1soundsystem(JNIEnv *env, jclass jclass1);

    void Java_fr_bowserf_soundsystem_SoundSystem_native_1extract_1from_1assets_1and_1play(JNIEnv *env, jobject obj, jobject assetManager, jstring filename);

    jshortArray Java_fr_bowserf_soundsystem_SoundSystem_native_1get_1extracted_1data(JNIEnv *env, jclass jclass1);
}

bool isSoundSystemInit();

SLDataLocator_URI *dataLocatorFromURLString(JNIEnv *env, jstring fileURLString);

SLDataLocator_AndroidFD getTrackFromAsset(JNIEnv *env, jobject assetManager, jstring filename);

#endif //TEST_SOUNDSYSTEM_SOUNDSYSTEM_ENTRYPOINT_H

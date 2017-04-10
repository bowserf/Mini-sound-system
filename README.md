# Mini Sound-system

A playground to learn playing music with OpenSL ES and to learn using OpenGL ES by drawing music
spectrum.

## How to use sound system library :

1. Make your app depends of the `soundsystem module`. Now, you have access to the `SoundSystem`
class which is the entry point to interact with the library.

2. Interesting things start here. You can call `initSoundSystem(int, int)` to initialize OpenSL ES
and send to native code sample rate and frames per buffer. These params will be used by extractor
and audio player. They are differents from one device to another so send the good one for your
device.

3. Call `loadFile(String)` with the path of the audio file on the device to start extracting it into
RAM. When it's finished, callback `onExtractionCompleted` is called.

4. When extraction has started, you can start playing music. `playMusic(boolean)` method allow to
start and pause playing. Normally, extraction is faster than playing so it doesn't matter if you
don't wait the `onExtractionCompleted` event before start playing.

5. To stop playing and set the reading position at the start, call `stopMusic()`.

6. To stop the sound system, call `release()` method which will free all objects.

## A word on the project :

### Module nativesoundsystem :

This package contains the whole logic to extract and play an audio file. Here, we don't use high
level code (java & Android SDK) to make the audio player but C++ code with OpenSL ES library.
We wanted to put hands in the sludge and understand what are behind Android SDK and how it works.
OpenSL ES (Open Sound Library for Embedded Systems) is the library used by Android to communicate
with hardware to be able to extract, play and apply some sound effects on music tracks.


### Module soundsystem :

This module plays the role of bridge between Android application wanting to play sound from
audio file and native code providing the feature.
With gradle-experimental, it's not possible to directly add a dependency on a native module for an
application. We can only add library module but library module can add native module with the
following code :

```java
android.sources {
    main {
        jniLibs {
            dependencies {
                project ":nativesoundsystem"
            }
        }
    }
}
```

That's why we added this module into the library instead of using only one module.

SoundSystem java class is the entry point for app wanting to use this library. It provides methods
to extract audio file into RAM. To play, pause, stop the current music and provide methods to get
extracted data in stereo and mono.


### Module app :

Module app is a sample displaying how use the `soundsystem` library. It's also a playground to use
OpenGL ES in order to display audio spectrum with a low level library instead of using Android SDK.

### Other branches

[AAudio player](https://github.com/bowserf/Mini-sound-system/tree/ft/aaudio_android_o) :
With Android O, Google provides a new Audio API called AAudio focus on low latency. It has been developed to improve performances compare to OpenSL player.
For now, we have only access to Android O preview 1 which has just a preview of the new API, without hoped performances but it provides a good idea of how it will work.
MinSdkVersion 26 is not available yet so we use a boolean define inside build.gradle project to use good native library inside nativesoundsystem project.
You need at NDK r15 to use AAudio because this version provide AAudio header files.

[CMake to compile native code](https://github.com/bowserf/Mini-sound-system/tree/ft/cmake_in_library) :
Use of standard android gradle with CMake in order to compile module `soundsystem` which contain native code.
It differs from gradle-experimental because we haven't to add a native module which contains only C & C++ code
and a library module which will be the bridge between native module and application module. Here, we have only
one library module.

[Vertex buffer object](https://github.com/bowserf/Mini-sound-system/tree/dev/vbo) : Improve OpenGL performance by using vertex buffer object.
With this method, we send data directly to the GPU RAM. GPU doesn't have to ask to CPU RAM what are
data because it already has it into its own RAM.

[Vertex array object](https://github.com/bowserf/Mini-sound-system/tree/dev/vao) : Available with OpenGL ES 3, it improves preformance over VBO.
Previously, the GPU had to ask to CPU where were data to display, the CPU answered in the GPU RAM.
With VAO, the GPU already knows where are data because we saved the call to data directly into the
GPU RAM.
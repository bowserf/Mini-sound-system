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

[Player read float data](https://github.com/bowserf/Mini-sound-system/tree/dev/test_player_float) : From Lollipop (API 21), an OpenSL ES player
can manage float data instead of short data.
Structure `SLAndroidDataFormat_PCM_EX` has been added and can replace `SLDataFormat_PCM`. You can
use field `representation` with value `SL_ANDROID_PCM_REPRESENTATION_FLOAT` to say that you work
with float data. Your code will stay the same instead that you need a float array buffer.

[Vertex buffer object](https://github.com/bowserf/Mini-sound-system/tree/dev/vbo): Improve OpenGL performance by using vertex buffer object.
With this method, we send data directly to the GPU RAM. GPU doesn't have to ask to CPU RAM what are
data because it already has it into its own RAM.

[Vertex array object](https://github.com/bowserf/Mini-sound-system/tree/dev/vao) : Available with OpenGL ES 3, it improves preformance over VBO.
Previously, the GPU had to ask to CPU where were data to display, the CPU answered in the GPU RAM.
With VAO, the GPU already knows where are data because we saved the call to data directly into the
GPU RAM.
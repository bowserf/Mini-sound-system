#ifdef AAUDIO

#ifndef AAUDIO_AUDIO_COMMON_H
#define AAUDIO_AUDIO_COMMON_H

#include <aaudio/AAudio.h>
#include <utils/android_debug.h>

/*
 * Audio Sample Controls...
 */
#define AUDIO_SAMPLE_CHANNELS 2

uint16_t SampleFormatToBpp(aaudio_format_t format);

/*
 * GetSystemTicks(void):  return the time in micro sec
 */
__inline__ uint64_t GetSystemTicks(void) {
    struct timeval Time;
    gettimeofday(&Time, NULL);

    return (static_cast<uint64_t>(1000000) * Time.tv_sec + Time.tv_usec);
}

/*
 * flag to enable file dumping
 */
//#define ENABLE_LOG  1

void PrintAudioStreamInfo(const AAudioStream *stream);

#endif // AAUDIO_AUDIO_COMMON_H

#endif

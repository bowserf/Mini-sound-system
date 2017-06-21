#ifdef AAUDIO

#ifndef AAUDIO_STREAM_BIULDER_H
#define AAUDIO_STREAM_BIULDER_H

#include <cassert>
#include <aaudio/AAudio.h>

#define INVALID_AUDIO_PARAM 0

class StreamBuilder {

public:

    explicit StreamBuilder() {
        aaudio_result_t result = AAudio_createStreamBuilder(&builder);
        if (result != AAUDIO_OK && !builder) {
            assert(false);
        }
    }

    ~StreamBuilder() {
        if (builder)
            AAudioStreamBuilder_delete(builder);
        builder = nullptr;
    };

    AAudioStream *CreateStream(
            aaudio_format_t format,
            int32_t samplesPerFrame,
            aaudio_sharing_mode_t sharing,
            aaudio_performance_mode_t performanceMode = AAUDIO_PERFORMANCE_MODE_NONE,
            aaudio_direction_t dir = AAUDIO_DIRECTION_OUTPUT,
            int32_t sampleRate = INVALID_AUDIO_PARAM,
            AAudioStream_dataCallback callback = nullptr,
            void *userData = nullptr) {

        AAudioStreamBuilder_setFormat(builder, format);
        AAudioStreamBuilder_setSharingMode(builder, sharing);
        AAudioStreamBuilder_setDirection(builder, dir);
        AAudioStreamBuilder_setSampleRate(builder, sampleRate);
        AAudioStreamBuilder_setPerformanceMode(builder, performanceMode);
        AAudioStreamBuilder_setSamplesPerFrame(builder, samplesPerFrame);
        if (sampleRate != INVALID_AUDIO_PARAM) {
            AAudioStreamBuilder_setSampleRate(builder, sampleRate);
        }
        AAudioStreamBuilder_setDataCallback(builder, callback, userData);

        AAudioStream *stream;
        aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &stream);
        if (result != AAUDIO_OK) {
            assert(false);
            stream = nullptr;
        }
        return stream;
    }

private:

    AAudioStreamBuilder *builder;

};

#endif //AAUDIO_STREAM_BIULDER_H

#endif
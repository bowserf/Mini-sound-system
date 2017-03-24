/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef AAUDIO

#ifndef AAUDIO_AAUDIODEFINITIONS_H
#define AAUDIO_AAUDIODEFINITIONS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This is used to represent a value that has not been specified.
 * For example, an application could use AAUDIO_UNSPECIFIED to indicate
 * that is did not not care what the specific value of a parameter was
 * and would accept whatever it was given.
 */
#define AAUDIO_UNSPECIFIED           0
#define AAUDIO_DEVICE_UNSPECIFIED    ((int32_t) -1)

enum {
    AAUDIO_DIRECTION_OUTPUT = 0,
    AAUDIO_DIRECTION_INPUT = 1
};
typedef int32_t aaudio_direction_t;

enum {
    AAUDIO_FORMAT_INVALID = -1,
    AAUDIO_FORMAT_UNSPECIFIED = 0,
    AAUDIO_FORMAT_PCM_I16 = 1,
    AAUDIO_FORMAT_PCM_FLOAT = 2,
    AAUDIO_FORMAT_PCM_I8_24 = 3,
    AAUDIO_FORMAT_PCM_I32 = 4
};
typedef int32_t aaudio_audio_format_t;

enum {
    AAUDIO_OK = 0,
    AAUDIO_ERROR_BASE = -900,
    AAUDIO_ERROR_DISCONNECTED = -899,
    AAUDIO_ERROR_ILLEGAL_ARGUMENT = -898,
    AAUDIO_ERROR_INCOMPATIBLE = -897,
    AAUDIO_ERROR_INTERNAL = -896, // an underlying API returned an error code
    AAUDIO_ERROR_INVALID_STATE = -895,
    AAUDIO_ERROR_UNEXPECTED_STATE = -894,
    AAUDIO_ERROR_UNEXPECTED_VALUE = -893,
    AAUDIO_ERROR_INVALID_HANDLE = -892,
    AAUDIO_ERROR_INVALID_QUERY = -891,
    AAUDIO_ERROR_UNIMPLEMENTED = -890,
    AAUDIO_ERROR_UNAVAILABLE = -889,
    AAUDIO_ERROR_NO_FREE_HANDLES = -888,
    AAUDIO_ERROR_NO_MEMORY = -887,
    AAUDIO_ERROR_NULL = -886,
    AAUDIO_ERROR_TIMEOUT = -885,
    AAUDIO_ERROR_WOULD_BLOCK = -884,
    AAUDIO_ERROR_INVALID_ORDER = -883,
    AAUDIO_ERROR_OUT_OF_RANGE = -882,
    AAUDIO_ERROR_NO_SERVICE = -881
};
typedef int32_t  aaudio_result_t;

enum
{
    AAUDIO_STREAM_STATE_UNINITIALIZED = 0,
    AAUDIO_STREAM_STATE_UNKNOWN = 1,
    AAUDIO_STREAM_STATE_OPEN = 2,
    AAUDIO_STREAM_STATE_STARTING = 3,
    AAUDIO_STREAM_STATE_STARTED = 4,
    AAUDIO_STREAM_STATE_PAUSING = 5,
    AAUDIO_STREAM_STATE_PAUSED = 6,
    AAUDIO_STREAM_STATE_FLUSHING = 7,
    AAUDIO_STREAM_STATE_FLUSHED = 8,
    AAUDIO_STREAM_STATE_STOPPING = 9,
    AAUDIO_STREAM_STATE_STOPPED = 10,
    AAUDIO_STREAM_STATE_CLOSING = 11,
    AAUDIO_STREAM_STATE_CLOSED = 12,
};
typedef int32_t aaudio_stream_state_t;

enum {
    /**
     * This will be the only stream using a particular source or sink.
     * This mode will provide the lowest possible latency.
     * You should close EXCLUSIVE streams immediately when you are not using them.
     */
    AAUDIO_SHARING_MODE_EXCLUSIVE,
    /**
     * Multiple applications will be mixed by the AAudio Server.
     * This will have higher latency than the EXCLUSIVE mode.
     */
    AAUDIO_SHARING_MODE_SHARED
};
typedef int32_t aaudio_sharing_mode_t;

#ifdef __cplusplus
}
#endif

#endif // AAUDIO_AAUDIODEFINITIONS_H

#endif
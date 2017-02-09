/*
 * Copyright 2015 The Android Open Source Project
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

#define LOG_TAG "AAudio"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <new>
#include <stdint.h>

#include <aaudio/AAudioDefinitions.h>
#include <aaudio/AAudio.h>

#include "client/AudioStreamInternal.h"
#include "core/AudioStream.h"
#include "core/AudioStreamBuilder.h"
#include "legacy/AudioStreamRecord.h"
#include "legacy/AudioStreamTrack.h"

using namespace aaudio;

/*
 * AudioStreamBuilder
 */
AudioStreamBuilder::AudioStreamBuilder() {
}

AudioStreamBuilder::~AudioStreamBuilder() {
}

aaudio_result_t AudioStreamBuilder::build(AudioStream** streamPtr) {
    // TODO Is there a better place to put the code that decides which class to use?
    AudioStream* audioStream = nullptr;
    const aaudio_sharing_mode_t sharingMode = getSharingMode();
    switch (getDirection()) {
    case AAUDIO_DIRECTION_INPUT:
        switch (sharingMode) {
            case AAUDIO_SHARING_MODE_LEGACY:
                audioStream = new(std::nothrow) AudioStreamRecord();
                break;
            default:
                ALOGE("AudioStreamBuilder(): bad sharing mode = %d", sharingMode);
                return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
                break;
        }
        break;
    case AAUDIO_DIRECTION_OUTPUT:
        switch (sharingMode) {
            case AAUDIO_SHARING_MODE_LEGACY:
                audioStream = new(std::nothrow) AudioStreamTrack();
                break;
            case AAUDIO_SHARING_MODE_EXCLUSIVE:
                audioStream = new(std::nothrow) AudioStreamInternal();
                break;
            default:
                ALOGE("AudioStreamBuilder(): bad sharing mode = %d", sharingMode);
                return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
                break;
        }
        break;
    default:
        ALOGE("AudioStreamBuilder(): bad direction = %d", getDirection());
        return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
        break;
    }
    if (audioStream == nullptr) {
        return AAUDIO_ERROR_NO_MEMORY;
    }
    ALOGD("AudioStreamBuilder(): created audioStream = %p", audioStream);

    // TODO maybe move this out of build and pass the builder to the constructors
    // Open the stream using the parameters from the builder.
    const aaudio_result_t result = audioStream->open(*this);
    if (result != AAUDIO_OK) {
        delete audioStream;
    } else {
        *streamPtr = audioStream;
    }
    return result;
}

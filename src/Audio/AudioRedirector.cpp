#include "AudioRedirector.hpp"
#include <format>
#include <cassert>

#include "MAConvert.hpp"

#define MINIAUDIO_IMPLEMENTATION

namespace internal {
    namespace loopback {
        ma_format format = ma_format_f32;   // Default format
        ma_uint32 channels = 2;             // Default to stereo
        ma_uint32 sampleRate = 48000;       // Default sample rate
    };

    namespace duplex {
        ma_format format = ma_format_f32;   // Default format
        ma_uint32 channels = 2;             // Default to stereo
        ma_uint32 sampleRate = 48000;       // Default sample rate
    };

    ma_context context;
    ma_device duplexDevice = {};
    ma_device loopbackDevice = {};
    ma_device playbackDevice = {};
    ma_pcm_rb ringBuffer;

    // ------------------------------------------------------------------------
    // Internal helpers
    // ------------------------------------------------------------------------

    ma_result init_loopback_device(const ma_device_id *id);
    ma_result init_playback_device(const ma_device_id *id);
    ma_result init_duplex_device(const ma_device_id *inputId, const ma_device_id *playbackId);

    void data_callback_loopback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
    void data_callback_playback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
    void data_callback_duplex(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
};

// ============================================================================
// Public API accessors
// ============================================================================

ma_format AudioRedirector::GetLoopbackFormat() { return internal::loopback::format; }
ma_uint32 AudioRedirector::GetLoopbackSampleRate() { return internal::loopback::sampleRate; }

void AudioRedirector::SetLoopbackFormat(ma_format format) { internal::loopback::format = format; }
void AudioRedirector::SetLoopbackSampleRate(ma_uint32 sampleRate) { internal::loopback::sampleRate = sampleRate; }

ma_format AudioRedirector::GetDuplexFormat() { return internal::duplex::format; }
ma_uint32 AudioRedirector::GetDuplexSampleRate() { return internal::duplex::sampleRate; }

void AudioRedirector::SetDuplexFormat(ma_format format) { internal::duplex::format = format; }
void AudioRedirector::SetDuplexSampleRate(ma_uint32 sampleRate) { internal::loopback::sampleRate = sampleRate; }

Result<AudioDevices, Error> AudioRedirector::GetAudioDevices() { 
    AudioDevices devices = { nullptr, 0, nullptr, 0 };

    ma_result result = ma_context_get_devices(
        &internal::context,
        &devices.playbackDeviceInfos, &devices.playbackDeviceCount,
        &devices.captureDeviceInfos, &devices.captureDeviceCount
    );

    if (result != MA_SUCCESS) {
        return Error(std::format(
            "Failed to get devices list ({}).",
            ma::convert::to_string(result)
        ));
    }

    return devices;
}

Result<float, Error> AudioRedirector::GetPlaybackVolume() {
    float volume = 0.0f;
    ma_result result = ma_device_get_master_volume(
        &internal::playbackDevice, &volume
    );
    if (result != MA_SUCCESS) {
        return Error(std::format(
            "Failed to get playback device volume ({}).",
            ma::convert::to_string(result)
        ));
    }
    return volume;
}

ma_result AudioRedirector::SetPlaybackVolume(float volume) {
    return ma_device_set_master_volume(&internal::playbackDevice, volume);
}

Result<float, Error> AudioRedirector::GetDuplexVolume() {
    float volume = 0.0f;
    ma_result result = ma_device_get_master_volume(
        &internal::duplexDevice, &volume
    );
    if (result != MA_SUCCESS) {
        return Error(std::format(
            "Failed to get duplex device volume ({}).",
            ma::convert::to_string(result)
        ));
    }
    return volume;
}

ma_result AudioRedirector::SetDuplexVolume(float volume) {
    return ma_device_set_master_volume(&internal::duplexDevice, volume);
}

// ============================================================================
// Main Implementation
// Actual implementation logic starts here
// ============================================================================

ResultVoid AudioRedirector::Initialize()
{
    ma_result result = ma_context_init(nullptr, 0, nullptr, &internal::context);

    if (result != MA_SUCCESS) {
        return Error(std::format(
            "Failed to initialize miniaudio context ({}).",
            ma::convert::to_string(result)
        ));
    }

    return std::monostate{};
}

ResultVoid AudioRedirector::Uninitialize()
{
    StopLoopbackRedirect(); // Ensure devices are stopped and uninitialized
    StopDuplexRedirect();

    ma_result result = ma_context_uninit(&internal::context);
    if (result != MA_SUCCESS) {
        return Error(std::format(
            "Failed to uninitialize miniaudio context ({}).",
            ma::convert::to_string(result)
        ));
    }

    return std::monostate{};
}

ResultVoid AudioRedirector::StartLoopbackRedirect(const ma_device_id *loopbackId, const ma_device_id *playbackId)
{
    ma_result result = internal::init_loopback_device(loopbackId);

    if (result != MA_SUCCESS) {
        return Error(std::format(
            "Failed to initialize loopback device ({}).",
            ma::convert::to_string(result)
        ));
    }

    result = internal::init_playback_device(playbackId);

    if (result != MA_SUCCESS) {
        ma_device_uninit(&internal::loopbackDevice);
        return Error(std::format(
            "Failed to initialize playback device ({}).",
            ma::convert::to_string(result)
        ));
    }

    // Init ring buffer (1 second at 48kHz, stereo)
    result = ma_pcm_rb_init(
        internal::loopback::format,
        internal::loopback::channels,
        internal::loopback::sampleRate,  // bufferSizeInFrames
        nullptr,                         // let miniaudio allocate the buffer
        nullptr,                         // use default allocator for buffer
        &internal::ringBuffer
    );

    if (result != MA_SUCCESS) {
        ma_device_uninit(&internal::loopbackDevice);
        ma_device_uninit(&internal::playbackDevice);

        return Error(std::format(
            "Failed to initialize ring buffer ({}).",
            ma::convert::to_string(result)
        ));
    }

    const ma_result loopback_result = ma_device_start(&internal::loopbackDevice);
    const ma_result playback_result = ma_device_start(&internal::playbackDevice);

    if (loopback_result != MA_SUCCESS || playback_result != MA_SUCCESS) {
        ma_device_uninit(&internal::loopbackDevice);
        ma_device_uninit(&internal::playbackDevice);
        ma_pcm_rb_uninit(&internal::ringBuffer);

        return Error(std::format(
            "Failed to start {} device ({}).",
            (loopback_result != MA_SUCCESS) ? "loopback" : "playback",
            ma::convert::to_string((loopback_result != MA_SUCCESS) ? loopback_result : playback_result))
        );
    }

    return std::monostate{};
}

ResultVoid AudioRedirector::StopLoopbackRedirect()
{
    /* Stop and Uninitialize loopback device */

    ma_device_state device_state = ma_device_get_state(&internal::loopbackDevice);

    if (device_state == ma_device_state_started || device_state == ma_device_state_starting) {
        ma_result result = ma_device_stop(&internal::loopbackDevice);
        if (result != MA_SUCCESS) {
            return Error(std::format(
                "Failed to stop input device ({}).",
                ma::convert::to_string(result)
            ));
        }
    }

    if (device_state != ma_device_state_uninitialized) {
        ma_device_uninit(&internal::loopbackDevice);
    }

    /* Stop and Uninitialize playback device */

    device_state = ma_device_get_state(&internal::playbackDevice);

    if (device_state == ma_device_state_started || device_state == ma_device_state_starting) {
        ma_result result = ma_device_stop(&internal::playbackDevice);
        if (result != MA_SUCCESS) {
            return Error(std::format(
                "Failed to stop playback device ({}).",
                ma::convert::to_string(result)
            ));
        }
    }

    if (device_state != ma_device_state_uninitialized) {
        ma_device_uninit(&internal::playbackDevice);
    }  
    
    /* Uninitialize Ring buffer */

    if (&internal::ringBuffer != nullptr && &internal::ringBuffer.rb != nullptr) {
        ma_pcm_rb_uninit(&internal::ringBuffer);
    }

    return std::monostate{};
}

ResultVoid AudioRedirector::StartDuplexRedirect(const ma_device_id *captureId, const ma_device_id *playbackId)
{
    ma_device_state device_state = ma_device_state_uninitialized;
    if (internal::duplexDevice.pContext != nullptr) {
        device_state = ma_device_get_state(&internal::duplexDevice);
    }

    if (device_state == ma_device_state_started || device_state == ma_device_state_starting) {
        ma_result result = ma_device_stop(&internal::duplexDevice);
        if (result != MA_SUCCESS) {
            return Error(std::format(
                "Failed to stop duplex device ({}).",
                ma::convert::to_string(result)
            ));
        }
    }

    if (device_state != ma_device_state_uninitialized) {
        ma_device_uninit(&internal::duplexDevice);
    }

    ma_result result = internal::init_duplex_device(captureId, playbackId);

    if (result != MA_SUCCESS) {
        return Error(std::format(
            "Failed to initialize duplex device ({}).",
            ma::convert::to_string(result)
        ));
    }

    result = ma_device_start(&internal::duplexDevice);

    if (result != MA_SUCCESS) {
        ma_device_uninit(&internal::duplexDevice);

        return Error(std::format(
            "Failed to start duplex device ({}).",
            ma::convert::to_string(result)
        ));
    }

    return std::monostate{};
}

ResultVoid AudioRedirector::StopDuplexRedirect()
{
    ma_device_state device_state = ma_device_get_state(&internal::duplexDevice);

    if (device_state == ma_device_state_started || device_state == ma_device_state_starting) {
        ma_result result = ma_device_stop(&internal::duplexDevice);
        if (result != MA_SUCCESS) {
            return Error(std::format(
                "Failed to stop duplex device ({}).",
                ma::convert::to_string(result)
            ));
        }
    }

    if (device_state != ma_device_state_uninitialized) {
        ma_device_uninit(&internal::duplexDevice);
    }

    return std::monostate{};
}

ma_result internal::init_loopback_device(const ma_device_id *id) {
    // --- Configure loopback capture ---
    ma_device_config config = ma_device_config_init(ma_device_type_loopback);
    config.capture.pDeviceID = id;
    config.capture.format = internal::loopback::format;
    config.capture.channels = internal::loopback::channels;
    config.sampleRate = internal::loopback::sampleRate;
    config.dataCallback = internal::data_callback_loopback;

    return ma_device_init(&internal::context, &config, &internal::loopbackDevice);
}

ma_result internal::init_playback_device(const ma_device_id *id) {
    // --- Configure playback ---
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = id;
    config.playback.format = internal::loopback::format;
    config.playback.channels = internal::loopback::channels;
    config.sampleRate = internal::loopback::sampleRate;
    config.dataCallback = internal::data_callback_playback;

    return ma_device_init(&internal::context, &config, &internal::playbackDevice);
}

ma_result internal::init_duplex_device(const ma_device_id *captureId, const ma_device_id *playbackId) {
    // --- Configure duplex ---
    ma_device_config config = ma_device_config_init(ma_device_type_duplex);
    config.capture.pDeviceID = captureId;
    config.playback.pDeviceID = playbackId;
    config.capture.format = internal::duplex::format;
    config.capture.channels = internal::duplex::channels;
    config.playback.format = internal::duplex::format;
    config.playback.channels = internal::duplex::channels;
    config.sampleRate = internal::duplex::sampleRate;
    config.dataCallback = internal::data_callback_duplex;

    return ma_device_init(&internal::context, &config, &internal::duplexDevice);
}

void internal::data_callback_duplex(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    /* Assert that the playback and capture sides use the same format and channel count. */
    assert(pDevice->capture.format == pDevice->playback.format && "Format mismatch");
    assert(pDevice->capture.channels == pDevice->playback.channels && "Channel count mismatch");

    /* Since the format and channel count are the same for both input and output which means we can just memcpy(). */
    memcpy(pOutput, pInput, frameCount * ma_get_bytes_per_frame(pDevice->capture.format, pDevice->capture.channels));
}

// Loopback -> write to RB
void internal::data_callback_loopback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    (void)pDevice; (void)pOutput;

    float* pWrite = nullptr;
    ma_uint32 framesToWrite = frameCount; // in/out

    if (ma_pcm_rb_acquire_write(&internal::ringBuffer, &framesToWrite, (void**)&pWrite) == MA_SUCCESS && framesToWrite > 0) {
        const size_t bytesPerFrame = ma_get_bytes_per_frame(internal::loopback::format, internal::loopback::channels);
        memcpy(pWrite, pInput, (size_t)(framesToWrite * bytesPerFrame));
        ma_pcm_rb_commit_write(&internal::ringBuffer, framesToWrite);
    }
}

// Playback -> read from RB
void internal::data_callback_playback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    (void)pDevice; (void)pInput;

    float* pRead = nullptr;
    ma_uint32 framesToRead = frameCount; // in/out
    const size_t bytesPerFrame = ma_get_bytes_per_frame(internal::loopback::format, internal::loopback::channels);

    size_t bytesFilled = 0;
    if (ma_pcm_rb_acquire_read(&internal::ringBuffer, &framesToRead, (void**)&pRead) == MA_SUCCESS && framesToRead > 0) {
        const size_t bytes = (size_t)(framesToRead * bytesPerFrame);
        memcpy(pOutput, pRead, bytes);
        ma_pcm_rb_commit_read(&internal::ringBuffer, framesToRead);
        bytesFilled = bytes;
    }

    // Pad any unfilled output with silence.
    const size_t totalBytes = (size_t)frameCount * bytesPerFrame;
    if (bytesFilled < totalBytes) {
        memset((ma_uint8*)pOutput + bytesFilled, 0, totalBytes - bytesFilled);
    }
}

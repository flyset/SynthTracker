#include "coreaudio_facade.h"

#include <stddef.h>

#if defined(__APPLE__)

#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

static_assert(ATOMIC_INT_LOCK_FREE == 2,
              "HAL callback admission must be lock-free");

typedef enum {
    HAL_CALLBACK_CLOSED,
    HAL_CALLBACK_OPEN,
    HAL_CALLBACK_IN_FLIGHT,
    HAL_CALLBACK_CLOSING,
} hal_callback_admission_state;

typedef struct {
    AudioUnit output_unit;
    AudioDeviceID output_device;
    audio_output_coreaudio_request request;
    void *request_context;
    audio_output_coreaudio_format format;
    atomic_int callback_admission;
    bool opened;
    bool configured;
    bool bound;
    bool initialized;
    bool active;
} coreaudio_hal_output_context;

static coreaudio_hal_output_context default_context = {
    .callback_admission = HAL_CALLBACK_CLOSED,
};

static bool admit_hal_callback(coreaudio_hal_output_context *context)
{
    int expected = HAL_CALLBACK_OPEN;
    return atomic_compare_exchange_strong_explicit(
        &context->callback_admission, &expected, HAL_CALLBACK_IN_FLIGHT,
        memory_order_acquire, memory_order_relaxed);
}

static void release_hal_callback(coreaudio_hal_output_context *context)
{
    int expected = HAL_CALLBACK_IN_FLIGHT;
    if (atomic_compare_exchange_strong_explicit(
        &context->callback_admission, &expected, HAL_CALLBACK_OPEN,
        memory_order_release, memory_order_relaxed)) {
        return;
    }

    expected = HAL_CALLBACK_CLOSING;
    (void)atomic_compare_exchange_strong_explicit(
        &context->callback_admission, &expected, HAL_CALLBACK_CLOSED,
        memory_order_release, memory_order_relaxed);
}

static void close_and_drain_hal_callbacks(
    coreaudio_hal_output_context *context)
{
    for (;;) {
        int current = atomic_load_explicit(&context->callback_admission,
                                           memory_order_acquire);
        if (current == HAL_CALLBACK_CLOSED) {
            return;
        }
        if (current == HAL_CALLBACK_OPEN) {
            int expected = HAL_CALLBACK_OPEN;
            if (atomic_compare_exchange_weak_explicit(
                    &context->callback_admission, &expected,
                    HAL_CALLBACK_CLOSED, memory_order_acq_rel,
                    memory_order_acquire)) {
                return;
            }
            continue;
        }
        if (current == HAL_CALLBACK_IN_FLIGHT) {
            int expected = HAL_CALLBACK_IN_FLIGHT;
            if (atomic_compare_exchange_weak_explicit(
                    &context->callback_admission, &expected,
                    HAL_CALLBACK_CLOSING, memory_order_acq_rel,
                    memory_order_acquire)) {
                current = HAL_CALLBACK_CLOSING;
            }
        }
        if (current == HAL_CALLBACK_CLOSING) {
            while (atomic_load_explicit(&context->callback_admission,
                                        memory_order_acquire) ==
                   HAL_CALLBACK_CLOSING) {
            }
            return;
        }
    }
}

static bool supported_rate(uint32_t sample_rate_hz)
{
    return sample_rate_hz == 44100 || sample_rate_hz == 48000;
}

static bool strict_static_format(const audio_output_coreaudio_format *format)
{
    return format != NULL && format->channel_count == 2 &&
           format->sample_format == AUDIO_OUTPUT_COREAUDIO_SAMPLE_FORMAT_FLOAT32 &&
           format->layout == AUDIO_OUTPUT_COREAUDIO_LAYOUT_INTERLEAVED;
}

static bool strict_requested_format(
    const audio_output_coreaudio_format *format)
{
    return strict_static_format(format) &&
           (format->sample_rate_hz == 0 ||
            supported_rate(format->sample_rate_hz));
}

static bool strict_stream_format(
    const AudioStreamBasicDescription *stream,
    uint32_t sample_rate_hz)
{
    return stream != NULL && stream->mSampleRate == (Float64)sample_rate_hz &&
           stream->mFormatID == kAudioFormatLinearPCM &&
           (stream->mFormatFlags & kAudioFormatFlagIsFloat) != 0 &&
           (stream->mFormatFlags & kAudioFormatFlagIsPacked) != 0 &&
           (stream->mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0 &&
           stream->mBytesPerPacket == 2 * sizeof(Float32) &&
           stream->mFramesPerPacket == 1 &&
           stream->mBytesPerFrame == 2 * sizeof(Float32) &&
           stream->mChannelsPerFrame == 2 && stream->mBitsPerChannel == 32;
}

static void reset_context(coreaudio_hal_output_context *context)
{
    context->output_unit = NULL;
    context->output_device = kAudioObjectUnknown;
    context->request = NULL;
    context->request_context = NULL;
    context->format = (audio_output_coreaudio_format){0};
    context->opened = false;
    context->configured = false;
    context->bound = false;
    context->initialized = false;
    context->active = false;
    atomic_store_explicit(&context->callback_admission, HAL_CALLBACK_CLOSED,
                          memory_order_release);
}

static void dispose_unopened_unit(coreaudio_hal_output_context *context)
{
    if (context->output_unit != NULL) {
        (void)AudioComponentInstanceDispose(context->output_unit);
    }
    reset_context(context);
}

static OSStatus get_default_output_device(AudioDeviceID *device)
{
    if (device == NULL) {
        return kAudio_ParamError;
    }

    const AudioObjectPropertyAddress address = {
        .mSelector = kAudioHardwarePropertyDefaultOutputDevice,
        .mScope = kAudioObjectPropertyScopeGlobal,
        .mElement = kAudioObjectPropertyElementMain,
    };
    UInt32 size = sizeof(*device);
    return AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0,
                                      NULL, &size, device);
}

static OSStatus get_device_nominal_rate(
    AudioDeviceID device,
    Float64 *sample_rate_hz)
{
    if (sample_rate_hz == NULL) {
        return kAudio_ParamError;
    }

    const AudioObjectPropertyAddress address = {
        .mSelector = kAudioDevicePropertyNominalSampleRate,
        .mScope = kAudioObjectPropertyScopeGlobal,
        .mElement = kAudioObjectPropertyElementMain,
    };
    UInt32 size = sizeof(*sample_rate_hz);
    return AudioObjectGetPropertyData(device, &address, 0, NULL, &size,
                                      sample_rate_hz);
}

static OSStatus hal_output_render_callback(
    void *context,
    AudioUnitRenderActionFlags *action_flags,
    const AudioTimeStamp *time_stamp,
    UInt32 bus_number,
    UInt32 frame_count,
    AudioBufferList *io_data)
{
    (void)action_flags;
    (void)time_stamp;
    (void)bus_number;

    coreaudio_hal_output_context *hal = context;
    if (hal == NULL || !admit_hal_callback(hal)) {
        return kAudio_ParamError;
    }

    OSStatus status = noErr;
    float *samples = NULL;
    size_t frame_capacity = 0;
    size_t required_byte_count = 0;
    bool size_valid = (size_t)frame_count <= SIZE_MAX / 2;
    if (size_valid) {
        const size_t sample_count = (size_t)frame_count * 2;
        size_valid = sample_count <= SIZE_MAX / sizeof(float);
        if (size_valid) {
            required_byte_count = sample_count * sizeof(float);
        }
    }

    if (frame_count > 0) {
        if (io_data == NULL || io_data->mNumberBuffers != 1 ||
            io_data->mBuffers[0].mNumberChannels != 2 ||
            io_data->mBuffers[0].mData == NULL ||
            !size_valid ||
            (size_t)io_data->mBuffers[0].mDataByteSize < required_byte_count) {
            status = kAudio_ParamError;
        } else {
            samples = io_data->mBuffers[0].mData;
            frame_capacity = frame_count;
        }
    }

    if (status == noErr && hal->request == NULL) {
        status = kAudio_ParamError;
    }

    if (status == noErr &&
        hal->request(hal->request_context, frame_count, samples,
                     frame_capacity) != AUDIO_OUTPUT_SUBMIT_ACCEPTED) {
        status = kAudio_ParamError;
    }

    release_hal_callback(hal);
    return status;
}

static audio_output_coreaudio_facade_result hal_open(void *raw_context)
{
    coreaudio_hal_output_context *context = raw_context;
    if (context == NULL || context->output_unit != NULL) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    const AudioComponentDescription description = {
        .componentType = kAudioUnitType_Output,
        .componentSubType = kAudioUnitSubType_HALOutput,
        .componentManufacturer = kAudioUnitManufacturer_Apple,
        .componentFlags = 0,
        .componentFlagsMask = 0,
    };
    const AudioComponent component = AudioComponentFindNext(NULL, &description);
    if (component == NULL) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE;
    }

    AudioUnit output_unit = NULL;
    if (AudioComponentInstanceNew(component, &output_unit) != noErr) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }
    context->output_unit = output_unit;

    AudioDeviceID device = kAudioObjectUnknown;
    if (get_default_output_device(&device) != noErr ||
        device == kAudioObjectUnknown) {
        dispose_unopened_unit(context);
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }
    context->output_device = device;

    UInt32 enable_io = 0;
    if (AudioUnitSetProperty(
            output_unit, kAudioOutputUnitProperty_EnableIO,
            kAudioUnitScope_Input, 1, &enable_io, sizeof(enable_io)) != noErr) {
        dispose_unopened_unit(context);
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    enable_io = 1;
    if (AudioUnitSetProperty(
            output_unit, kAudioOutputUnitProperty_EnableIO,
            kAudioUnitScope_Output, 0, &enable_io, sizeof(enable_io)) != noErr) {
        dispose_unopened_unit(context);
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    if (AudioUnitSetProperty(
            output_unit, kAudioOutputUnitProperty_CurrentDevice,
            kAudioUnitScope_Global, 0, &device, sizeof(device)) != noErr) {
        dispose_unopened_unit(context);
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    context->opened = true;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_OK;
}

static audio_output_coreaudio_facade_result hal_configure(
    void *raw_context,
    const audio_output_coreaudio_format *format,
    audio_output_coreaudio_format *configured_format)
{
    coreaudio_hal_output_context *context = raw_context;
    if (context == NULL || !context->opened || context->output_unit == NULL ||
        !strict_requested_format(format) || configured_format == NULL) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    Float64 device_rate = 0;
    if (get_device_nominal_rate(context->output_device, &device_rate) != noErr ||
        !(device_rate == 44100.0 || device_rate == 48000.0) ||
        (format->sample_rate_hz != 0 &&
         device_rate != (Float64)format->sample_rate_hz)) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    const AudioStreamBasicDescription stream = {
        .mSampleRate = device_rate,
        .mFormatID = kAudioFormatLinearPCM,
        .mFormatFlags = kAudioFormatFlagsNativeFloatPacked,
        .mBytesPerPacket = 2 * sizeof(Float32),
        .mFramesPerPacket = 1,
        .mBytesPerFrame = 2 * sizeof(Float32),
        .mChannelsPerFrame = 2,
        .mBitsPerChannel = 32,
        .mReserved = 0,
    };
    if (!strict_stream_format(&stream, (uint32_t)device_rate) ||
        AudioUnitSetProperty(
            context->output_unit, kAudioUnitProperty_StreamFormat,
            kAudioUnitScope_Input, 0, &stream, sizeof(stream)) != noErr) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    AudioStreamBasicDescription negotiated = {0};
    UInt32 negotiated_size = sizeof(negotiated);
    if (AudioUnitGetProperty(
            context->output_unit, kAudioUnitProperty_StreamFormat,
            kAudioUnitScope_Input, 0, &negotiated, &negotiated_size) != noErr ||
        !strict_stream_format(&negotiated, (uint32_t)device_rate)) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    context->format = *format;
    context->format.sample_rate_hz = (uint32_t)device_rate;
    context->configured = true;
    *configured_format = context->format;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_OK;
}

static audio_output_coreaudio_facade_result hal_bind_request(
    void *raw_context,
    audio_output_coreaudio_request request,
    void *request_context)
{
    coreaudio_hal_output_context *context = raw_context;
    if (context == NULL || !context->configured || request == NULL ||
        context->output_unit == NULL) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    const AURenderCallbackStruct callback = {
        .inputProc = hal_output_render_callback,
        .inputProcRefCon = context,
    };
    if (AudioUnitSetProperty(
            context->output_unit, kAudioUnitProperty_SetRenderCallback,
            kAudioUnitScope_Input, 0, &callback, sizeof(callback)) != noErr) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    context->request = request;
    context->request_context = request_context;
    context->bound = true;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_OK;
}

static audio_output_coreaudio_facade_result hal_start(void *raw_context)
{
    coreaudio_hal_output_context *context = raw_context;
    if (context == NULL || !context->bound || context->output_unit == NULL) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    atomic_store_explicit(&context->callback_admission, HAL_CALLBACK_OPEN,
                          memory_order_release);
    if (AudioUnitInitialize(context->output_unit) != noErr) {
        close_and_drain_hal_callbacks(context);
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }
    context->initialized = true;

    if (AudioOutputUnitStart(context->output_unit) != noErr) {
        close_and_drain_hal_callbacks(context);
        (void)AudioUnitUninitialize(context->output_unit);
        context->initialized = false;
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    context->active = true;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_OK;
}

static audio_output_coreaudio_facade_result hal_stop(void *raw_context)
{
    coreaudio_hal_output_context *context = raw_context;
    if (context == NULL || !context->initialized || context->output_unit == NULL) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    close_and_drain_hal_callbacks(context);
    if (context->active && AudioOutputUnitStop(context->output_unit) != noErr) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }
    context->active = false;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_OK;
}

static audio_output_coreaudio_facade_result hal_quiesce(void *raw_context)
{
    coreaudio_hal_output_context *context = raw_context;
    if (context == NULL || context->output_unit == NULL) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    close_and_drain_hal_callbacks(context);
    return AUDIO_OUTPUT_COREAUDIO_FACADE_OK;
}

static audio_output_coreaudio_facade_result hal_dispose(void *raw_context)
{
    coreaudio_hal_output_context *context = raw_context;
    if (context == NULL || context->output_unit == NULL || context->active ||
        atomic_load_explicit(&context->callback_admission,
                             memory_order_acquire) != HAL_CALLBACK_CLOSED) {
        return AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
    }

    OSStatus status = noErr;
    if (context->initialized) {
        status = AudioUnitUninitialize(context->output_unit);
    }
    const OSStatus dispose_status =
        AudioComponentInstanceDispose(context->output_unit);
    reset_context(context);
    return status == noErr && dispose_status == noErr
               ? AUDIO_OUTPUT_COREAUDIO_FACADE_OK
               : AUDIO_OUTPUT_COREAUDIO_FACADE_FAILED;
}

static const audio_output_coreaudio_facade default_facade = {
    .context = &default_context,
    .workspace_only = true,
    .open = hal_open,
    .configure = hal_configure,
    .bind_request = hal_bind_request,
    .start = hal_start,
    .stop = hal_stop,
    .quiesce = hal_quiesce,
    .dispose = hal_dispose,
};

#else

static audio_output_coreaudio_facade_result unavailable_open(void *context)
{
    (void)context;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE;
}

static audio_output_coreaudio_facade_result unavailable_configure(
    void *context,
    const audio_output_coreaudio_format *format,
    audio_output_coreaudio_format *configured_format)
{
    (void)context;
    (void)format;
    (void)configured_format;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE;
}

static audio_output_coreaudio_facade_result unavailable_start(void *context)
{
    (void)context;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE;
}

static audio_output_coreaudio_facade_result unavailable_quiesce(void *context)
{
    (void)context;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE;
}

static audio_output_coreaudio_facade_result unavailable_stop(void *context)
{
    (void)context;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE;
}

static audio_output_coreaudio_facade_result unavailable_dispose(void *context)
{
    (void)context;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE;
}

static audio_output_coreaudio_facade_result unavailable_bind_request(
    void *context,
    audio_output_coreaudio_request request,
    void *request_context)
{
    (void)context;
    (void)request;
    (void)request_context;
    return AUDIO_OUTPUT_COREAUDIO_FACADE_UNAVAILABLE;
}

static const audio_output_coreaudio_facade default_facade = {
    .context = NULL,
    .open = unavailable_open,
    .configure = unavailable_configure,
    .bind_request = unavailable_bind_request,
    .start = unavailable_start,
    .stop = unavailable_stop,
    .quiesce = unavailable_quiesce,
    .dispose = unavailable_dispose,
};

#endif

const audio_output_coreaudio_facade *
audio_output_coreaudio_facade_default(void)
{
    return &default_facade;
}

#include <stdint.h>
#include <stdlib.h>

#include "player.h"
#include "playback_legacy_mixer.h"

extern struct Audio audioData[8];

#define MIX_CLOCK 357955U
#define FRACTION_BITS 14U

static int32_t sample_at(const struct Audio *audio, uint32_t position)
{
    uint32_t index = position >> FRACTION_BITS;
    int32_t first = audio->sbeg[index];
    int32_t next = index + 1U < audio->slen ? audio->sbeg[index + 1U]
                                             : audio->SampleStart[0];
    return first + ((next - first) * (int32_t)(position & 0x3FFFU) >> FRACTION_BITS);
}

static void mix_voice(struct Audio *audio, size_t frames, int32_t *lane)
{
    uint32_t position = audio->pos;
    uint32_t delta = audio->delta;
    uint32_t length;

    if (audio->sbeg == NULL || (audio->mode & 1U) == 0 || audio->slen == 0 ||
        audio->vol == 0) {
        return;
    }
    if (audio->vol > 0x40U) {
        audio->vol = 0x40U;
    }
    if ((audio->mode & 3U) == 1U) {
        audio->sbeg = audio->SampleStart;
        audio->slen = audio->SampleLength;
        position = 0;
        audio->mode |= 2U;
    }
    length = (uint32_t)audio->slen << FRACTION_BITS;
    for (size_t index = 0; index < frames; ++index) {
        lane[index] += sample_at(audio, position) * audio->vol;
        position += delta;
        if (position >= length) {
            position -= length;
            audio->sbeg = audio->SampleStart;
            length = (uint32_t)(audio->slen = audio->SampleLength) << FRACTION_BITS;
            if (length < 0x10000U || audio->loop == NULL || !audio->loop(audio)) {
                audio->slen = 0;
                position = 0;
                delta = 0;
                break;
            }
        }
    }
    audio->pos = position;
    audio->delta = delta;
    if (audio->mode & 4U) {
        audio->mode = 0;
    }
}

void tfmx_playback_legacy_mixer_reset(tfmx_playback_legacy_mixer *mixer)
{
    if (mixer != NULL) {
        *mixer = (tfmx_playback_legacy_mixer){ 0, 0 };
    }
}

void tfmx_playback_legacy_mixer_begin_tick(tfmx_playback_legacy_mixer *mixer,
                                           unsigned int eclocks,
                                           unsigned int output_rate)
{
    uint64_t process;

    if (mixer == NULL || output_rate == 0) {
        return;
    }
    process = (uint64_t)eclocks * (output_rate >> 1U);
    mixer->pending_frames = (size_t)(process / MIX_CLOCK);
    mixer->remainder += (unsigned int)(process % MIX_CLOCK);
    if (mixer->remainder > MIX_CLOCK) {
        ++mixer->pending_frames;
        mixer->remainder -= MIX_CLOCK;
    }
}

int tfmx_playback_legacy_mixer_render(tfmx_playback_legacy_mixer *mixer,
                                      unsigned char *output, size_t capacity,
                                      size_t *bytes_written)
{
    size_t frames;
    int32_t *left;
    int32_t *right;

    if (mixer == NULL || output == NULL || bytes_written == NULL ||
        mixer->pending_frames == 0) {
        return 0;
    }
    frames = mixer->pending_frames;
    if (frames > (SIZE_MAX / 4U) || capacity < frames * 4U) {
        return 0;
    }
    left = calloc(frames, sizeof(*left));
    right = calloc(frames, sizeof(*right));
    if (left == NULL || right == NULL) {
        free(left);
        free(right);
        return 0;
    }
    if (multimode) {
        for (unsigned int voice = 4; voice < 8; ++voice) {
            mix_voice(&audioData[voice], frames, right);
        }
    }
    mix_voice(&audioData[0], frames, right);
    mix_voice(&audioData[1], frames, left);
    mix_voice(&audioData[2], frames, left);
    if (!multimode) {
        mix_voice(&audioData[3], frames, right);
    }
    for (size_t index = 0; index < frames; ++index) {
        int32_t blended_left = (left[index] * 11 + right[index] * 5) >> 4;
        int32_t blended_right = (left[index] * 5 + right[index] * 11) >> 4;
        int16_t channels[2] = { (int16_t)blended_left, (int16_t)blended_right };
        for (unsigned int channel = 0; channel < 2; ++channel) {
            uint16_t value = (uint16_t)channels[channel];
            output[index * 4U + channel * 2U] = (unsigned char)value;
            output[index * 4U + channel * 2U + 1U] = (unsigned char)(value >> 8U);
        }
    }
    free(left);
    free(right);
    *bytes_written = frames * 4U;
    mixer->pending_frames = 0;
    return 1;
}

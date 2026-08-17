#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "player.h"
#include "tfmxsong.h"
#include "playback_legacy_bridge.h"

#define BRIDGE_EDIT_WORDS 16385

U32 editbuf[BRIDGE_EDIT_WORDS];
S8 *smplbuf;
int *patterns;
int *macros;
int multimode;
U32 outRate = 44100;
U32 stereo = 1;
char outf[PATHNAME_LENGTH];

struct Header hdr;
int startPat = -1;
int gemx;
int loops = 1;
int dangerFreakHack;
int oopsUpHack;
int monkeyHack;

static S8 *bridge_sample;

extern struct TrackManager trackManager;
extern struct Audio audioData[8];
void TfmxInit(void);
void StartSong(int song, int mode);
void tfmxIrqIn(void);

static unsigned short read_be16(const unsigned char *bytes)
{
    return (unsigned short)(((unsigned short)bytes[0] << 8) | bytes[1]);
}

static int copy_state(const unsigned char *mdat, size_t mdat_size,
                      const unsigned char *smpl, size_t smpl_size,
                      const struct tfmx_loader_metadata *metadata)
{
    size_t edit_size;
    unsigned int index;

    if (mdat_size < 0x200 || mdat_size - 0x200 > (BRIDGE_EDIT_WORDS - 1) * 4 ||
        smpl_size > (size_t)INT_MAX || metadata == NULL ||
        metadata->pattern_count > 128 || metadata->macro_count > 128) {
        return 0;
    }
    edit_size = mdat_size - 0x200;
    memset(&hdr, 0, sizeof(hdr));
    memcpy(&hdr, mdat, sizeof(hdr));
    for (index = 0; index < 32; ++index) {
        hdr.start[index] = read_be16(mdat + 0x100 + index * 2);
        hdr.end[index] = read_be16(mdat + 0x140 + index * 2);
        hdr.tempo[index] = read_be16(mdat + 0x180 + index * 2);
    }
    hdr.trackstart = (metadata->trackstart - 0x200) / 4;
    hdr.pattstart = (metadata->pattstart - 0x200) / 4;
    hdr.macrostart = (metadata->macrostart - 0x200) / 4;
    if (metadata->trackstart < 0x200 || metadata->pattstart < 0x200 ||
        metadata->macrostart < 0x200 ||
        hdr.pattstart > BRIDGE_EDIT_WORDS - metadata->pattern_count ||
        hdr.macrostart > BRIDGE_EDIT_WORDS - metadata->macro_count) {
        return 0;
    }
    memset(editbuf, 0, sizeof(editbuf));
    memcpy(editbuf, mdat + 0x200, edit_size);

    patterns = (int *)&editbuf[hdr.pattstart];
    macros = (int *)&editbuf[hdr.macrostart];
    for (index = 0; index < metadata->pattern_count; ++index) {
        patterns[index] = metadata->patterns[index];
    }
    for (index = 0; index < metadata->macro_count; ++index) {
        macros[index] = metadata->macros[index];
    }
    for (index = hdr.trackstart;
         index < (metadata->first_pattern - 0x200) / 4; ++index) {
        unsigned int offset = index * 4;
        ((U16 *)editbuf)[index * 2] = read_be16(mdat + 0x200 + offset);
        ((U16 *)editbuf)[index * 2 + 1] =
            read_be16(mdat + 0x200 + offset + 2);
    }

    bridge_sample = malloc(smpl_size);
    if (bridge_sample == NULL) {
        return 0;
    }
    memcpy(bridge_sample, smpl, smpl_size);
    smplbuf = bridge_sample;
    return 1;
}

void tfmx_playback_legacy_bridge_reset(void)
{
    free(bridge_sample);
    bridge_sample = NULL;
    smplbuf = NULL;
    patterns = NULL;
    macros = NULL;
    memset(editbuf, 0, sizeof(editbuf));
    memset(&hdr, 0, sizeof(hdr));
    trackManager.PlayerEnable = 0;
}

int tfmx_playback_legacy_bridge_tick(int *active, unsigned short *pitch,
                                     unsigned char *volume)
{
    if (active == NULL || pitch == NULL || volume == NULL) {
        return 0;
    }
    tfmxIrqIn();
    *active = audioData[0].mode != 0;
    *pitch = audioData[0].channel == NULL ? 0 : audioData[0].channel->CurPeriod;
    *volume = audioData[0].vol;
    return 1;
}

int tfmx_playback_legacy_bridge_is_complete(void)
{
    return trackManager.PlayerEnable == 0;
}

int tfmx_playback_legacy_bridge_start(const unsigned char *mdat,
                                      size_t mdat_size,
                                      const unsigned char *smpl,
                                      size_t smpl_size,
                                      const struct tfmx_loader_metadata *metadata,
                                      unsigned int subsong)
{
    tfmx_playback_legacy_bridge_reset();
    if (mdat == NULL || smpl == NULL || subsong != 0 ||
        !copy_state(mdat, mdat_size, smpl, smpl_size, metadata)) {
        return 0;
    }
    TfmxInit();
    StartSong(0, 0);
    return trackManager.PlayerEnable != 0;
}

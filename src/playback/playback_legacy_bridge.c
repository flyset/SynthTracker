#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "player.h"
#include "tfmxsong.h"
#include "playback_legacy_bridge.h"

#define BRIDGE_EDIT_WORDS 16385
#define BRIDGE_TABLE_CAPACITY 128

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
static int bridge_patterns[BRIDGE_TABLE_CAPACITY];
static int bridge_macros[BRIDGE_TABLE_CAPACITY];

extern struct TrackManager trackManager;
extern struct Audio audioData[8];
extern struct Channel channelData[16];
extern struct PatternBlock patternBlockData;
extern struct Idb idb;
extern S8 tempVol;
extern int jiffies;
extern U32 eClocks;
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
    size_t edit_words;
    unsigned int index;

    if (mdat_size < 0x200 || mdat_size - 0x200 > (BRIDGE_EDIT_WORDS - 1) * 4 ||
        smpl_size > (size_t)INT_MAX || metadata == NULL ||
        metadata->pattern_count == 0 ||
        metadata->pattern_count > BRIDGE_TABLE_CAPACITY ||
        metadata->macro_count == 0 ||
        metadata->macro_count > BRIDGE_TABLE_CAPACITY) {
        return 0;
    }
    edit_size = mdat_size - 0x200;
    edit_words = edit_size / sizeof(editbuf[0]);
    if (metadata->first_pattern <= metadata->trackstart ||
        metadata->trackstart < 0x200 || metadata->first_pattern < 0x200 ||
        (metadata->trackstart & 3) != 0 ||
        (metadata->first_pattern & 3) != 0 ||
        metadata->first_pattern > mdat_size) {
        return 0;
    }
    for (index = 0; index < metadata->pattern_count; ++index) {
        if (metadata->patterns[index] < 0 ||
            (size_t)metadata->patterns[index] >= edit_words) {
            return 0;
        }
    }
    for (index = 0; index < metadata->macro_count; ++index) {
        if (metadata->macros[index] < 0 ||
            (size_t)metadata->macros[index] >= edit_words) {
            return 0;
        }
    }
    memset(&hdr, 0, sizeof(hdr));
    memcpy(&hdr, mdat, sizeof(hdr));
    for (index = 0; index < 32; ++index) {
        hdr.start[index] = read_be16(mdat + 0x100 + index * 2);
        hdr.end[index] = read_be16(mdat + 0x140 + index * 2);
        hdr.tempo[index] = read_be16(mdat + 0x180 + index * 2);
    }
    if (hdr.end[0] >=
        (metadata->first_pattern - metadata->trackstart) / 16U) {
        return 0;
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

    for (index = 0; index < metadata->pattern_count; ++index) {
        bridge_patterns[index] = metadata->patterns[index];
    }
    for (index = 0; index < metadata->macro_count; ++index) {
        bridge_macros[index] = metadata->macros[index];
    }
    patterns = bridge_patterns;
    macros = bridge_macros;
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
    trackManager.PlayerEnable = 0;
    free(bridge_sample);
    bridge_sample = NULL;
    smplbuf = NULL;
    patterns = NULL;
    macros = NULL;
    memset(audioData, 0, sizeof(audioData));
    memset(channelData, 0, sizeof(channelData));
    memset(&patternBlockData, 0, sizeof(patternBlockData));
    memset(&trackManager, 0, sizeof(trackManager));
    memset(&idb, 0, sizeof(idb));
    tempVol = 0;
    jiffies = 0;
    multimode = 0;
    eClocks = 14318;
    startPat = -1;
    gemx = 0;
    loops = 1;
    dangerFreakHack = 0;
    oopsUpHack = 0;
    monkeyHack = 0;
    memset(editbuf, 0, sizeof(editbuf));
    memset(bridge_patterns, 0, sizeof(bridge_patterns));
    memset(bridge_macros, 0, sizeof(bridge_macros));
    memset(&hdr, 0, sizeof(hdr));
}

void tfmx_playback_legacy_bridge_set_output_rate(unsigned int output_rate_hz)
{
    if (output_rate_hz != 0) {
        outRate = output_rate_hz;
    }
}

int tfmx_playback_legacy_bridge_tick(tfmx_voice_snapshot *snapshots)
{
    unsigned int voice;

    if (snapshots == NULL) {
        return 0;
    }
    tfmxIrqIn();
    for (voice = 0; voice < 8; ++voice) {
        snapshots[voice].active = audioData[voice].mode != 0;
        snapshots[voice].pitch = audioData[voice].channel == NULL
                                     ? 0
                                     : audioData[voice].channel->CurPeriod;
        snapshots[voice].volume = audioData[voice].vol;
    }
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
    int requested_start_pat = startPat;

    tfmx_playback_legacy_bridge_reset();
    if (mdat == NULL || smpl == NULL || subsong >= 32) {
        return 0;
    }
    if (!copy_state(mdat, mdat_size, smpl, smpl_size, metadata)) {
        tfmx_playback_legacy_bridge_reset();
        return 0;
    }
    if (hdr.start[subsong] > hdr.end[subsong] ||
        hdr.end[subsong] >=
            (metadata->first_pattern - metadata->trackstart) / 16U ||
        (requested_start_pat >= 0 &&
         ((unsigned int)requested_start_pat < hdr.start[subsong] ||
          (unsigned int)requested_start_pat > hdr.end[subsong]))) {
        tfmx_playback_legacy_bridge_reset();
        return 0;
    }
    TfmxInit();
    if (requested_start_pat >= 0) {
        startPat = requested_start_pat;
    }
    StartSong((int)subsong, 0);
    if (trackManager.PlayerEnable == 0) {
        tfmx_playback_legacy_bridge_reset();
        return 0;
    }
    return 1;
}

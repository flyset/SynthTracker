#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tfmx_loader.h"

static unsigned short read_be16(const unsigned char *bytes)
{
    return (unsigned short)(((unsigned short)bytes[0] << 8) | bytes[1]);
}

static unsigned int read_be32(const unsigned char *bytes)
{
    return ((unsigned int)bytes[0] << 24) | ((unsigned int)bytes[1] << 16) |
           ((unsigned int)bytes[2] << 8) | bytes[3];
}

static int range_is_inside(size_t size, unsigned int offset, size_t length)
{
    return (size_t)offset <= size && length <= size - (size_t)offset;
}

static int valid_mdat(const unsigned char *data, size_t size,
                      size_t smpl_size, struct tfmx_loader_metadata *metadata)
{
    unsigned int trackstart;
    unsigned int pattstart;
    unsigned int macrostart;
    unsigned int pattern;
    unsigned int macro;
    unsigned int first_macro;
    unsigned int end;
    unsigned int sample_start;
    unsigned int sample_length;
    int finite_loop_layout;
    int envelope_tempo_layout;
    int voices_01_layout;

    if (size < 0x200 || memcmp(data, "TFMX", 4) != 0) {
        return 0;
    }
    end = read_be16(data + 0x140);
    trackstart = read_be32(data + 0x1d0);
    pattstart = read_be32(data + 0x1d4);
    macrostart = read_be32(data + 0x1d8);
    finite_loop_layout = trackstart == 0x240 && pattstart == 0x220 &&
                         macrostart == 0x228 && read_be16(data + 0x180) != 2;
    envelope_tempo_layout = size == 0x2a0 && smpl_size == 2 &&
                            read_be16(data + 0x180) == 2 &&
                            trackstart == 0x240 && pattstart == 0x220 &&
                            macrostart == 0x228;
    voices_01_layout = size == 0x2b8 && smpl_size == 2 &&
                       read_be16(data + 0x180) == 6 &&
                       trackstart == 0x240 && pattstart == 0x220 &&
                       macrostart == 0x228;
    if (read_be16(data + 0x100) > end || end > 0x7fff ||
        trackstart < 0x200 || pattstart < 0x200 || macrostart < 0x200 ||
        (trackstart & 3) != 0 || (pattstart & 3) != 0 ||
        (macrostart & 3) != 0 || !range_is_inside(size, trackstart, 16) ||
         !range_is_inside(size, pattstart, voices_01_layout ? 8 : 4) ||
         !range_is_inside(size, macrostart, voices_01_layout ? 8 : 4)) {
        return 0;
    }
    if (end > (size - (size_t)trackstart) / 16 - 1) {
        return 0;
    }
    pattern = read_be32(data + pattstart);
    macro = read_be32(data + macrostart);
    first_macro = macro;
    if (pattern < 0x200 || macro < 0x200 || (pattern & 3) != 0 ||
        (macro & 3) != 0 || !range_is_inside(size, pattern, 12) ||
        !range_is_inside(size, macro, 32)) {
        return 0;
    }
    /* The first trackstep binds the selected voices and inactive voices to
     * their distinct disabled channel slots; the next trackstep stops it. */
    if (read_be16(data + trackstart) != 0 ||
        read_be16(data + trackstart + 2) != (voices_01_layout ? 0x0100 : 0xfe01) ||
        read_be16(data + trackstart + 4) != 0xfe02 ||
        read_be16(data + trackstart + 6) != 0xfe03 ||
        read_be16(data + trackstart + 8) != 0xfe04 ||
        read_be16(data + trackstart + 10) != 0xfe05 ||
        read_be16(data + trackstart + 12) != 0xfe06 ||
        read_be16(data + trackstart + 14) != 0xfe07 ||
        read_be16(data + trackstart + 16) != 0xeffe) {
        return 0;
    }
    if (voices_01_layout &&
        (read_be32(data + pattstart) != 0x260 ||
         read_be32(data + pattstart + 4) != 0x26c ||
         read_be32(data + macrostart) != 0x278 ||
         read_be32(data + macrostart + 4) != 0x298 ||
         !range_is_inside(size, 0x260, 12) ||
         !range_is_inside(size, 0x26c, 12) ||
         !range_is_inside(size, 0x278, 32) ||
         !range_is_inside(size, 0x298, 32) ||
         read_be32(data + 0x260) != 0x80000002 ||
         read_be32(data + 0x264) != 0xf3010000 ||
         read_be32(data + 0x268) != 0xf0000000 ||
         read_be32(data + 0x26c) != 0x81010102 ||
         read_be32(data + 0x270) != 0xf3010000 ||
         read_be32(data + 0x274) != 0xf0000000 ||
         read_be32(data + 0x278) != 0x09000000 ||
         read_be32(data + 0x27c) != 0x02000000 ||
         read_be32(data + 0x280) != 0x03000002 ||
         read_be32(data + 0x284) != 0x0e000012 ||
         read_be32(data + 0x288) != 0x01010000 ||
         read_be32(data + 0x28c) != 0x04000020 ||
         read_be32(data + 0x290) != 0x13000000 ||
         read_be32(data + 0x294) != 0x07000000 ||
         read_be32(data + 0x298) != 0x09010000 ||
         read_be32(data + 0x29c) != 0x02000000 ||
         read_be32(data + 0x2a0) != 0x03000002 ||
         read_be32(data + 0x2a4) != 0x0e00001e ||
         read_be32(data + 0x2a8) != 0x01010000 ||
         read_be32(data + 0x2ac) != 0x04000020 ||
         read_be32(data + 0x2b0) != 0x13000000 ||
         read_be32(data + 0x2b4) != 0x07000000 ||
         data[0x27d] != 0 || data[0x27e] != 0 || data[0x27f] != 0 ||
         data[0x29d] != 0 || data[0x29e] != 0 || data[0x29f] != 0 ||
         read_be16(data + trackstart + 18) != 0 ||
         read_be16(data + 0x282) != 2 ||
         read_be16(data + 0x2a2) != 2)) {
        return 0;
    }
    if (voices_01_layout) {
        memset(metadata, 0, sizeof(*metadata));
        metadata->patterns[0] = (0x260 - 0x200) / 4;
        metadata->patterns[1] = (0x26c - 0x200) / 4;
        metadata->macros[0] = (0x278 - 0x200) / 4;
        metadata->macros[1] = (0x298 - 0x200) / 4;
        metadata->pattern_count = 2;
        metadata->macro_count = 2;
        metadata->trackstart = trackstart;
        metadata->first_pattern = 0x260;
        metadata->pattstart = pattstart;
        metadata->macrostart = macrostart;
        return 1;
    }
    if (read_be16(data + trackstart + 18) != 0) {
        return 0;
    }
    if (!finite_loop_layout && !envelope_tempo_layout &&
        (pattern <= trackstart ||
         read_be32(data + pattern) != 0x80000001 ||
         read_be32(data + pattern + 4) != 0xf3010000 ||
         read_be32(data + pattern + 8) != 0xf0000000 ||
        read_be32(data + macro) != 0x09000000 ||
        read_be32(data + macro + 4) != 0x02000000 ||
        read_be32(data + macro + 8) != 0x03000002 ||
        read_be32(data + macro + 12) != 0x0e00000f ||
        read_be32(data + macro + 16) != 0x01010000 ||
        read_be32(data + macro + 20) != 0x04000001 ||
        read_be32(data + macro + 24) != 0x13000000 ||
        read_be32(data + macro + 28) != 0x07000000)) {
        return 0;
    }
    if (finite_loop_layout &&
        (pattern <= trackstart || !range_is_inside(size, pattern, 16) ||
         read_be32(data + pattern) != 0x80000001 ||
         read_be32(data + pattern + 4) != 0xf3010000 ||
         read_be32(data + pattern + 8) != 0xf1020000 ||
         read_be32(data + pattern + 12) != 0xf0000000 ||
         read_be32(data + macro) != 0x09000000 ||
         read_be32(data + macro + 4) != 0x02000000 ||
         read_be32(data + macro + 8) != 0x03000002 ||
         read_be32(data + macro + 12) != 0x0e00000f ||
         read_be32(data + macro + 16) != 0x01010000 ||
         read_be32(data + macro + 20) != 0x04000001 ||
         read_be32(data + macro + 24) != 0x13000000 ||
         read_be32(data + macro + 28) != 0x07000000)) {
        return 0;
    }
    if (envelope_tempo_layout &&
        (read_be32(data + pattstart) != 0x00000260 ||
         read_be32(data + macrostart) != 0x00000280 ||
         pattern != 0x260 || macro != 0x280 || pattern <= trackstart ||
         !range_is_inside(size, pattern, 20) ||
         read_be32(data + pattern) != 0x80000001 ||
         read_be32(data + pattern + 4) != 0xf3010000 ||
         read_be32(data + pattern + 8) != 0xf7030003 ||
         read_be32(data + pattern + 12) != 0xf3010000 ||
         read_be32(data + pattern + 16) != 0xf0000000 ||
         read_be32(data + macro) != 0x09000000 ||
         read_be32(data + macro + 4) != 0x02000000 ||
         read_be32(data + macro + 8) != 0x03000002 ||
         read_be32(data + macro + 12) != 0x0e00000f ||
         read_be32(data + macro + 16) != 0x01010000 ||
         read_be32(data + macro + 20) != 0x04000010 ||
         read_be32(data + macro + 24) != 0x13000000 ||
         read_be32(data + macro + 28) != 0x07000000)) {
        return 0;
    }
    memset(metadata, 0, sizeof(*metadata));
    if (!range_is_inside(size, pattstart, 4) ||
        !range_is_inside(size, macrostart, 4)) {
        return 0;
    }
    if (pattern < 0x200 || macro < 0x200 || (pattern & 3) != 0 ||
        (macro & 3) != 0 || !range_is_inside(size, pattern, 4) ||
        !range_is_inside(size, macro, 4)) {
        return 0;
    }
    metadata->patterns[0] = (int)((pattern - 0x200) / 4);
    metadata->macros[0] = (int)((macro - 0x200) / 4);
    metadata->pattern_count = 1;
    metadata->macro_count = 1;
    metadata->trackstart = trackstart;
    metadata->first_pattern = read_be32(data + pattstart);
    metadata->pattstart = pattstart;
    metadata->macrostart = macrostart;

    sample_start = ((unsigned int)data[first_macro + 5] << 16) |
                   ((unsigned int)data[first_macro + 6] << 8) |
                   data[first_macro + 7];
    sample_length = read_be16(data + first_macro + 8 + 2);
    return sample_start <= smpl_size && sample_length <= smpl_size - sample_start;
}

static tfmx_load_status read_file(const char *path, unsigned char **data,
                                  size_t *size)
{
    FILE *file;
    long length;
    unsigned char *contents;

    file = fopen(path, "rb");
    if (file == NULL) {
        return TFMX_LOAD_IO_ERROR;
    }
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return TFMX_LOAD_IO_ERROR;
    }
    if (length == 0) {
        fclose(file);
        return TFMX_LOAD_INVALID_FORMAT;
    }

    contents = malloc((size_t)length);
    if (contents == NULL) {
        fclose(file);
        return TFMX_LOAD_OUT_OF_MEMORY;
    }
    if (fread(contents, 1, (size_t)length, file) != (size_t)length ||
        ferror(file) != 0) {
        free(contents);
        fclose(file);
        return TFMX_LOAD_IO_ERROR;
    }
    fclose(file);
    *data = contents;
    *size = (size_t)length;
    return TFMX_LOAD_SUCCESS;
}

tfmx_load_status tfmx_loader_read(const char *mdat_path,
                                  const char *smpl_path,
                                  tfmx_loader_candidate *candidate)
{
    tfmx_load_status status;

    if (mdat_path == NULL || smpl_path == NULL || candidate == NULL) {
        return TFMX_LOAD_INVALID_ARGUMENT;
    }
    memset(candidate, 0, sizeof(*candidate));

    status = read_file(mdat_path, &candidate->mdat, &candidate->mdat_size);
    if (status != TFMX_LOAD_SUCCESS) {
        return status;
    }
    status = read_file(smpl_path, &candidate->smpl, &candidate->smpl_size);
    if (status != TFMX_LOAD_SUCCESS) {
        tfmx_loader_candidate_dispose(candidate);
        return status;
    }
    if (!valid_mdat(candidate->mdat, candidate->mdat_size,
                    candidate->smpl_size, &candidate->metadata) ||
        candidate->smpl_size < 2 ||
        (candidate->smpl[0] == 0 && candidate->smpl[1] == 0)) {
        tfmx_loader_candidate_dispose(candidate);
        return TFMX_LOAD_INVALID_FORMAT;
    }
    return TFMX_LOAD_SUCCESS;
}

void tfmx_loader_candidate_dispose(tfmx_loader_candidate *candidate)
{
    if (candidate == NULL) {
        return;
    }
    free(candidate->mdat);
    free(candidate->smpl);
    memset(candidate, 0, sizeof(*candidate));
}

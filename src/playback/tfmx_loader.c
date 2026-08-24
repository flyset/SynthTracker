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

static int range_is_inside(size_t size, size_t offset, size_t length)
{
    return offset <= size && length <= size - offset;
}

enum { TFMX_LOADER_TABLE_CAPACITY = 128 };

static unsigned int scan_pointer_table(
    const unsigned char *data, size_t size, unsigned int table_start,
    int normalized[TFMX_LOADER_TABLE_CAPACITY])
{
    unsigned int count = 0;

    while (count < TFMX_LOADER_TABLE_CAPACITY) {
        size_t cell_offset = (size_t)table_start + (size_t)count * 4U;
        unsigned int target;

        if (!range_is_inside(size, cell_offset, 4)) {
            break;
        }
        target = read_be32(data + cell_offset);
        if (target == 0 || target < 0x200 || (target & 3) != 0 ||
            !range_is_inside(size, target, 4)) {
            break;
        }
        normalized[count] = (int)((target - 0x200) / 4);
        ++count;
    }
    return count;
}

static int valid_mdat(const unsigned char *data, size_t size,
                      struct tfmx_loader_metadata *metadata)
{
    unsigned int trackstart;
    unsigned int pattstart;
    unsigned int macrostart;
    unsigned int end;
    unsigned int pattern_count;
    unsigned int macro_count;

    if (size < 0x200 || memcmp(data, "TFMX", 4) != 0) {
        return 0;
    }
    end = read_be16(data + 0x140);
    trackstart = read_be32(data + 0x1d0);
    pattstart = read_be32(data + 0x1d4);
    macrostart = read_be32(data + 0x1d8);
    if (trackstart == 0) {
        trackstart = 0x800;
    }
    if (pattstart == 0) {
        pattstart = 0x400;
    }
    if (macrostart == 0) {
        macrostart = 0x600;
    }
    if (read_be16(data + 0x100) > end || end > 0x7fff ||
        trackstart < 0x200 || pattstart < 0x200 || macrostart < 0x200 ||
        (trackstart & 3) != 0 || (pattstart & 3) != 0 ||
        (macrostart & 3) != 0 || !range_is_inside(size, trackstart, 16) ||
        !range_is_inside(size, pattstart, 4) ||
        !range_is_inside(size, macrostart, 4)) {
        return 0;
    }
    if (end > (size - (size_t)trackstart) / 16 - 1) {
        return 0;
    }

    memset(metadata, 0, sizeof(*metadata));
    pattern_count = scan_pointer_table(data, size, pattstart, metadata->patterns);
    macro_count = scan_pointer_table(data, size, macrostart, metadata->macros);
    if (pattern_count == 0 || macro_count == 0) {
        return 0;
    }

    metadata->pattern_count = pattern_count;
    metadata->macro_count = macro_count;
    metadata->trackstart = trackstart;
    metadata->first_pattern =
        0x200U + (unsigned int)metadata->patterns[0] * 4U;
    metadata->pattstart = pattstart;
    metadata->macrostart = macrostart;
    if (metadata->first_pattern <= trackstart) {
        return 0;
    }
    if (end >= (metadata->first_pattern - trackstart) / 16U) {
        return 0;
    }
    return 1;
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
    if (!valid_mdat(candidate->mdat, candidate->mdat_size, &candidate->metadata) ||
        candidate->smpl_size < 2) {
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

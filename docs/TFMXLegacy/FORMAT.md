# TFMX Module File Format

This document describes the on-disk layout of a TFMX module as read by the
legacy player in this repository. All byte offsets are relative to the start
of the data file being read (the `mdat` part in the dual-file case). Every
multibyte value is stored big-endian and converted with `ntohs`/`ntohl`
(`src/tfmx.c`, `src/player.c`). Evidence: `include/tfmxsong.h`
(`struct Header`), `src/tfmx.c` (`load_tfmx`, `tfmxtest`).

## File variants

The player distinguishes several physical layouts ([observed],
`src/tfmx.c:main` argument handling, `load_tfmx`):

| Variant | Magic / prefix | Layout |
|---------|---------------|--------|
| Dual-file (default) | `mdat.*` + `smpl.*` | Two files: the `mdat` file holds header + note data; the `smpl` file holds sample data. The `smpl` name is derived from `mdat` (`src/tfmx.c:727`). |
| Dual-file, DOS extensions | `*.tfx` + `*.sam` | Same two-file split; `.tfx` → `.sam` name conversion (`src/tfmx.c:737`). |
| Single-file | `tfmx.*` prefix | Header, note data, and samples in one file. The loader has a path for this (`singleFile=1`, `src/tfmx.c:719`), but the TFHD sub-header parsing that would populate the sample offsets is disabled in this snapshot; see the "Single-file (TFHD) gap" section below. |

Accepted header magics in the note-data file ([observed], `src/tfmx.c:259`):
`"TFMX-SONG"`, `"TFMX_SONG"`, `"TFMXSONG"` (first 8 chars, case-insensitive),
and `"TFMX"` (first 4 chars).

## Header layout (512 bytes)

`struct Header` (`include/tfmxsong.h:6`) is exactly 512 bytes (0x200). The
field offsets follow from the struct layout; the loader's byte-sniffing code
in `tfmxtest()` reads several of these offsets directly
(`src/tfmx.c:99`–`167`).

| Offset | Size | Field | Meaning |
|--------|------|-------|---------|
| 0x000 | 10 | `magic` | Format magic string (see above). |
| 0x00A | 6 | `pad` | Padding. |
| 0x010 | 240 | `text[6][40]` | Six 40-character text lines (song/credits text; printed by `-i`, `src/tfmx.c:783`). |
| 0x100 | 64 | `start[32]` | Subsong start positions (trackstep numbers), big-endian `uint16`. |
| 0x140 | 64 | `end[32]` | Subsong end positions (trackstep numbers), big-endian `uint16`. |
| 0x180 | 64 | `tempo[32]` | Per-subsong tempo values, big-endian `uint16` (see `PLAYER.md`). |
| 0x1C0 | 16 | `mute[8]` | Mute flags (`int16` each; not used by the interpreter in this snapshot). |
| 0x1D0 | 4 | `trackstart` | File offset of the trackstep table. |
| 0x1D4 | 4 | `pattstart` | File offset of the pattern pointer table. |
| 0x1D8 | 4 | `macrostart` | File offset of the macro pointer table. |
| 0x1DC | 36 | `pad2` | Padding. |

The byte-sniffing code reads fixed byte patterns in the header padding
region (0x0E/0x0F), the subsong end table at 0x140, the trackstart field at
0x1D0 (decimal 464), and the first data words at 0x200
(`src/tfmx.c:129`–`153`), consistent with the struct layout above. The
"TFMX 1.5" test additionally requires the trackstart field to be zero
(`src/tfmx.c:129`).

## Pointer fields

`trackstart`, `pattstart`, and `macrostart` are stored as big-endian
`uint32` file offsets into the note-data region (relative to the start of
the `mdat` data, which begins after the 512-byte header). The loader converts
them to array indices into the in-memory `editbuf` word array
(`src/tfmx.c:283`–`296`):

```
editbuf index = (stored_offset - 0x200) >> 2
```

A stored value of 0 selects a default index instead of the formula
([observed], `src/tfmx.c:283`–`296`):

| Field | Default `editbuf` index | Default file offset |
|-------|-------------------------|---------------------|
| `trackstart` | 0x180 (384 words) | 0x800 |
| `pattstart` | 0x80 (128 words) | 0x400 |
| `macrostart` | 0x100 (256 words) | 0x600 |

These defaults match the "unpacked" trackstep offset `s = 0x00000800` used
by the sniffing code (`src/tfmx.c:163`).

## Data tables

The note-data region holds three kinds of tables ([observed],
`src/tfmx.c:303`–`347`):

- **Pattern pointer table** at `pattstart`: up to 128 big-endian `uint32`
  entries. Entry `i` is the file offset of pattern `i`'s data; each entry is
  converted in place to an `editbuf` index: `(offset - 0x200) >> 2`, stored
  into `patterns[i]`. The loop stops early when the offset is not 4-byte
  aligned or points beyond the loaded data (`src/tfmx.c:329`–`336`).
  `num_pat` counts the valid entries.
- **Macro pointer table** at `macrostart`: same structure and conversion,
  yielding `macros[i]` and `num_mac` (`src/tfmx.c:319`–`326`).
- **Trackstep table** at `trackstart`: an array of tracksteps, each 4 words
  (16 bytes) = 8 big-endian `uint16` values. The table spans from
  `trackstart` up to the first pattern's data:
  `num_ts = (patterns[0] - trackstart) >> 2` (`src/tfmx.c:339`–`347`).
  Trackstep words are byte-swapped in place during load.

Tracksteps are indexed by subsong start/end values: a subsong's trackstep
range is `start[song]`..`end[song]` (see `PLAYER.md`). The sniffing code
computes trackstep byte addresses as `start[song] * 16 + s`
(`src/tfmx.c:171`–`172`), consistent with 16 bytes per trackstep.

## Pattern and macro data

Pattern and macro data are arrays of 32-bit words whose high byte is the
opcode; the words are converted from big-endian at use time with `ntohl`
(`src/player.c:141` in `RunMacro`, `src/player.c:907` in `DoTrack`). The
loader does not decode them up front ([observed], comment at
`src/tfmx.c:813`–`814`). See `PATTERNS.md` and `MACROS.md` for the opcode
sets.

## Sample data

In the dual-file layout the `smpl` file is read whole into `smplbuf`
(`src/tfmx.c:377`–`402`). Sample addresses inside macros are indices into
`smplbuf` (see `MACROS.md`). In the single-file layout the sample region
would start at `nTFhd_offset + nTFhd_mdatsize` (`src/tfmx.c:355`); see the
gap below.

## Loader in-place mutation

The loader mutates the loaded data in place ([observed]):
- `start`/`end`/`tempo` are byte-swapped in place (`src/tfmx.c:303`–`308`).
- Pointer-table entries are replaced by `editbuf` indices
  (`src/tfmx.c:319`–`337`).
- Trackstep words are byte-swapped in place (`src/tfmx.c:339`–`347`).

`docs/ARCHITECTURE.md` ("Module data is mutated in place on load") reflects
this behavior.

## Format detection strings

`tfmxtest()` (`src/tfmx.c:94`) is a byte-sniffing detector contributed from
UADE (per `README_LEGACY`). It assigns the following labels; note that this
function is **not called anywhere** in the current snapshot ([observed],
verified by search), so the labels are descriptive of the sniffing logic only:

| Detected bytes | Label assigned | Meaning per code comment |
|----------------|----------------|--------------------------|
| `TFHD` + byte 0x08 == 0x01 | `TFHD1.5` | "One File TFMX format" |
| `TFHD` + byte 0x08 == 0x02 | `TFHDPro` | One-file format |
| `TFHD` + byte 0x08 == 0x03 | `TFHD7V` | One-file format |
| `TFMX`/`tfmx` (default) | `MDAT` | "default TFMX: TFMX Pro" |
| `TFMX` + title pattern + constraints | `TFMX1.5` | "TFMX 1.0 - 1.6" |
| `TFMX` + specific BMWi/B.C. Kid byte patterns | `TFMX7V` | "special cases TFMX 7V" |
| `TFMX` + trackstep scan for `EFFE 0003 FF00 0000` | `TFMX7V` | "TFMX 7V" |

The 7V detection scans the subsong trackstep ranges for an `0xEFFE` step
whose control word is `0x0003` (timeshare; see `PLAYER.md`) with `l[2] ==
0xFF00` and the high byte of `l[3]` zero (`src/tfmx.c:174`–`183`). The
labels are heuristic and [unverified] against any authoritative format
registry.

## Single-file (TFHD) gap

The globals `nTFhd_offset`, `nTFhd_mdatsize`, and `nTFhd_smplsize`
(`src/tfmx.c:50`–`52`) are only ever read, never assigned: the function that
would parse the TFHD sub-header (`check_md5_and_headers`) is commented out
(`src/tfmx.c:21`, `src/tfmx.c:760`), and the MD5-based per-song hack
auto-detection is likewise commented out (`src/tfmx.c:28`–`38`). As a
consequence, single-file playback cannot position the sample region
correctly in this snapshot. This is an implementation-state observation, not
a statement about the TFHD format itself.

# The Audio Path

This document describes how per-voice state becomes output samples, as
implemented in `src/audio.c`. Evidence: `src/audio.c` (mixing, filter,
stereo blend, ring buffer, SDL callback, timing conversion),
`include/player.h` (`struct Audio`), `src/player.c` (voice update in
`DoMacro`), `README_LEGACY` (feature history).

## Output platform

The audio path uses the SDL 1.x-era API: `SDL_OpenAudio`,
`SDL_PauseAudio`, `SDL_MixAudio` (`src/audio.c:397`–`429`, `577`).
`README.md` describes the dependency as SDL 1.1.7; `CMakeLists.txt`
configures SDL2 via `sdl-config` ([discrepancy], see `PROVENANCE.md`).
Thread synchronization uses a pthread mutex/condition pair
(`src/audio.c:62`–`63`, `fill_audio`).

## Ring buffer

- `BUFSIZE` = 131072 × 4 = 524,288 bytes; `HALFBUFSIZE` = 65536 × 4 =
  262,144 (`src/audio.c:25`–`26`).
- A producer thread (the main loop) fills `tbuf[]` and converts into the
  byte ring `buf[]`; the SDL callback (`fill_audio`) consumes it with
  `SDL_MixAudio` and signals the condition (`src/audio.c:556`–`588`).
- The converter aborts if the ring would overflow
  (`src/audio.c:161`–`163`, `207`–`209`).

## Voice state and per-tick update

Each of the 8 voices carries a `struct Audio` (`include/player.h:6`):
`pos` (fixed-point position), `delta` (per-sample increment), `slen` /
`SampleLength` (current / nominal length), `sbeg` / `SampleStart`
(current / nominal start), `vol`, `mode`, and `loop` (function pointer).
The interpreter refreshes these each tick in `DoMacro`
(`src/player.c:655`–`663`):

- `delta = CurPeriod ? (3579545 << 9) / (CurPeriod * outRate >> 5) : 0`
  (`src/player.c:655`).
- `SampleStart = &smplbuf[SaveAddr]`;
  `SampleLength = SaveLen ? SaveLen << 1 : 65535`.
- If `(mode & 3) == 1`, `sbeg`/`slen` are re-synced to the sample start.
- `vol = (CurVol * MasterVol) >> 6`.

`3579545` is a fixed constant in the delta formula; its physical meaning is
**[unverified]** (it is the well-known PAL color-clock figure, but the code
does not say so).

## Mixing

Two mixers exist (`src/audio.c:248`, `296`):

- `mix_add` — nearest-sample mixing at 14-bit fixed-point position
  (`ps += d`; sample index `ps >> 14`).
- `mix_add_ov` — linear interpolation between adjacent samples using the
  14-bit fraction (`FRACTION_BITS = 14`; `FRACTION_MASK`), with wrap-around
  reading the first sample of the loop.

Selection: the global `over` starts `-1` (`src/tfmx.c:604`), which selects
`mix_add_ov`; `-v` sets `over = 0`, selecting `mix_add`
(`src/audio.c:389`–`393`). `README_LEGACY` credits the oversampling code to
Peter Schlaile.

Mixer guards: the voice is skipped when `sbeg` points at the null dummy,
`(mode & 1) == 0`, or `slen == 0`; `vol` is clamped at `0x40`; a
`(mode & 3) == 1` state re-arms the sample (`src/audio.c:253`–`262`).

**Sample looping:** on position wrap, the mixer reloads `l = slen =
SampleLength << 14` and calls `audio->loop(audio)`. The sample stops (all
state zeroed) when `SampleLength < 4` (`l < 0x10000`) or the loop function
returns 0 (`src/audio.c:281`–`285`, `337`–`341`). The loop functions
(`src/player.c:120`–`132`):

- `LoopOff` returns 1 — normal looping continues.
- `LoopOn` (set by the "wait on DMA" macro opcode) returns 1 while
  `WaitDMACount` counts down, then installs `LoopOff` and resumes macro
  stepping (`MacroRun = 0xFF`).

After the mixer, `mode & 4` (set by DMA-off with a flag) clears `mode` to 0
(`src/audio.c:291`–`293`, `349`–`351`), which halts the voice on the next
tick's re-sync.

## Channel routing and panning

`mixit` (`src/audio.c:356`–`385`) routes voices to the two mixer lanes
`tbuf[0..]` and `tbuf[HALFBUFSIZE..]`:

- Non-multimode: channel 3 → lane 0; channels 0 → lane 0; channels 1–2 →
  lane 1.
- Multimode: channels 4–7 → lane 0 (with the lane-1 side hard-clipped to
  ±16383, `src/audio.c:369`–`372`), then channels 0–2 as above.

In stereo conversion, lane 1 is emitted first, then lane 0
(`src/audio.c:218`–`220`, `172`–`174`), so lane 1 is the left channel of
the interleaved stream [inferred]. In mono mode the two lanes are mixed
down: `(L + R) >> 1` for 16-bit, `((L + R) >> 9) ^ 0x80` for 8-bit
(`src/audio.c:226`–`231`, `180`–`186`).

`act[8]` gates which channels are mixed (`src/audio.c:363`–`381`); the `-V`
option disables channels by name (`src/tfmx.c:662`).

## Stereo blend

`blend` defaults to 1 ("headphone" mode). `main` maps `-b`:
`if (blend) stereo = 1; blend &= 1;` (`src/tfmx.c:775`–`776`), so `-b 1` =
stereo with blend, `-b 2` = full stereo without blend, `-b 0` = mono.
`stereoblend` (`src/audio.c:138`) mixes the two lanes with an 11/5 weighted
split:

```
y  = (L * 11 + R * 5) >> 4
R' = (L * 5  + R * 11) >> 4
```

## Low-pass filter

`filter` (`src/audio.c:105`) is a weighted-sum one-pole low-pass applied
per lane before blending. Settings (`-w`):

| `filt` | New/old weighting | Character |
|--------|-------------------|-----------|
| 0 | — (off) | none |
| 1 | `(3*new + old) >> 2` | highest cutoff ("high") |
| 2 | `(new + old) >> 1` | medium |
| 3 | `(new + old*3) >> 2` | lowest cutoff ("low") |

The default is off: `filt = 0` is set in `main` (`src/tfmx.c:605`);
`README_LEGACY` describes the settings as off/high/medium/low and notes the
exact cutoff ratios were not documented by the author.

## Format conversion and output

- `conv_s16` (default) or `conv_u8` (`-8`); mono sums as above
  (`src/audio.c:152`–`241`).
- Device output: `SDL_OpenAudio` with `wanted.freq = outRate`
  (default 44100, `-f`), 16-bit (or 8-bit) samples, mono/stereo per `-b`
  (`src/audio.c:397`–`429`).
- File output: `-o` writes raw audio to a file (`open_sndfile`,
  `write_output`).
- `multiplier`/`blocksize` computations size the ring writes
  (`src/audio.c:421`–`422`, `445`–`459`); `MULTIPLIER_DEFAULT_VALUE = 2`.

## Tick-to-samples conversion

`calculateSamplesToProcess` (`src/audio.c:507`) converts the `eClocks`
clock (see `PLAYER.md`) into output samples per tick:

```
process = eClocks * (outRate >> 1)
eRem    += process % 357955
samples  = process / 357955   (+1 when eRem exceeds 357955)
```

With the default `eClocks = 14318` and `outRate = 44100` this yields ≈ 882
samples per tick (≈ 50 ticks/second), matching the interpreter's
50 Hz-style cadence ([inferred]; see `PLAYER.md`).

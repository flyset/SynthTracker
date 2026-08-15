# The Interpreter and Sequencer

This document describes how the legacy player drives the module: the
per-tick entry point, the trackstep table, the pattern sequencer, the timing
model, song start, multimode, per-song hacks, and the SFX lock mechanism.
Evidence: `src/player.c` (`tfmxIrqIn`, `DoAllMacros`, `DoMacro`, `DoTracks`,
`DoTrack`, `GetTrackStep`, `StartSong`, `TfmxInit`, `ChannelOff`, `DoFade`),
`include/player.h` (`struct Channel`, `struct TrackManager`,
`struct PatternBlock`, `struct Pattern`).

## Voices and channels

Eight voices are driven: `channelData[0..7]` with `audioData[0..7]`
(`src/player.c:18`–`20`; `channelData` is declared as size 16 but only
indices 0–7 are used). `DoAllMacros` always steps channels 0–3; channels
4–7 step only in multimode (`src/player.c:666`–`681`). The audio mixer
routes channels by the same rule (`AUDIO.md`). The `act[8]` array
(`src/audio.c:15`) toggles which channels are mixed; the `-V` option clears
flags by character (`src/tfmx.c:662`).

## Per-tick entry point

`tfmxIrqIn` (`src/player.c:1065`) is the per-tick heartbeat:

1. If `PlayerEnable` is clear, return immediately.
2. `DoAllMacros()` — step all macros and apply effects.
3. If `CurrSong >= 0`, `DoTracks()` — step the sequencer.

Each call also produces audio for one tick (the mixer derives the sample
count from the timing model; see `AUDIO.md`). `jiffies` counts ticks
(`src/player.c:30`, incremented in `DoTracks`).

## Macro stepping (`DoMacro`, `RunMacro`)

`DoMacro(cc)` (`src/player.c:600`) per channel per tick:

- SFX lock countdown: while `SfxLockTime >= 0`, decrement it; when it goes
  negative, clear `SfxFlag`/`SfxPriority` (`src/player.c:607`–`610`). The
  `SfxCode` queue is never populated anywhere in this snapshot [observed],
  so only the lock flag path is live.
- `MacroWait` countdown; `RunMacro` is invoked when `MacroRun` is set and
  the wait expired (`src/player.c:632`–`644`).
- `DoEffects(channel)` — see `MACROS.md`.
- Per-tick voice update: `delta` from `CurPeriod`; `SampleStart`/
  `SampleLength` from `SaveAddr`/`SaveLen`; `vol = (CurVol * MasterVol) >> 6`
  (`src/player.c:655`–`663`).

`RunMacro` implements the macro opcode set (`MACROS.md`) and loops internally
until a yielding opcode.

## Trackstep table

A trackstep is 16 bytes = 8 big-endian `uint16` words (`src/player.c:732`).
Subsongs select a contiguous range of tracksteps via `start[song]`/`end[song]`
(see `FORMAT.md`); `CurrPos` walks the range. `GetTrackStep`
(`src/player.c:715`) is called when a new trackstep is needed.

**Control steps:** if word 0 is `0xEFFE`, word 1 selects the command
(`src/player.c:746`–`795`):

| `l[1]` | Command | Effect |
|--------|---------|--------|
| `0` | Stop | `PlayerEnable = 0` (end of song) — `src/player.c:749`–`751`. |
| `1` | Loop | Loop to trackstep `l[2]` with count `l[3]`; interacts with the global `loops` count and `TrackLoop` (`TrackLoop` is seeded `-1` in `StartSong`, then set to `l[3]`; the loop exits after `l[3] + 1` passes, [inferred] off-by-one as coded) — `src/player.c:752`–`770`. |
| `2` | Speed | `SpeedCnt = Prescale = l[2]`; if `(l[3] & 0xF200) == 0` and `(l[3] & 0x1FF) > 0xF`, set `eClocks = CIASave = 0x1B51F8 / (l[3] & 0x1FF)` — `src/player.c:771`–`776`. |
| `3` | Timeshare | If `(l[3] & 0x8000) == 0`: clamp `(char)l[3]` at `-0x20`, then `eClocks = CIASave = (14318 * (x + 100)) / 100` and set `multimode = 1` — `src/player.c:777`–`785`. |
| `4` | Fade | `DoFade(l[2] & 0xFF, l[3] & 0xFF)` — `src/player.c:786`–`789`. |
| other | — | Prints a diagnostic and skips to the next trackstep — `src/player.c:790`–`794`. |

Note that `multimode` is only ever set to 1 in this snapshot; the commented
`else` branch that would clear it is disabled (`src/player.c:783`).

**Normal steps:** the 8 words bind tracks to patterns
(`src/player.c:796`–`809`): for each track `x`, `PXpose = l[x] & 0xFF` and
`PNum = l[x] >> 8`; if `PNum < 0x80` the pattern is loaded
(`PAddr = patterns[PNum]`, `PStep = PWait = 0`, `PLoop = 0xFFFF`).
`PNum == 0xFE` turns the channel off (`DoTrack`, `src/player.c:884`–`890`);
`PNum >= 0x90` is ignored (`src/player.c:895`–`898`).

The `loops` global (`-l`, default 1) guards the first-trackstep check: when
`CurrPos == FirstPos` and `loops <= 0`, a negative `loops` stops playback
and zero decrements once (`src/player.c:722`–`730`). Observed behavior:
`-l 0` plays one pass and stops at the loop-back; `-l 1` repeats forever;
negative values stop immediately. The `EFFE` loop command also decrements
`loops` (`src/player.c:753`–`760`).

## Pattern sequencing (`DoTracks`, `DoTrack`)

`DoTracks` (`src/player.c:1044`) counts ticks and, every `Prescale + 1`
ticks [inferred from `if (!SpeedCnt--)`], steps all 8 tracks via
`DoTrack(&patternBlockData.p[x])`. `DoTrack` (`src/player.c:812`) honors
`PWait` countdowns, then executes the pattern word stream (`PATTERNS.md`).
An `End` (`0xF0`) command advances `CurrPos` (wrapping `LastPos` →
`FirstPos`) and re-reads the trackstep table, returning 1 to restart the
8-track pass (`src/player.c:970`–`976`).

## Timing model

Two mechanisms combine ([observed]; the intended physical interpretation is
**[inferred]**):

- **Prescale**: `Prescale` (0–15) is the number of ticks between track
  steps. Set from the song's `tempo` entry when it is below 0x10
  (`StartSong`, `src/player.c:1131`–`1132`) or from a speed control step.
- **eClocks**: a tick clock count. Default `14318` (`src/player.c:33`),
  with the code comment "assume 125bpm, NTSC timing" (`src/player.c:1122`).
  When the song `tempo` is `>= 0x10`, `eClocks = CIASave = 0x1B51F8 / tempo`
  (`src/player.c:1126`–`1130`; `0x1B51F8` = 1,790,456, which is ≈
  125 × 14,318 — consistent with the comment, [inferred]). Control steps 2
  and 3 adjust it as shown above.

The mixer converts `eClocks` to a sample count per tick
(`src/audio.c:507`–`518`):

```
samples = floor(eClocks * (outRate / 2) / 357955)   (+ accumulated remainder)
```

With the defaults (`eClocks = 14318`, `outRate = 44100`) this is ≈ 882
samples per tick, i.e. ≈ 50 ticks per second ([inferred]: consistent with a
PAL-era 50 Hz VBI assumption; the constants are **[unverified]** against an
authoritative reference).

## Song start and stop

`StartSong(song, mode)` (`src/player.c:1113`):

- Resets `PlayerEnable = 0`, `MasterVol = 0x40`, `FadeSlope = 0`,
  `TrackLoop = -1`, `PlayPattFlag = 0`, `eClocks = CIASave = 14318`.
- For `mode != 2`: `CurrPos = FirstPos = hdr.start[song]`;
  `LastPos = hdr.end[song]`; tempo handling as above.
- Clears the 8 pattern slots (`PAddr = 0`, `PNum = 0xFF`, `PXpose = 0`).
- Honors the `-P` start-position override (`startPat`).
- Sets `SpeedCnt = EndFlag = 0` and `PlayerEnable = 1`.

`AllOff` (`src/player.c:1076`) silences everything; `TfmxInit`
(`src/player.c:1099`) initializes channels and pattern slots; `ChannelOff`
(`src/player.c:683`) turns one channel off unless the SFX flag is set.

`DoFade(sp, dv)` (`src/player.c:702`) sets `FadeDest = dv`, `FadeTime =
FadeReset = sp`; if `sp == 0` or `MasterVol == sp` it applies `dv`
immediately, otherwise `FadeSlope` steps by ±1 per tick in `DoEffects`.

## Multimode (8-channel)

The timeshare control step (`EFFE` type 3) sets `multimode = 1`. In
multimode, `DoAllMacros` also steps channels 4–7, and the mixer mixes them
with the standard three (see `AUDIO.md` for routing and the right-side
clip). `NotePort` also selects channels by `b2 & (multimode ? 7 : 3)`
(`src/player.c:58`).

## Per-song behavior hacks

The player contains conditional behaviors keyed on globals
(`src/tfmx.c:40`–`43`, `81`; `docs/ARCHITECTURE.md` "Per-song behavior
hacks"):

| Flag | Switch | Effect where referenced | State in snapshot |
|------|--------|-------------------------|-------------------|
| `gemx` | `-G` | In macro `0x00`, applies a velocity-based volume formula (`src/player.c:160`–`165`); README_LEGACY notes the GemX Theme hack was "still incomplete". | Wired to `-G` only (`src/tfmx.c:649`). |
| `dangerFreakHack` | `-D` | Forces `Finetune = 0` on notes (`src/player.c:68`–`71`) and forces the sample reload path on DMA-on (`src/player.c:192`). README_LEGACY says the hack "should no longer be needed" (auto-detected by MD5 there). | Wired to `-D`; the MD5 auto-detection is commented out in this snapshot, so `-D` is manual. |
| `oopsUpHack` | — | Forces `SpeedCnt = 5` in `DoTracks` (`src/player.c:1052`–`1054`). | Declared, never set in this snapshot ([observed] search); **inert**. |
| `monkeyHack` | — | Only a debug print guarded by `== 1` (`src/tfmx.c:808`). | Declared, never set; **inert**. |
| `weirdZoutThm` | — | Declared (`src/tfmx.c:40`), no references. | **Inert**; the Z-Out issue itself is described in `README_LEGACY` (byteorder/alignment suspicion). |

`gemx` is set by `-G` (`src/tfmx.c:649`); there is no other assignment
([observed] search of `src/`).

## Notes on reliability

The code contains a comment about an intermittent state mismatch between
`MacroRun` and `MacroWait` on macOS (`src/player.c:625`–`631`), attributed
to the Z-Out theme problem. The macro wait countdown reads the pre-decrement
value (`src/player.c:632`–`641`), so waits effectively last one tick longer
than the stored count ([inferred]).

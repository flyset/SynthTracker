# Timeshare fixture layout

This document describes the self-authored fixture pair used by the focused
timeshare timing tests:

- `mdat.timeshare` — module data, stored big-endian.
- `smpl.timeshare` — two authored, nonzero sample bytes (`0x40`, `0xC0`).

The fixture is self-authored and contains no copied module or sample content
from any external TFMX module. It demonstrates the private `eClocks` handoff
from the legacy interpreter to the mixer through the private tick path: the
first bridge tick uses default timing, the second bridge tick executes the
self-authored timeshare control step (whose interpreter `eClocks` update feeds
the same rendered tick), and the third tick keeps the dynamic scaled timing on
the inert hold trackstep while the engine remains active.

## Local offsets and layout

The normalized tables are at `0x220` (pattern table) and `0x228` (macro table),
the tracksteps begin at `0x240`, the first pattern begins at `0x270`, and the
first macro begins at `0x280`. The header stores `start[0] = 0` at `0x100`,
`end[0] = 2` at `0x140`, `tempo[0] = 0` at `0x180`, and the three table
pointers at `0x1D0` (`trackstart = 0x240`), `0x1D4`
(`pattstart = 0x220`), and `0x1D8` (`macrostart = 0x228`).

| Offset | Contents |
|---|---|
| `0x220` | Pattern pointer table; entry 0 points to `0x270` |
| `0x228` | Macro pointer table; entry 0 points to `0x280` |
| `0x240` | Trackstep 0: binding step `0000 FE01 FE02 FE03 FE04 FE05 FE06 FE07` |
| `0x250` | Trackstep 1: timeshare control step `EFFE 0003 0000 0064` |
| `0x260` | Trackstep 2: inert hold step `FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00` |
| `0x270` | Pattern 0: `F3000000` (pattern wait, count zero) then `F0000000` (pattern end) |
| `0x280` | Macro 0: one `07 00 00 00` channel-macro-stop word (stops the channel macro, not the engine; never executed because pattern 0 has no note event) |

Trackstep 0 binds voice 0 to pattern 0 and leaves the other voices on their
distinct inactive bindings, matching the binding step used by the step-8
fixture. Pattern 0 contains no note event, so no macro runs and the rendered
PCM is silent; the fixture's contract is the rendered tick frame count, not
audio content.

## Control flow

These are repository-specific observations of the local player flow
(`StartSong`, `DoTracks`, `DoTrack`, and `GetTrackStep` in `src/player.c`),
not universal TFMX format claims:

1. `StartSong` establishes the default `eClocks = 14318` and consumes trackstep
   0 (the binding step). The first bridge tick therefore renders with default
   timing.
2. Pattern 0's wait command with count zero spans the first tick: it is
   processed on tick 1, so the pattern-end command executes on tick 2.
3. On tick 2 the pattern ends, advancing to trackstep 1, whose `EFFE 0003`
   timeshare control scales `eClocks` to `(14318 * (100 + 100)) / 100 =
   28636` and sets `multimode = 1`. Because the interpreter update happens
   during the bridge tick, the same tick's duration is computed from the
   updated value (interpreter update → duration calculation → mix), and the
   mixer carries the remainder from tick 1.
4. Trackstep 2 is the inert hold step: its eight `FF00` words are pattern
   numbers whose high byte is `0xFF` (≥ `0x80`), so `GetTrackStep` loads no
   pattern for any voice and runs no control action. The engine is not
   stopped, keeps playing, and remains active through the third bridge tick;
   `eClocks` stays `28636`, so the third tick stays dynamic and renders with
   the carried remainder. There is no stop step and the tests end after tick
   3.

## Focused TDD observations

The focused playback-context test ticks and renders each tick fully at
44.1 kHz through `tfmx_playback_context_tick_at_rate` and
`tfmx_playback_context_render_frames`. The verified rendered frame sequence,
with the mixer's carried remainder after each tick, is:

| Tick | Timing | Rendered frames | Carried remainder |
|---:|---|---:|---:|
| 1 | default `eClocks = 14318` | `881` | `353545` |
| 2 | same-tick post-interpreter timeshare `eClocks = 28636` | `1764` | `344725` |
| 3 | dynamic `eClocks = 28636`, engine active on the hold step | `1764` | `335905` |

The engine is not complete after any of the three rendered ticks: the fixture
contains no stop step, and tick 3 is an active, dynamic tick.

The audio-output composition test drives the same fixture through the fake
CoreAudio facade → adapter → coordinator → renderer path at 44.1 kHz and
verifies exact requested-frame delivery, retained partial ticks, the expected
tick advance sequence across the same `881`/`1764`/`1764` ticks, and that the
engine remains active (not complete) after advancing to the dynamic ticks.

These observations are limited to this self-authored fixture and the private
playback boundary (`src/playback/playback_context.c`,
`src/playback/playback_legacy_bridge.c`, `src/playback/playback_legacy_mixer.c`,
`src/playback/playback_legacy_renderer.c`). They are not a format-wide
compatibility claim: the timeshare step's `eClocks` scaling is a local
repository-specific observation of `src/player.c`, and no external module or
sample content is referenced.

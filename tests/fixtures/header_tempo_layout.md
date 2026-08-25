# Header tempo fixture layout

This document describes the self-authored fixture pair used by the focused
header-tempo timing test:

- `mdat.header_tempo` — module data, stored big-endian.
- `smpl.header_tempo` — two authored, nonzero sample bytes (`0x20`, `0xE0`).

The fixture is self-authored and contains no copied module or sample content
from any external TFMX module, and no external file paths. It demonstrates the
header-tempo timing source: the header stores `tempo[0] = 100`, so `StartSong`
derives `eClocks` before the first bridge tick, and every rendered tick uses
that header-derived duration through the existing private `eClocks` handoff.

## Local offsets and layout

The normalized pattern table is at `0x220` and the macro table at `0x228`,
the tracksteps begin at `0x240`, the first pattern begins at `0x260`, and the
first macro begins at `0x270`. The header stores `start[0] = 0` at `0x100`,
`end[0] = 1` at `0x140`, `tempo[0] = 100` (`0x64`) at `0x180`, and the three
table pointers at `0x1D0` (`trackstart = 0x240`), `0x1D4`
(`pattstart = 0x220`), and `0x1D8` (`macrostart = 0x228`). All multi-byte
header and table values are big-endian; the two sample bytes are the raw
signed sample words themselves.

| Offset | Contents |
|---|---|
| `0x220` | Pattern pointer table; entry 0 points to `0x260` |
| `0x228` | Macro pointer table; entry 0 points to `0x270` |
| `0x240` | Trackstep 0: binding step `0000 FE01 FE02 FE03 FE04 FE05 FE06 FE07` |
| `0x250` | Trackstep 1: inert hold step `FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00` |
| `0x260` | Pattern 0: `F3000000` (pattern wait, count zero) then `F0000000` (pattern end) |
| `0x270` | Macro 0: one `07 00 00 00` channel-macro-stop word (stops the channel macro, not the engine; never executed because pattern 0 has no note event) |

The header `tempo[0] = 100` is at or above `0x10`, so `StartSong` sets
`eClocks = 0x1B51F8 / 100 = 17904` and sets the prescale to `0` before any
bridge tick runs. Trackstep 0 binds voice 0 to pattern 0 and leaves the other
voices on their distinct inactive bindings, matching the binding step used by
the step-8 fixture. Pattern 0 contains no note event, so no macro runs and the
rendered PCM is silent; the fixture's contract is the rendered tick frame
count, not audio content.

## Control flow

These are repository-specific observations of the local player flow
(`StartSong`, `DoTracks`, `DoTrack`, and `GetTrackStep` in `src/player.c`),
not universal TFMX format claims:

1. `StartSong` reads `tempo[0] = 100`, which is `>= 0x10`, so it sets
   `eClocks = 0x1B51F8 / 100 = 17904` and consumes trackstep 0 (the binding
   step). The first bridge tick therefore renders with the header-derived
   timing, not the default `14318`.
2. Pattern 0's wait command with count zero spans the first tick: it is
   processed on tick 1, so the pattern-end command executes on tick 2.
3. On tick 2 the pattern ends, advancing `CurrPos` from 0 to 1 (not at
   `LastPos`), and `GetTrackStep` loads trackstep 1, the inert hold step.
4. Trackstep 1 is the inert hold step: its eight `FF00` words are pattern
   numbers whose high byte is `0xFF` (`>= 0x80`), so `GetTrackStep` loads no
   pattern for any voice and runs no control action. The engine is not
   stopped, keeps playing, and remains active through the third bridge tick;
   `eClocks` stays `17904`, so every tick renders with the header-derived
   timing and the carried remainder. There is no stop step and the test ends
   after tick 3.

## Focused TDD observations

The focused playback-context test ticks and renders each tick fully at
44.1 kHz through `tfmx_playback_context_tick_at_rate` and
`tfmx_playback_context_render_frames`, with output capacity 1103 frames. The
verified rendered frame sequence, with the mixer's carried remainder after
each tick, is:

| Tick | Timing | Rendered frames | Carried remainder |
|---:|---|---:|---:|
| 1 | header-derived `eClocks = 17904` | `1102` | `316790` |
| 2 | header-derived `eClocks = 17904` | `1103` | `275625` |
| 3 | header-derived `eClocks = 17904` | `1103` | `234460` |

The exact-N arithmetic is `process = eClocks * (rate >> 1)` =
`17904 * 22050 = 394783200`; `pending_frames = process / 357955 = 1102` with
remainder `316790`, and the mixer carries the remainder so tick 2 and tick 3
each cross the `357955` clock threshold and render `1103` frames. The engine
is not complete after any of the three rendered ticks: the fixture contains no
stop step, and every tick is an active, header-derived tick.

These observations are limited to this self-authored fixture and the private
playback boundary (`src/playback/playback_context.c`,
`src/playback/playback_legacy_bridge.c`, `src/playback/playback_legacy_mixer.c`,
`src/playback/playback_legacy_renderer.c`). They are not a format-wide
compatibility claim: the header-tempo `eClocks = 0x1B51F8 / tempo` behavior
for `tempo >= 0x10` is a local repository-specific observation of
`src/player.c` (`StartSong`), and no external module or sample content is
referenced.

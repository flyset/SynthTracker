# Header tempo + speed-control finite fixture layout

This document describes the self-authored fixture pair used by the focused
application-level audio-lifecycle test in
`tests/application/test_application_audio_lifecycle.c`:

- `mdat.header_speed_finite` — module data, stored big-endian.
- `smpl.header_speed_finite` — two authored, nonzero sample bytes (`0x30`,
  `0xD0`).

The fixture is self-authored and contains no copied module or sample content
from any external TFMX module, and no external file paths. It demonstrates
header-tempo and qualifying speed-control timing in one finite flow: header
tempo 100 is established before the first bridge tick, a qualifying speed
control (`low9` divisor 100) runs on a later bridge tick, at least one tick
renders with the retained speed-derived timing, and a later stop step
completes the song.

## Local offsets and layout

The normalized pattern table is at `0x220` (two entries plus terminator), the
macro table at `0x22C` (one entry plus terminator), the tracksteps begin at
`0x240`, pattern 0 begins at `0x280`, pattern 1 begins at `0x290`, and macro 0
begins at `0x2A0`. The header stores `start[0] = 0` at `0x100`, `end[0] = 3`
at `0x140`, `tempo[0] = 100` (`0x64`) at `0x180`, and the three table pointers
at `0x1D0` (`trackstart = 0x240`), `0x1D4` (`pattstart = 0x220`), and `0x1D8`
(`macrostart = 0x22C`). All multi-byte header and table values are big-endian;
the two sample bytes are the raw signed sample words themselves.

| Offset | Contents |
|---|---|
| `0x220` | Pattern pointer table; entry 0 → `0x280`, entry 1 → `0x290`, terminator |
| `0x22C` | Macro pointer table; entry 0 → `0x2A0`, terminator |
| `0x240` | Trackstep 0: binding step `0000 FE01 FE02 FE03 FE04 FE05 FE06 FE07` |
| `0x250` | Trackstep 1: speed control step `EFFE 0002 0000 0064` |
| `0x260` | Trackstep 2: binding step `0100 FE01 FE02 FE03 FE04 FE05 FE06 FE07` |
| `0x270` | Trackstep 3: stop step `EFFE 0000` |
| `0x280` | Pattern 0: `F3000000` (pattern wait, count zero) then `F0000000` (pattern end) |
| `0x290` | Pattern 1: `F3010000` (pattern wait, count one) then `F0000000` (pattern end) |
| `0x2A0` | Macro 0: one `07 00 00 00` channel-macro-stop word (never executed because neither pattern has a note event) |

The header `tempo[0] = 100` is at or above `0x10`, so `StartSong` sets
`eClocks = 0x1B51F8 / 100 = 17904` and sets the prescale to `0` before any
bridge tick runs. Trackstep 0 binds voice 0 to pattern 0 and leaves the other
voices on their distinct inactive bindings, matching the binding step used by
the step-8 fixture. Neither pattern contains a note event, so no macro runs
and the rendered PCM is silent; the fixture's contract is the rendered tick
frame count and the finite completion flow, not audio content.

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
3. On tick 2 the pattern ends, advancing `CurrPos` from 0 to 1, and
   `GetTrackStep` loads trackstep 1, the speed control step. Its word
   `EFFE 0002 0000 0064` selects the speed control (`l[1] = 0x0002`) with
   prescale `l[2] = 0` and speed value `l[3] = 0x0064`; the high mask
   `(l[3] & 0xF200) == 0` passes and the low nine bits `0x64 = 100` lie in
   `16..511`, so the corrected expression sets `eClocks = 0x1B51F8 / 100 =
   17904`. Because the interpreter update happens during the bridge tick, the
   same tick's duration is computed from the updated value. `GetTrackStep`
   then advances to trackstep 2, which binds voice 0 to pattern 1.
4. Trackstep 2's pattern 1 wait command with count one spans tick 3, so tick 3
   renders with the retained speed-derived `eClocks = 17904` while the pattern
   waits.
5. On tick 4 pattern 1 ends, advancing `CurrPos` from 2 to 3, and
   `GetTrackStep` loads trackstep 3, the stop step `EFFE 0000`, which sets
   `trackManager.PlayerEnable = 0`. The engine completes after this tick.

## Focused observations

The application-level test does not assert exact per-tick frame counts (those
are owned by the playback component tests for the separate `mdat.header_tempo`
and `mdat.speed` fixtures). It drives this finite fixture through the private
application → fake CoreAudio workspace/HAL route and asserts only
application-owned observable composition: successful completion status, the
rate-0 startup selection, the fake negotiated 44.1 kHz renderer rate, the full
open/configure/prepare/bind/start/stop/quiesce/dispose lifecycle, enough
nonzero fake requests to reach the finite stop, and clean teardown.

Because header tempo 100 and speed divisor 100 both produce `eClocks = 17904`,
each rendered tick is the same header/speed-derived duration; the fixture's
contract is the finite flow that exercises both timing sources and then stops,
not a per-tick distinction between the two sources.

These observations are limited to this self-authored fixture and the private
playback boundary (`src/playback/playback_context.c`,
`src/playback/playback_legacy_bridge.c`, `src/playback/playback_legacy_mixer.c`,
`src/playback/playback_legacy_renderer.c`). They are not a format-wide
compatibility claim: the header-tempo `eClocks = 0x1B51F8 / tempo` behavior for
`tempo >= 0x10` and the speed step's `eClocks = 0x1B51F8 / low9` divisor
behavior are local repository-specific observations of `src/player.c`
(`StartSong` and `GetTrackStep`), and no external module or sample content is
referenced.

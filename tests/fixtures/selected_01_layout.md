# Selected subsong 01 fixture layout

This document describes the wholly self-authored fixture used by the
selected-subsong playback-context tests in
`tests/playback/test_playback_context.c`:

- `mdat.selected_01` — module data, stored big-endian.
- `smpl.step8` — reused sample data (two authored bytes `0x40`, `0xC0`).

The fixture exercises Track 017 selected-subsong behavior only: loader
admission remains slot-0-bounded, while the private bridge is expected to
validate the selected slot's inclusive trackstep range before start. Fixture
layout is not a product contract; assertions target observable start, render,
and completion behavior.

## Header

The 512-byte header records the normalized layout (file offsets):

| Field | Value | Meaning |
|---|---|---|
| `start[0]` | `0` | Slot 0 first trackstep |
| `end[0]` | `1` | Slot 0 last trackstep |
| `tempo[0]` | `6` | Slot 0 tempo |
| `start[1]` | `1` | Slot 1 first trackstep (normal selected start) |
| `end[1]` | `3` | Slot 1 last trackstep (inclusive stop at step 3) |
| `tempo[1]` | `6` | Slot 1 tempo |
| `trackstart` | `0x230` | Trackstep table offset |
| `pattstart` | `0x20C` | Pattern pointer table offset |
| `macrostart` | `0x200` | Macro pointer table offset |

Slot 0 remains structurally valid (`start[0] <= end[0]` within the validated
trackstep span), retaining slot-0 loader admission. Slot 1 is the selected
range `1..3` exercised by the focused tests. The focused tests start slot 1
only; they never start slot 0 of this fixture.

## Normalized tables

| Offset | Contents |
|---|---|
| `0x200` | Macro pointer table: entry 0 → `0x288`, entry 1 → `0x2A8`, terminator |
| `0x20C` | Pattern pointer table: entry 0 → `0x270`, entry 1 → `0x27C`, terminator |
| `0x230` | Trackstep table, four 16-byte tracksteps (`0x230`..`0x26F`) |
| `0x270` | Pattern 0 |
| `0x27C` | Pattern 1 |
| `0x288` | Macro 0 |
| `0x2A8` | Macro 1 |

## Tracksteps

Each trackstep contains eight big-endian 16-bit words. The high byte of each
word selects the pattern for that voice; `0xFE..` keeps a voice on its inactive
binding:

| Step | Bytes | Voice-0 binding | Role |
|---|---|---|---|
| 0 | `0100 FE01 FE02 FE03 FE04 FE05 FE06 FE07` | pattern 1 (volume 9) | Slot 0 start, distinct from slot-1 start |
| 1 | `0000 FE01 FE02 FE03 FE04 FE05 FE06 FE07` | pattern 0 (volume 15) | Slot 1 normal selected start |
| 2 | `0100 FE01 FE02 FE03 FE04 FE05 FE06 FE07` | pattern 1 (volume 9) | Slot 1 absolute override (trackstep 2) |
| 3 | `EFFE 0000 ...` | — | Stop step |

Step 0's volume-9 binding is deliberately distinct from step 1's volume-15
binding so a normal slot-1 start can be proven to begin at step 1 rather than
step 0. Starting slot 1 normally begins at step 1 (pattern 0 → macro 0,
volume 15), advances to step 2 (pattern 1 → macro 1, volume 9), then stops at
step 3. An absolute override at trackstep 2 begins directly at step 2 (volume
9) and stops at step 3, so neither the step-0 nor the step-1 signature plays.

## Patterns and macros

Both patterns are one note event, a bounded wait, and pattern end:

- Pattern 0 (`0x270`): `80000001` (note, macro 0, wait 1), `F3010000` (wait),
  `F0000000` (end).
- Pattern 1 (`0x27C`): `80010001` (note selects macro 1 on voice/channel 0,
  wait 1), `F3010000` (wait), `F0000000` (end).

Both macros set the same pitch (`0x06AE`), sample range (two bytes), DMA on,
one-tick wait, DMA off, then stop. They differ only in authored volume:

- Macro 0 (`0x288`): `0E00000F` sets volume `15`.
- Macro 1 (`0x2A8`): `0E000009` sets volume `9`.

## Focused observable signatures

- Slot 1 normal start: the first active voice-0 snapshot carries pitch
  `0x06AE` and volume `15` (step-1 / macro-0 signature), proving the start
  began at step 1 and not at the volume-9 step 0; the step-2 volume-9
  signature then appears before the step-3 stop, rendering is non-silent while
  active, and the track completes.
- Slot 1 with absolute override `startPat = 2`: the first active voice-0
  snapshot carries pitch `0x06AE` and volume `9` (step-2 / macro-1 signature);
  the volume-15 signature never appears, rendering is non-silent while active,
  and the track completes after the step-3 stop.
- Slot 1 with absolute override `startPat = 4`: position 4 lies outside the
  selected inclusive range `1..3`, so start fails safely with
  `TFMX_START_LEGACY_FAILURE`; a subsequent tick reports `TFMX_TICK_NOT_STARTED`
  and the legacy bridge reports complete with no playback state left active.

These observations document the local `GetTrackStep`, `DoTrack`, `NotePort`,
`RunMacro`, and mixer path only; they are not universal TFMX claims.

# Voices 0 and 1 fixture layout

This document describes the wholly self-authored fixture pair used by
`test_playback_context_plays_independent_voice_zero_and_one_fixture`:

- `mdat.voices_01` — module data, stored big-endian.
- `smpl.voices_01` — two authored, nonzero sample bytes.

The fixture is intentionally bounded to two patterns and two macros.  Its
header records `trackstart = 0x240`, `pattstart = 0x220`, `macrostart = 0x228`,
`tempo[0] = 6`, `start[0] = 0`, and `end[0] = 1`.  The normalized tables contain
pattern pointers `0x260` and `0x26c`, followed by macro pointers `0x278` and
`0x298`.

The first trackstep binds voice 0 to pattern 0 and voice 1 to pattern 1:

```text
0000 0100 FE02 FE03 FE04 FE05 FE06 FE07
```

The next trackstep is `EFFE 0000`, stopping the track.  Both patterns contain
one note event, a bounded wait, and pattern end.  The first note selects macro
0 and note 0; the second selects macro 1, note 1, and channel 1.  Both macros
set their sample range to the two-byte sample payload, enable DMA, wait 32
engine ticks, then stop.  The focused snapshot observes fixture-authored volume
18 for voice 0 and volume 30 for voice 1 at their first jointly active tick.

These fixture observations are limited to the local implementation symbols
`GetTrackStep`, `DoTrack`, `NotePort`, `RunMacro`, `DoAllMacros`, and
`tfmx_playback_legacy_mixer_render`; they are not universal TFMX claims.  The
lane-to-output conclusion that voice 0 contributes to the right lane and voice
1 to the left lane is **[inferred]** from the local mixer implementation and
the focused test's first-frame observation.

## Focused TDD observations

The first jointly active state occurred on engine tick 3.  At that tick the
snapshot reported voice 0 as active with pitch `0x06AE` and volume 18, voice 1
as active with pitch `0x064E` and volume 30, and voices 2–7 inactive.  Rendering
that tick succeeded with 3528 bytes; the first little-endian stereo frame was
left `1680`, right `1392`.  This is one bounded current routing/blend
observation, not a PCM fingerprint.  Completion was observed after engine tick
35, followed by both selected voices being inactive.

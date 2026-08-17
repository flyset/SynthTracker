# Envelope/tempo fixture layout

This document describes the self-authored fixture pair used by
`test_playback_context_applies_envelope_on_engine_ticks_with_tempo_prescale`:

- `mdat.envelope_tempo` — module data, stored big-endian.
- `smpl.envelope_tempo` — two authored, nonzero sample bytes.

The layout is intentionally distinct from the earlier fixtures. The normalized
tables are at `0x220` and `0x228`, the tracksteps begin at `0x240`, the pattern
begins at `0x260`, and the macro begins at `0x280`. Subsong 0 has one binding
trackstep followed by the existing stop trackstep. Its header stores
`tempo[0] = 2`.

The pattern contains a voice-0 note event, a wait, the pattern command
`0xF7030003` (the supported `0xF7` envelope command), another wait, and pattern
end. The macro sets pitch, sample range, initial volume 15, DMA, and a bounded
wait before stopping. These are repository-specific observations of
`DoTrack`, `NotePort`, and `RunMacro` in `src/player.c`, not universal format
claims. The fixture is self-authored and contains no copied module or sample
content.

The focused TDD test established the exact tick, envelope, render, and
completion observations below. The header's tempo value `2` is also used by
the local `StartSong` implementation to establish the prescale (`pre`)
behavior (`src/player.c`, `StartSong`); this is local implementation evidence,
not a direct focused-test report of a `pre=2` value or a universal timing claim.

## Focused TDD observations

The focused test loaded and started the pair through the private playback
context boundary. The voice-0 snapshot trace was:

| Engine tick | Active | Pitch | Volume |
|---:|---:|---:|---:|
| 1 | 0 | 0 | 0 |
| 2 | 0 | `0x06AE` | 0 |
| 3 | 1 | `0x06AE` | 15 |
| 4 | 1 | `0x06AE` | 15 |
| 5 | 1 | `0x06AE` | 15 |
| 6 | 1 | `0x06AE` | 15 |
| 7 | 1 | `0x06AE` | 15 |
| 8 | 1 | `0x06AE` | 15 |
| 9 | 1 | `0x06AE` | 15 |
| 10 | 1 | `0x06AE` | 15 |
| 11 | 1 | `0x06AE` | 15 |
| 12 | 1 | `0x06AE` | 15 |
| 13 | 1 | `0x06AE` | 15 |
| 14 | 1 | `0x06AE` | 15 |
| 15 | 1 | `0x06AE` | 12 |
| 16 | 1 | `0x06AE` | 12 |
| 17 | 1 | `0x06AE` | 9 |
| 18 | 1 | `0x06AE` | 9 |
| 19 | 1 | `0x06AE` | 6 |

This demonstrates the selected envelope changing on engine ticks under the
prescale of two established by local `StartSong`; the clean tick-1 pitch is 0,
and the observed pitch is
`0x06AE` from tick 2 onward. The active tick-3 state and envelope observation
are established by this focused test. The `0xF7` dispatch and envelope state
updates are repository-specific behavior observed in `NotePort` and `DoEffects`
(`src/player.c:96-100`, `src/player.c:566-585`); the prescale assignment is in
`StartSong` (`src/player.c:1123-1134`).

Each tick rendered into the caller-provided PCM buffer through
`tfmx_playback_context_render`. Across the 19 observed renders, 17 contained
at least one nonzero byte; the observed successful render sizes ranged from
3524 to 3528 bytes. The test does not assert a render-frame count or dynamic
eClock behavior. Completion was reported on tick 19; one further successful
tick left the context complete and did not reactivate it. These observations
are limited to this self-authored fixture and the private boundary
(`src/playback/playback_context.c`).

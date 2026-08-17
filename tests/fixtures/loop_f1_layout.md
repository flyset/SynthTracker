# Finite pattern-level `0xF1` fixture layout

This repository-specific, self-authored fixture pair is used only by
`test_playback_context_plays_finite_pattern_loop_to_completion`:

- `mdat.loop_f1` — big-endian module data authored for this test.
- `smpl.loop_f1` — two nonzero sample bytes authored for this test.

The layout is intentionally distinct from `step8_layout.md`. In the checked-in
binary, the header stores `trackstart = 0x240`, `pattstart = 0x220`, and
`macrostart = 0x228`; the pattern pointer table at `0x220` points to `0x260`,
the macro pointer table at `0x228` points to `0x270`, and the tracksteps begin
at `0x240`. Subsong 0 uses the binding trackstep followed by the existing stop
trackstep.

The four pattern words at `0x260` are `0x80000001`, `0xF3010000`,
`0xF1020000`, and `0xF0000000`. The `0xF1` word has count byte `0x02` and
target step `0`; its control-flow interpretation is **[inferred]** from the
local `DoTrack` implementation. The repository-observed trace records six
repetitions by tick 85; that observation is limited to this fixture and is not
universal TFMX behavior.

The eight macro words at `0x270` are `0x09000000`, `0x02000000`, `0x03000002`,
`0x0E00000F`, `0x01010000`, `0x04000001`, `0x13000000`, and `0x07000000`.
The macro selects the authored sample payload, sets pitch and volume, enables
DMA, waits, disables DMA, and stops. These interpretations are repository-
specific observations of `DoTrack` and `RunMacro` in `src/player.c`; they are
not universal format claims. The private load/start/tick/snapshot/render/
completion boundary is implemented by `tfmx_loader_read`,
`tfmx_playback_context_load`, `tfmx_playback_context_start`,
`tfmx_playback_context_tick`, `tfmx_playback_context_snapshot`,
`tfmx_playback_context_render`, and `tfmx_playback_context_is_complete`.

The fixture is self-authored and contains no copied module or sample content.
The loop interpretation is **[inferred]** from the local `DoTrack` `case 1`
implementation and remains limited to this repository-specific fixture.

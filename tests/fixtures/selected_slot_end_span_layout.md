# Malformed selected slot end span fixture layout

This document describes the wholly self-authored fixture used by the
selected-subsong rejection test in `tests/playback/test_playback_context.c`:

- `mdat.malformed_selected_slot_end_span` — module data, stored big-endian.
- `smpl.step8` — reused sample data (two authored bytes `0x40`, `0xC0`).

The fixture is byte-identical to `mdat.selected_01` except for the selected
slot's inclusive range: `end[1]` is `4` instead of `3`. In particular, pattern
1's note word at file offset `0x27C..0x27F` is `80010001` (selecting macro 1 on
voice/channel 0), matching `mdat.selected_01`. See `selected_01_layout.md` for
the shared header, table, trackstep, pattern, and macro layout.

## Selected-slot range and validation contract

The validated trackstep span is `(first_pattern - trackstart) / 16 = 4` steps,
indexed `0..3`. The header records:

| Field | Value | Meaning |
|---|---|---|
| `start[0]` | `0` | Slot 0 first trackstep |
| `end[0]` | `1` | Slot 0 last trackstep (loader admission bound) |
| `start[1]` | `1` | Slot 1 first trackstep |
| `end[1]` | `4` | Slot 1 last trackstep — beyond the validated span |

Slot 0 stays structurally valid, so loader admission still succeeds. Slot 1's
inclusive range `1..4` does not fit the validated span (step 4 does not
exist), so the private bridge is expected to reject the selected slot before
the legacy interpreter starts.

## Focused observable contract

- Loading `mdat.malformed_selected_slot_end_span` with `smpl.step8` succeeds.
- Starting subsong 1 fails safely with `TFMX_START_LEGACY_FAILURE` (not
  `TFMX_START_UNSUPPORTED_SUBSONG`, because slot 1 is within the selector
  domain but structurally invalid).
- After the failed start, a tick reports `TFMX_TICK_NOT_STARTED` and the
  legacy bridge reports complete (`PlayerEnable == 0`), so no legacy playback
  state is left active.

This observation documents the local loader and private bridge path only; it
is not a universal TFMX claim.

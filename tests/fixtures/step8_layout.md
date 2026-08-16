# Step 8 fixture layout

This document describes the self-authored fixture pair used by
`tests/playback/test_playback_context.c`:

- `mdat.step8` — module data, stored big-endian.
- `smpl.step8` — sample data.

## Normalized module layout

The loader accepts this normalized legacy-compatible layout (the offsets are
file offsets in `mdat.step8`):

| Offset | Contents |
|---|---|
| `0x000..0x1FF` | 512-byte header |
| `0x220` | Macro pointer table; entry 0 points to `0x260` |
| `0x224` | Pattern pointer table; entry 0 points to `0x250` |
| `0x230` | Trackstep table, two 16-byte tracksteps |
| `0x250` | First pattern |
| `0x260` | First macro |

The header stores `trackstart = 0x230`, `pattstart = 0x224`, and
`macrostart = 0x220`. Subsong 0 has `start[0] = 0` and `end[0] = 1`, so its
trackstep range contains the binding step at index 0 and the stop step at
index 1. Its tempo entry is `tempo[0] = 6`.

Each trackstep contains eight big-endian 16-bit words. The first step binds
voice 0 to pattern 0 and leaves the other voices on their distinct inactive
bindings:

```text
0000 FE01 FE02 FE03 FE04 FE05 FE06 FE07
```

The second step is the stop step `EFFE 0000`.

## Pattern and macro

The first pattern at `0x250` is exactly:

1. `0x80000001` — note event selecting macro 0.
2. `0xF3010000` — wait command with count one.
3. `0xF0000000` — pattern end.

The macro at `0x260` is exactly this ordered eight-word sequence:

1. `0x09000000` — set pitch.
2. `0x02000000` — set sample start to offset zero.
3. `0x03000002` — set sample length to two bytes.
4. `0x0E00000F` — set volume to `15`.
5. `0x01010000` — enable DMA.
6. `0x04000001` — wait one tick.
7. `0x13000000` — disable DMA.
8. `0x07000000` — stop.

`smpl.step8` is exactly two signed, nonzero sample bytes: `0x40` (+64) and
`0xC0` (-64). This is self-authored fixture data, not external module or
sample content.

## Verified five-tick snapshot trace

After loading and starting subsong 0, the playback-context test ticks once
before each snapshot. The verified voice-0 trace is:

| Tick | Active | Pitch | Volume |
|---:|---:|---:|---:|
| 1 | `0` | `0x0000` | `0` |
| 2 | `0` | `0x06AE` | `0` |
| 3 | `1` | `0x06AE` | `15` |
| 4 | `1` | `0x06AE` | `15` |
| 5 | `0` | `0x06AE` | `15` |

This trace documents playback-context ticks and snapshots only; it does not
establish wall-clock timing or audio-output timing.

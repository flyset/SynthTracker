# Default-pointer fixture layout

This self-authored fixture pair exercises the documented zero-pointer defaults
without changing the existing `step8` pattern, macro, or sample content:

- `mdat.default_pointers` — module data, stored big-endian.
- `smpl.step8` — the reused two-byte sample fixture.

## Module layout

The three header pointer fields at `0x1D0`, `0x1D4`, and `0x1D8` are raw zero
values. The valid content is placed at the documented default file offsets:

| Offset | Contents |
|---|---|
| `0x000..0x1FF` | 512-byte header |
| `0x400` | Pattern pointer table; entry 0 points to `0x820` |
| `0x600` | Macro pointer table; entry 0 points to `0x830` |
| `0x800` | Trackstep table, two 16-byte tracksteps |
| `0x820` | First pattern |
| `0x830` | First macro |

The defaults therefore resolve to `trackstart = 0x800`, `pattstart = 0x400`,
and `macrostart = 0x600`. Subsong 0 has `start[0] = 0` and `end[0] = 1`.
The trackstep bindings, pattern, macro, and sample are intentionally the same
self-authored content accepted by the present fixture recognizer.

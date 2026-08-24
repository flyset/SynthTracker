# `table_scan_cap128` loader-only layout

This self-authored MDAT proves the table capacity. It reuses `smpl.step8`; no
new SMPL fixture is part of this case.

| Offset | Contents |
|---|---|
| `0x000..0x1FF` | Header |
| `0x220` | Pattern table: `0x260`, then zero |
| `0x240` | Trackstep storage |
| `0x300` | Macro table with 129 valid cells, `0x520 + 4*i` for `i = 0..128` |
| `0x520..0x723` | Readable four-byte targets for all 129 macro cells |

The header stores `trackstart = 0x240`, `pattstart = 0x220`, and
`macrostart = 0x300`. The loader scans only the first 128 macro cells, so the
test expects one pattern, 128 macros, the first retained macro target `0x520`,
and the last retained target `0x71C`. The 129th valid cell points to `0x720`
and is intentionally beyond the retained capacity. This fixture is loader
only: no playback semantics are asserted.

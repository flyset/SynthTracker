# `table_scan_2p3m` loader-only layout

This self-authored MDAT proves independent bounded scans without asserting any
playback meaning. It reuses `smpl.step8`; no new SMPL fixture is part of this
case.

| Offset | Contents |
|---|---|
| `0x000..0x1FF` | Header |
| `0x220` | Pattern table: `0x260`, `0x270`, then zero |
| `0x230` | Macro table: `0x280`, `0x290`, `0x2A0`, then zero |
| `0x240` | Trackstep storage |
| `0x260`, `0x270` | Readable four-byte pattern targets |
| `0x280`, `0x290`, `0x2A0` | Readable four-byte macro targets |

The header stores `trackstart = 0x240`, `pattstart = 0x220`, and
`macrostart = 0x230`. The loader-only test expects two normalized pattern
indices, three normalized macro indices, and `first_pattern = 0x260`.
Pattern and macro contents are structural target storage only; this fixture is
not used to start or tick playback.

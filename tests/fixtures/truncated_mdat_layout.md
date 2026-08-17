# Malformed `truncated_mdat` layout

This is a wholly self-authored derivative of `mdat.step8`/`smpl.step8`; no
external module or sample material is included. `mdat.malformed_truncated_mdat`
is truncated from 640 bytes to 511 bytes (`0x1FF`), while
`smpl.malformed_truncated_mdat` remains the authored two-byte step8 payload.

The exact mutation is the file-length change: the `TFMX` marker at `0x000` is
retained, but the MDAT is less than `0x200` bytes. The intended local rejection
is `valid_mdat()`'s minimum-size check in `src/playback/tfmx_loader.c` before
header/table reads. This is repository-local behavior, not a universal TFMX
format claim **[inferred]**.

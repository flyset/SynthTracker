# Malformed `out_of_range_pattern` layout

This wholly self-authored derivative uses the step8 layout and no external
module or sample material. At MDAT offset `0x227`, the last byte of the pattern
pointer at `0x224` changes from `0x50` to `0x80`, producing aligned pointer
`0x280`, beyond the 640-byte file ending at `0x280`. The paired SMPL remains
the authored two-byte step8 payload.

The intended local rejection is `range_is_inside()` while validating the first
pattern in `valid_mdat()` (`src/playback/tfmx_loader.c`). This records a local
loader check, not a universal format claim **[inferred]**.

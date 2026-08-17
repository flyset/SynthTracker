# Malformed `invalid_stop_step` layout

This wholly self-authored step8 derivative contains no copied module or sample
data. MDAT byte `0x242` changes from `0x00` to `0x01`: the stop word at `0x240`
is still `EFFE`, but its following zero word is no longer zero. The paired SMPL
file remains the authored step8 two-byte payload.

The intended rejection is the follow-on zero-word check in `valid_mdat()` at
`src/playback/tfmx_loader.c`. This describes the local loader contract only
**[inferred]**.

# Malformed `invalid_pattern_contract` layout

This is a wholly self-authored derivative of step8, with no copied module or
sample material. MDAT byte `0x253` changes from `0x01` to `0x81`, changing the
first pattern word at `0x250` from `0x80000001` to `0x80000081`. The paired SMPL
file remains the authored two-byte step8 payload.

The intended local rejection is the first-pattern-word contract in
`valid_mdat()` (`src/playback/tfmx_loader.c`). It is local implementation
evidence **[inferred]**, not a universal TFMX claim.

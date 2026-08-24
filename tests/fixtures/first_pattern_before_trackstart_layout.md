# Malformed `first_pattern_before_trackstart` layout

This wholly self-authored pair derives from `mdat.step8`/`smpl.step8`; it makes
no playback claim. In `mdat.malformed_first_pattern_before_trackstart`, byte
`0x227` changes from `0x50` to `0x20`, changing the first pattern pointer at
`0x224` from `0x250` to the aligned, in-bounds `0x220`. That target is at least
`0x200` but precedes the `trackstart` at `0x230`.

The intended local rejection is the load/start relation rule requiring
`first_pattern > trackstart` after the first pattern is derived. This records
the private loader's bounded structural contract, not a universal TFMX claim
**[inferred]**.

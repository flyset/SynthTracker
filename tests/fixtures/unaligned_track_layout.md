# Malformed `unaligned_track` layout

This wholly self-authored pair derives only from the checked-in step8 fixture;
it contains no copied module or sample content. In
`mdat.malformed_unaligned_track`, header byte `0x1D3` changes from `0x30` to
`0x31`, making the big-endian `trackstart` at `0x1D0` equal `0x231`.
`smpl.malformed_unaligned_track` is unchanged step8 sample data.

The intended rejection is the alignment predicate for `trackstart` in local
`valid_mdat()` (`src/playback/tfmx_loader.c`). The cited check is evidence about
this loader's bounded fixture contract, not a universal TFMX assertion
**[inferred]**.

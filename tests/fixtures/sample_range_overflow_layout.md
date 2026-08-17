# Malformed `sample_range_overflow` layout

This wholly self-authored pair derives from step8 and contains no copied module
or sample material. MDAT byte `0x26B` changes from `0x02` to `0x03`, making the
macro's sample-length word at `0x268` request three bytes while the paired SMPL
file remains two authored bytes.

The actual intended local rejection is the earlier ordered macro-contract check
in `valid_mdat()` (`src/playback/tfmx_loader.c`), because this mutation no longer
matches the accepted macro word. Direct sample-range rejection at the later
`sample_start`/`sample_length` predicate is therefore deferred; loader ordering
is not changed. This is local evidence **[inferred]**, not a universal claim.

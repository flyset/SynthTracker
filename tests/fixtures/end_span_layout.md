# Malformed `end_span` layout

This wholly self-authored pair derives from `mdat.step8`/`smpl.step8`; it makes
no playback claim. In `mdat.malformed_end_span`, byte `0x141` changes from
`0x01` to `0x02`, changing subsong-0 `end` from `1` to `2`. The range
`[0x230,0x250)` contains only two complete 16-byte tracksteps, while
`end + 1` requires three.

The intended local rejection is the overflow-safe load/start trackstep-span
rule requiring the inclusive subsong-0 end to fit within
`[trackstart, first_pattern)`. This records the private loader's bounded
structural contract, not a universal TFMX claim **[inferred]**.

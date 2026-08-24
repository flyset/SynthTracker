# Malformed `out_of_range_macro` layout

This self-authored pair derives from `step8` and keeps its header, trackstep,
pattern table, and SMPL bytes. The first macro-table cell at `0x220` points to
`0x280`, exactly at the end of the `0x280`-byte MDAT, so no readable four-byte
target exists.

The independent macro scan stops before accepting the entry and the loader
rejects the empty result. This is local structural-loader evidence, not a
universal TFMX validity claim.

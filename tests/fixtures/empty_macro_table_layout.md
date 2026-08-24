# Malformed `empty_macro_table` layout

This self-authored pair derives from `step8` and keeps its header, trackstep,
pattern table, and SMPL bytes. The first macro-table cell at `0x220` is zero,
so the independent macro scan terminates without accepting an entry.

The loader must reject the pair because every table must yield at least one
readable, aligned target. This is local structural-loader evidence, not a
universal TFMX validity claim.

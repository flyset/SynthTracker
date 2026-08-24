# Malformed `truncated_macro_table` layout

This self-authored pair keeps a valid pattern table and truncates the MDAT at
`0x27E`. Its header resolves `macrostart` to `0x27C`, leaving fewer than four
bytes for the first macro-table cell.

The independent macro scan stops before reading an incomplete cell and the
loader rejects the empty result. This is local structural-loader evidence, not
a universal TFMX validity claim.

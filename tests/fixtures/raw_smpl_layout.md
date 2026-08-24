# Raw SMPL loader-only evidence

These self-authored fixtures exercise only direct `tfmx_loader_read()` admission.
The tests do not start a playback context, tick, or render, and make no playback
or audio claim.

- `smpl.raw_leading_zero` is exactly two bytes: `00 00`. It is paired with the
  existing `mdat.step8` and proves that leading zero bytes are admitted as raw
  SMPL data.
- `mdat.raw_smpl_opaque` is a self-authored derivative of `mdat.step8`. Its only
  change is byte `0x26B`, from `02` to `03`, so the first macro declares three
  sample bytes while the existing `smpl.step8` contains two bytes. It is paired
  with `smpl.step8` and proves that the loader does not infer a sample range from
  macro content.
- `smpl.raw_one_byte` is exactly one byte: `00`. It is paired with the existing
  loader-only `mdat.table_scan_2p3m` and proves that the retained minimum-size
  rule rejects SMPL shorter than two bytes.

The fixtures document loader admission only; they do not establish sample
interpretation, playback behavior, timing, or audio compatibility.

# Artifacts

This document catalogs artifacts that cross the approved target component
boundaries. Target artifacts are design vocabulary, not claims about the
current transitional CLI. This document does not define C APIs, concrete data
types, allocation or lifetime mechanics, threading, scheduling, or device
integration unless explicitly stated. [ADR-008](adr/ADR-008-audio-frame-block-mixed-value-boundary.md)
and [ASR-009](ASR.md#asr-009--audio-frame-block-boundary-invariants) are the
authority for the Audio Frame Block's mixed-value representation and validity
invariants. ADR-007 is the superseded historical record of the earlier fixed
PCM first format. [ADR-009](adr/ADR-009-callback-driven-audio-rendering.md)
and [`AUDIO_RENDERING_DESIGN.md`](AUDIO_RENDERING_DESIGN.md) record the
approved callback-driven rendering model for live audio; the private
device-driven route is implemented by Track 015 S5, with the negotiated-rate
preparation handoff and private legacy normalization added by S6.2–S6.4b,
while the public Audio Output Port and target `Mixer` remain target-only.
Track 016 separately delivered bounded structural loader admission in the
private loader/bridge: `first_pattern` strictly after `trackstart`, and a
subsong-0 inclusive `end` requiring all `end + 1` complete 16-byte tracksteps
within `[trackstart, first_pattern)` (loader primary admission; the private
bridge repeats both checks defensively before legacy state binding/start). It
changes no artifact contract, public API/ABI, timing/interpreter/audio
behavior for accepted modules, persistence, adapter, or compatibility
promise.
Track 017 separately delivered bounded private selected-subsong start through
the existing CLI, context, and bridge path: strict nonnegative `-p` selection
limited to header slots `0..31`, an absolute `-P` trackstep within the
selected inclusive range, and selected-range and position validation at the
private bridge before legacy start, with loader admission remaining
slot-0-only. It changes no artifact contract, public API/ABI,
timing/interpreter/audio behavior for accepted modules, persistence, adapter,
or compatibility promise.

## Audio Frame Block

An **Audio Frame Block** is a finite, ordered block of zero or more rendered
audio frames. The `Mixer` emits it for consumption by the device-independent
`Audio Output` port or by `File I/O` for rendered-audio export; the exact
internal processing point inside a future `Mixer` remains open. Zero-frame
blocks are valid.

Each frame contains a signed-32 left mix value and a signed-32 right mix value
at one sample instant. The block carries raw mixed values, not serialized PCM
and not device-native data; it is not a synthesizer stream, a legacy TFMX
`Channel`, a legacy `struct Audio`, or a device-native buffer.

### Deferred boundary contract

The mixed-value representation and validity invariants are defined by ADR-008
and ASR-009. A future `Mixer` emits the same Audio Frame Blocks and may perform
additional internal mixing, effects, or processing before emitting them; the
precise internal processing point inside the `Mixer` is not fixed. The private
legacy mixer's fixed L/R blend is producer-side: it blends the voice lanes and
reconstructs the low-16 historical PCM-domain value (×65536) before emitting
signed-32 Frame Blocks, so blending is not part of the block's representation.
Legacy low-pass filtering and PCM byte packing are outside this boundary, the
SDL-era paths that carried them were retired with the legacy audio path, and
none of these is performed in Audio
Output. Destination format adaptation is owned by Audio Output and its platform
adapters; File I/O serializes output for rendered-file export. The concrete C
API and types, numerical range/overflow/clip behavior, ownership, lifetime,
timing, ordering, queueing, backpressure, validation-result representation,
errors, adapter behavior, and implementation remain deferred.

### Responsibility

| Concern | Current target vocabulary |
| --- | --- |
| Producer | `Mixer` |
| Consumers | `Audio Output`; `File I/O` for rendered-audio export |
| Payload | An ordered sequence of frames, each with signed-32 left and right mix values emitted by the `Mixer`; the Mixer's internal creation point is not fixed |
| Descriptive metadata | Frame count and the left/right signed-32 mix-value representation; sample-rate and channel metadata remain deferred |
| Device ownership | None; SDL, CoreAudio, and other device handles are outside the artifact |

### Open contract questions

The following remain intentionally open:

- ownership, borrowing, copying, and lifetime across the consumer boundary;
- block timing, ordering, queueing, and backpressure (the approved
  callback-driven direction is recorded in
  [ADR-009](adr/ADR-009-callback-driven-audio-rendering.md) and
  [`AUDIO_RENDERING_DESIGN.md`](AUDIO_RENDERING_DESIGN.md); the specific policy
  remains open);
- adapter capability and format-negotiation or conversion policy beyond the
  destination-adaptation ownership stated in ADR-008;
- validation-result and error representation beyond the invariants in ASR-009;
- the concrete C type or API used to carry the artifact.

These questions remain unresolved. The artifact name alone does not establish a
public C API or ABI.

### Target flow

```text
Tracker → timed musical events → Synthesizer → synthesizer streams
    → Mixer → Audio Frame Blocks → Audio Output → platform adapter
                              └→ File I/O for rendered-audio export
```

The `Synthesizer` stream and `Mix data` artifacts are related target vocabulary
but require their own definitions. The current legacy `player.c` and the
playback/audio-output translation units do not implement these target
contracts.

### Rendering model (approved direction)

[ADR-009](adr/ADR-009-callback-driven-audio-rendering.md) records the approved
callback-driven rendering direction for live audio: the platform device owns
render timing and requests audio in blocks; a private render boundary answers
every requested Audio Frame Block, including variable-size and zero-frame
requests, on demand from current/future playback state and time-ordered events
— never
from pre-rendered audio. On the implemented workspace/HAL route, each nonzero
request is answered with exactly the requested frame count and a zero-frame
request is accepted at the adapter without invoking the renderer or the
exact-N coordinator; the "every request produced" phrasing remains the
approved direction. The callback path uses lifecycle-preallocated storage
and performs no allocation, locking, file I/O, UI work, or other unbounded
work. Adapter-private conversion remains separate, and no public Audio Output
Port, C API, header, or export is introduced.

Track 015 S5 implemented this direction as the private device-driven live
route: the real legacy exact-N renderer
(`src/playback/playback_legacy_renderer.c`) fulfills device requests with
partial-tick retention and exactly-once tick advance (44.1/48 kHz fixture
evidence); the private CoreAudio adapter runs a lock-free OPEN/IN_FLIGHT
admission gate, converts signed-32 mixed values to interleaved Float32 into a
validated borrowed workspace, and copies into the separate native buffer on
the HAL workspace-copy route; and the production default facade is a real HAL
Output Audio Unit on macOS with a strict 44.1/48 kHz negotiated-rate gate and
no resampler. The application cuts over to this route, and the SDL live-audio,
`-o` file-output, and removed-option (`-b`, `-8`, `-f`, `-o`, `-w`, `-v`)
paths are deleted. Track 015 S6.2–S6.4b delivered the private rate-0
startup-selection sentinel with a verified 44.1/48 kHz configured rate and
control-side renderer preparation before bind/start, and the private legacy
producer's blend-then-widen normalization (blended int32 lanes reconstructed
to their low-16 historical PCM-domain value and multiplied by 65536 into the
existing signed-32
Audio Frame Blocks; the generic adapter conversion remains signed-32/2^31);
  `voices_01` is the self-authored supplemental audible-smoke fixture, and a
  listener confirmed real 48 kHz `voices_01` playback. Track 015 S6.5 (final
  validation/documentation) is complete (full CTest 8/8, required documentation
  reconciled, Phase 4 roadmap revised to revision 14 with canonical index
  revision 21); S7 (completion) is checked and Track 015 is completed at
  `.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`,
  while Stage 3 remains in progress pending its wider acceptance/merge. The
  public Audio
Output Port/API, target `Mixer`, non-macOS adapters, the future GUI, live
input, rendered-file export, the device-rate-change restart policy, workspace
release/close, invalid storage-length proof, and general real-module loader
expansion remain deferred; Track 016 delivered bounded structural load/start
admission for the selected corpus as a restrictive private
structural-admission correction while general real-module loader
compatibility and the loader redesign remain deferred. See
[`AUDIO_RENDERING_DESIGN.md`](AUDIO_RENDERING_DESIGN.md).

### Compatibility boundary

During Phase 4, preserving current TFMX behavior where practical remains a
temporary development scaffold. The Audio Frame Block is target-architecture
vocabulary, not a claim about the current transitional CLI. In the current
CLI, the private live route renders the legacy engine's combined voices as
signed-32 Audio Frame Blocks: the private legacy producer first computes
blended int32 lanes using the legacy fixed L/R blend, then explicitly
reconstructs each blended lane's low-16 historical PCM-domain value and
multiplies it by 65536 into the signed-32 range (the generic adapter
conversion remains signed-32/2^31); the retired `-o`-dependent
stereo blending,
low-pass filtering, and PCM packing are not moved into Audio Output. This
compatibility behavior is distinct from the target `Mixer` rule
above, which requires a future `Mixer` to emit the same Audio Frame Blocks
without fixing its internal processing point. Defining the Audio Frame Block
and retiring the SDL-era audio and `-o` file-output paths does not create a
SynthTracker v1 compatibility promise; compatibility evidence is bounded
fixture and structural contract evidence only: the recorded XOut2 rejection
remains historical Track 015 evidence of the then-fixture-only loader, Track
016 later delivered bounded structural load/start admission as a restrictive
private structural-admission correction (with no change to timing/interpreter/
audio behavior for accepted modules), and general
real-module loader compatibility remains deferred.

## Artifact documentation rules

Each artifact entry should identify:

1. its producer and consumers;
2. its payload and descriptive metadata;
3. ownership, lifetime, timing, validation, and error expectations;
4. excluded responsibilities and compatibility impact; and
5. unresolved contract questions.

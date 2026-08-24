# SynthTracker

SynthTracker is the product and repository identity. It is a modern C product
with a transitional legacy CLI and a future GUI-first DAW. TFMX denotes the
legacy format, modules, semantics, and temporary compatibility lineage.

## Overview
SynthTracker is one system with a current transitional legacy CLI and private
CoreAudio live output on macOS. It aims to **refactor and reimplement** the
legacy **TFMX player** in modern C, with a future GUI-first DAW using SDL. All
SynthTracker-owned production
and test source, including the future GUI/DAW, must use C23 or a later ISO C
standard; C++ is not a project direction.
Third-party dependency implementation languages are evaluated separately.
During Phase 4, preserve current TFMX behavior where practical as a
temporary development scaffold; every Phase 4 Track must assess compatibility
impact and retain appropriate evidence. This is not a SynthTracker v1
compatibility promise. The product also aims to improve performance,
maintainability, and user experience.

### Current Status
- **Refactoring progress**: Initial refactoring and setup for the modern C product
  boundary.
- **UI Integration**: Planning and design for a future GUI-first DAW; no GUI or
  editing functionality is implemented yet.
- **Private CoreAudio live route (Phase 4)**: Track 015 S5 implemented the
  private device-driven live route: the real legacy exact-N renderer, the
  private CoreAudio adapter lifecycle/callback with control-side quiescence,
  the lock-free admission gate, workspace-only conversion, the real HAL Output
  Audio Unit facade with a strict 44.1/48 kHz negotiated-rate gate, and the
  application cutover. Track 015 S6.2–S6.4b delivered the private rate-0
  startup selection (the application requests rate 0; after device open the
  facade accepts only an actual 44.1/48 kHz nominal rate and reports the
  verified configured rate for control-side renderer preparation before
  bind/start) and the private legacy producer's blend-then-widen
  normalization (blended int32 lanes reconstructed to their low-16 historical
  PCM-domain value and multiplied by 65536 into the existing signed-32 Audio
  Frame Blocks); the generic adapter conversion remains signed-32/2^31.
  `voices_01` is a self-authored
  supplemental audible-smoke fixture, and a listener confirmed real 48 kHz
  `voices_01` playback. Track 015 S6.5 (final validation/documentation) is
  complete: the full CTest suite passes 8/8, the required documentation was
  reconciled, and the Phase 4 roadmap was revised (Phase 4 revision 14;
  canonical index revision 21). S7 (completion) is checked: Track 015 is
  completed at
  `.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`.
  Stage 3 remains in progress pending its wider acceptance/merge.
- **Loader compatibility**: Compatibility evidence is bounded to the four
  approved self-authored fixture pairs (`step8`, `loop_f1`, `envelope_tempo`,
  and `voices_01`) plus bounded Phase 4 evidence. General real-module loader
  compatibility remains deferred after the recorded XOut2 module rejection,
  with no format-wide promise.
- **SDL audio and `-o` removed**: The legacy SDL live-audio path, the `-o`
  file-output path, and the `-b`, `-8`, `-f`, `-o`, `-w`, and `-v` options are
  removed; each removed option is rejected as an unknown option, and `-o` has
  no file-output side effect. SDL remains only a future GUI decision.
- **Platform scope**: macOS only. Linux and other platform support are outside the current project scope; adding a platform requires a new explicit roadmap decision.
- **Phase status**: Phase 3 is delivered; Phase 4 is in progress.

## Features
- **Bounded loader evidence**: the private loader and playback route are
  validated against the four approved self-authored fixture pairs (`step8`,
  `loop_f1`, `envelope_tempo`, `voices_01`) plus bounded Phase 4 compatibility
  evidence. General real-module loader compatibility is not established and
  remains deferred after the recorded XOut2 module rejection; no format-wide
  promise is made.
- **macOS support** (the current platform scope)
- **Private CoreAudio live output (macOS)** — the transitional CLI now plays
  through a private device-driven route: the application composes a private
  legacy exact-N renderer and a private CoreAudio adapter instance, and the
  macOS HAL Output Audio Unit render callback drives the private render
  boundary for each device-requested nonzero frame count; a zero-frame
  workspace/HAL request is accepted at the adapter's admission gate without
  invoking the renderer or the exact-N coordinator. The application requests the
  private rate-0 startup-selection sentinel; the facade reports the verified
  44.1/48 kHz device rate, and the control side prepares the renderer at that
  rate before bind/start. The adapter converts signed-32 Audio Frame Blocks to
  adapter-private interleaved Float32 in a preallocated workspace with a
  lock-free admission gate and control-side quiescence; the private legacy
  producer first computes blended int32 lanes (legacy fixed L/R blend), then
  explicitly reconstructs each blended lane's low-16 historical PCM-domain
  value and multiplies by 65536 into the existing signed-32 Frame Blocks,
  while the generic adapter conversion remains signed-32/2^31. It is not a
  public API or the future Audio Output Port.
- **Retired SDL-era audio**: the SDL live-audio path and the legacy `-o` file
  output are removed; the `-o`-dependent file-output stereo blending,
  low-pass filtering, and oversampling retire with them, while the mixer's
  legacy fixed L/R blend remains in the private live route. The `-b`, `-8`,
  `-f`, `-o`, `-w`, and `-v` options are rejected as unknown options.

## Known Issues
- General real-module loader compatibility is not established: only the four
  approved self-authored fixture pairs plus bounded Phase 4 evidence are
  covered.
- The private loader's bounded fixture evidence does not cover all real-module
  layouts: a recorded XOut2 module/sample pair is rejected with
  `TFMX_LOAD_INVALID_FORMAT`. This is a private loader-layout limitation
  deferred to a separate loader-focused Track, not an assertion that the
  module is malformed, and no format-wide compatibility is promised.
- A specific version of the **Z-Out theme** causes a segfault on macOS. Historically, this was reported as fixed in Linux; Linux is outside the current project scope.
- Performance is slightly lower than the legacy OSS implementation but acceptable on modern hardware.

## Build Instructions
### Dependencies
- **Phase 1 validation baseline**: C23 on macOS with Clang. The legacy SDL-era
  audio dependency is retired; the current live route is a private CoreAudio
  HAL Output Audio Unit route, with the CoreAudio frameworks linked on Apple
  platforms only. Other-platform validation is outside the current project
  scope. SDL 1.1.7 is historical context, not an asserted current build
  dependency.

- **CMake 3.13+**: Minimum version required for configuring the build.
- **CMocka 2.0.2**: Chosen C-native test framework, provided as a system-installed
  Homebrew package on macOS. CMake uses config-mode discovery via the Homebrew
  CMocka prefix, and the test target is linked and registered with CTest. CMocka
  is not downloaded or vendored.

### Steps
1. Create a build directory and navigate into it:
   ```bash
   mkdir -p build && cd build
   ```

2. Configure the project using CMake:
   ```bash
   cmake ..
   ```

3. Compile the project:
   ```bash
   make
   ```

4. Run the executable:
   ```bash
   ./SynthTracker
   ```

> **Note**: This project is **legacy software**. The current Phase 1 boundary
> builds the legacy engine under C23 on macOS with a private CoreAudio live
> route; SDL 1.1.7 and the SDL 1.2-era API surface describe the historical
> legacy context only.

## Usage
Run `-h` for usage instructions and feature details.

## Documentation
- [Vision](docs/VISION.md) — product intent, boundaries, and future direction.
- [Architecture](docs/ARCHITECTURE.md) — current-system overview and entrypoint.
- [ADR index](docs/ADR.md) — governance for future architectural decisions.
- [ASR register](docs/ASR.md) — architecturally significant requirements.
- [Glossary](docs/GLOSSARY.md) — canonical product and protocol terminology.
- [Artifacts](docs/ARTIFACTS.md) — target component-boundary artifacts and open
  contracts.
- [Agent workflow](docs/AGENT_WORKFLOW.md) — contribution gates and verification.

## License
This project is released under the **GNU GPLv3**. See `LICENSE` for details.

## Contributing
Want to help? Fork the repository and submit a pull request. For questions or legal clarifications, contact **Neochrome** at [neko@netcologne.de](mailto:neko@netcologne.de).

## Links
- Latest updates: [http://darkstar.tabu.stw-bonn.de/~neo/audio.html](http://darkstar.tabu.stw-bonn.de/~neo/audio.html)
- More tools: [http://darkstar.tabu.stw-bonn.de/~neo/](http://darkstar.tabu.stw-bonn.de/~neo/)

# TFMX.cpp

A **modern C++ port and refactor** of the legacy **TFMX music player**, with SDL 1.1.7 retained as historical legacy context.

## Overview
This project aims to **port and refactor** the legacy **TFMX player** from C to **C++**, while adding a **modern UI** to enhance usability. The goal is to maintain compatibility with existing TFMX modules while improving performance, maintainability, and user experience.

### Current Status
- **Porting progress**: Initial refactoring and setup for C++ compatibility.
- **UI Integration**: Planning and design for a modern user interface (e.g., Qt, ImGui, or SDL2-based).
- **Legacy Compatibility**: Retaining support for legacy TFMX modules and the SDL 1.2-era audio features currently used by the engine, including stereo blending and low-pass filtering.

## Features
- Plays **most TFMX modules**, including:
  - MasterBlazer, Apidya, Turrican II/III, JimPower
  - MUDS, R-Type Theme, Z-Out, The Oath, and more
- **Stereo blending** (adjustable for headphone users)
- **Low-pass filter** (high, medium, low cutoff frequencies)
- **Portable** (Linux, macOS, and other SDL-supported platforms)

## Known Issues
- Some TFMX files may not play correctly.
- A specific version of the **Z-Out theme** causes a segfault on macOS (fixed in Linux).
- Performance is slightly lower than the legacy OSS implementation but acceptable on modern hardware.

## Build Instructions
### Dependencies
- **Phase 1 validation baseline**: C23 on macOS with Clang, against the SDL 1.2-era API surface currently used by the legacy engine. Linux/GCC validation is deferred. SDL 1.1.7 is historical context, not an asserted current build dependency.

- **CMake 3.10+**: Required for configuring the build.

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
   ./tfmx
   ```

> **Note**: This project is **legacy software**. The current Phase 1 boundary targets the SDL 1.2-era API surface used by the engine; SDL 1.1.7 describes the historical legacy context only.

## Usage
Run `-h` for usage instructions and feature details.

## License
This project is released under the **GNU GPLv3**. See `LICENSE` for details.

## Contributing
Want to help? Fork the repository and submit a pull request. For questions or legal clarifications, contact **Neochrome** at [neko@netcologne.de](mailto:neko@netcologne.de).

## Links
- Latest updates: [http://darkstar.tabu.stw-bonn.de/~neo/audio.html](http://darkstar.tabu.stw-bonn.de/~neo/audio.html)
- More tools: [http://darkstar.tabu.stw-bonn.de/~neo/](http://darkstar.tabu.stw-bonn.de/~neo/)

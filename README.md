# tfmx-play/SDL 1.1.7

A **TFMX music player** for Linux, macOS, and other platforms using **SDL 1.1.7**.

## Overview
tfmx-play is a **free and open-source** music player for TFMX (Turrican FMX) modules, originally developed by Jonathan H. Pickard and David Banz. This version supports **stereo blending** and **low-pass filtering** for improved audio quality.

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
- **SDL 1.1.7**: Required for building the project. Install it using:
  ```bash
  brew install sdl11  # macOS (Homebrew)
  ```
  If SDL 1.1.7 is unavailable, you may need to compile it from source.

- **CMake 3.10+**: Required for configuring the build. Install it using:
  ```bash
  brew install cmake  # macOS (Homebrew)
  ```

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

> **Note**: This project is **legacy software** and relies on **SDL 1.1.7**, which may not be readily available on modern systems. If you encounter issues, consider compiling SDL 1.1.7 from source.

## Usage
Run `-h` for usage instructions and feature details.

## License
tfmx-play is released under the **GPL**. See `COPYING` for details.

## Contributing
Want to help? Contact **Neochrome** at [neko@netcologne.de](mailto:neko@netcologne.de).

## Links
- Latest updates: [http://darkstar.tabu.stw-bonn.de/~neo/audio.html](http://darkstar.tabu.stw-bonn.de/~neo/audio.html)
- More tools: [http://darkstar.tabu.stw-bonn.de/~neo/](http://darkstar.tabu.stw-bonn.de/~neo/)
# OpenGraal GS2

This repository provides a reimplementation of GraalScript V2 (GS2). The
primary goal is full compatibility with the original
[GraalOnline](https://graalonline.com) implementation, allowing existing
scripts to execute as-is, while enabling standalone use, testing, and
future extensions.

## Requirements

To build and run the project, you’ll need:

- A C++ compiler with C++23 support (e.g. `clang++`)
- [CMake](https://cmake.org/) (version 4.1+ recommended)
- [Ninja](https://ninja-build.org/)
- [just](https://github.com/casey/just) (for running build commands)
- A Unix-like environment (Linux/macOS recommended)

Optional but recommended:

- `compile_commands.json`-compatible tooling (e.g. `clangd`)

## Build

### 1. Clone the repository

```sh
git clone https://github.com/Guthius/og-gs2.git
cd og-gs2
```

### 2. Configure

```sh
just configure
```

This will:

- Generate build files using CMake
- Use `clang++` with C++23
- Set up a `build/` directory
- Export `compile_commands.json` for tooling

### 3. Build

```sh
just build
```

### 4. Run a script

```sh
just run path/to/script.gs2
```

**Note**: At the moment the interpreter is not implemented. Running a script
will only output it's AST.

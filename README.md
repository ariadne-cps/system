# Ariadne System

[![macOS Status](https://github.com/ariadne-cps/system/actions/workflows/macos.yml/badge.svg)](https://github.com/ariadne-cps/system/actions/workflows/macos.yml)
[![Ubuntu Status](https://github.com/ariadne-cps/system/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/ariadne-cps/system/actions/workflows/ubuntu.yml)
[![Windows Status](https://github.com/ariadne-cps/system/actions/workflows/win.yml/badge.svg)](https://github.com/ariadne-cps/system/actions/workflows/win.yml)
[![Sanitizers Status](https://github.com/ariadne-cps/system/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/ariadne-cps/system/actions/workflows/sanitizers.yml)
[![Coverage Status](https://github.com/ariadne-cps/system/actions/workflows/coverage.yml/badge.svg)](https://github.com/ariadne-cps/system/actions/workflows/coverage.yml)
[![codecov](https://codecov.io/gh/ariadne-cps/system/branch/main/graph/badge.svg)](https://codecov.io/gh/ariadne-cps/system)

Ariadne System is the standalone C++20 dynamical-system specification layer. It defines continuous vector fields, iterated maps, differential inclusions, hybrid automata and first-order PDE models. 

## Layout

Public headers live under `include/system`, implementations under `src`, C++ tests under `tests`, and the Python bindings and smoke tests under `python`. The equivalent C++ and Python standalone examples live under `tutorial/`; pinned Git submodules live under `ss`. The umbrella header is `include/ariadne-system.hpp`.

The [System tutorial](tutorial/README.md) covers hybrid automata, vector fields, iterated maps, differential inclusions, and first-order PDE models.

## Dependencies

System depends directly on:

- [ariadne-cps/hybrid](https://github.com/ariadne-cps/hybrid), as a Git submodule.
- [ariadne-cps/configuration](https://github.com/ariadne-cps/configuration), for shared CMake configuration.

Hybrid provides the lower-level Ariadne components used by System.

## Build

```bash
git clone --recurse-submodules https://github.com/ariadne-cps/system.git
cd system
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel --target everything
ctest --test-dir build --output-on-failure
```

The [System tutorial](tutorial/README.md) includes separate build/run instructions for the installed C++ library and Python extension.

## Python bindings

When a compatible Python development environment is available, the standalone build produces the combined `pyariadne` extension from Hybrid and System bindings. Run the smoke tests with `ctest --test-dir build -L python --output-on-failure`; they exercise real System objects.

## Contribution guidelines

If you would like to contribute to Ariadne System, please contact the developer:

- Luca Geretti <luca.geretti@univr.it>

## License

Ariadne System is released under the GNU General Public License v3.0.

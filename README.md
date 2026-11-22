# Ariadne System

[![Format Status](https://github.com/ariadne-cps/system/actions/workflows/format.yml/badge.svg)](https://github.com/ariadne-cps/system/actions/workflows/format.yml)
[![macOS Status](https://github.com/ariadne-cps/system/actions/workflows/macos.yml/badge.svg)](https://github.com/ariadne-cps/system/actions/workflows/macos.yml)
[![Ubuntu Status](https://github.com/ariadne-cps/system/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/ariadne-cps/system/actions/workflows/ubuntu.yml)
[![Windows Status](https://github.com/ariadne-cps/system/actions/workflows/win.yml/badge.svg)](https://github.com/ariadne-cps/system/actions/workflows/win.yml)
[![Sanitizers Status](https://github.com/ariadne-cps/system/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/ariadne-cps/system/actions/workflows/sanitizers.yml)
[![Coverage Status](https://github.com/ariadne-cps/system/actions/workflows/coverage.yml/badge.svg)](https://github.com/ariadne-cps/system/actions/workflows/coverage.yml)
[![codecov](https://codecov.io/gh/ariadne-cps/system/branch/main/graph/badge.svg)](https://codecov.io/gh/ariadne-cps/system)

Ariadne System is the standalone C++20 dynamical-system specification layer. It defines explicit ordinary differential/algebraic equations and inclusions, iterated maps, hybrid automata and first-order PDE models.
Although independently maintained and usable, this repository is also part of the [Ariadne repository structure](https://github.com/ariadne-cps/ariadne/blob/main/doc/structure.md).

## Layout

Public headers live under `include/system`, implementations under `src`, C++ tests under `tests`, and the Python bindings and smoke tests under `python`. The equivalent C++ and Python standalone examples live under `tutorial/`; pinned Git submodules live under `ss`. The umbrella header is `include/ariadne-system.hpp`.

The [System tutorial](tutorial/README.md) covers hybrid automata, vector fields, iterated maps, differential inclusions, and first-order PDE models.

## Ordinary dynamics declarations

Use `system/odae.hpp` and `OrdinaryDifferentialAlgebraicEquation` for explicit
ordinary equations, or `system/odai.hpp` and
`OrdinaryDifferentialAlgebraicInclusion` for differential inclusions. These are
the actual C++ and Python class names; the former class names and header paths
are removed without compatibility aliases.

The equation class retains explicit auxiliary assignments. The names do not
imply support for generic implicit algebraic constraints, consistency solving or
index reduction. Those facilities belong to a later phase.

Both symbolic declarations distinguish physical state, an optional explicit
time variable, trajectory-constant parameters with a domain, and (for inclusions)
arbitrarily time-varying inputs. Function arguments have the fixed order
`[state, optional time, constant parameters, free inputs]`; `dimension()` counts
only physical states. `argument_space()` describes the complete layout.
Parameters are never normalised or treated as freely varying inputs.

Inclusions support two owned mathematical representations:

- Dotted symbolic assignments with an input box, retaining input centering and
  additive-coefficient normalisation. Consume `function()` and `inputs()` as a
  pair; input derivatives and the noise-independent component refer only to the
  final free-input block, even for timed/parameterised fields.
- A double-precision interval-Taylor vector field and its named state space.
  `interval_taylor_multifunction()` retains its full domain, coefficients and
  remainder; `multifunction()` provides a mathematical compact-set view.
  Coefficients have pointwise inclusion semantics. Zero named inputs does not
  mean deterministic dynamics. This representation has no fabricated
  single-valued input function; incompatible accessors fail explicitly.

The Python extension exposes the corresponding interval field models as well
as the new declarations. The C++ and Python tutorials demonstrate both forms.

System describes the mathematical problem using Function representations. It
has **no dependency on Integration** and constructs no numerical solver,
integration request, flow certificate or synthetic clock state. Numerical
adapters and any solver-specific lifting remain outside System. Evolution
consumes these declarations through its numerical adapters.

## Ordinary dynamics in hybrid modes

`HybridAutomaton::new_mode(location, equation_or_inclusion)` owns an immutable
copy of an ODAE or ODAI declaration. The `HybridAutomatonInterface::equation()`
and `inclusion()` queries expose the original declaration for a mode; legacy
symbolic-assignment modes return null for both. State dimension and labels
come from the declaration. Inputs, fixed parameters and explicit time keep
their roles and never become physical states. Guards and resets continue to
refer to state and auxiliary variables.

`dynamic_function()` remains a state-only single-valued query. It rejects
inclusion modes and timed or parameterised equation modes, rather than freezing
uncertainty or dropping arguments. Numerical consumers choose the capabilities
appropriate to the typed declaration. Composition of typed modes is explicitly
rejected until parameter domains and input roles can be composed consistently.
The new mode overloads are currently C++; their Python exposure will be added
with the final Evolution tutorial parity work.

## Dependencies

System depends directly on:

- [ariadne-cps/hybrid](https://github.com/ariadne-cps/hybrid), as a Git submodule.
- [ariadne-cps/neural-set](https://github.com/ariadne-cps/neural-set), for validated neural evaluation and constrained images.
- [ariadne-cps/build](https://github.com/ariadne-cps/build), for shared CMake configuration.
- [ariadne-cps/testing](https://github.com/ariadne-cps/testing), pinned at `ss/tst`, for C++ test fixtures, assertions and diagnostics only.

Hybrid provides the lower-level Ariadne components used by System. C++ behavioral suites use independent Ariadne Testing cases; lightweight header and return-code checks retain their original execution.

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

When a compatible Python development environment is available, the standalone build produces the combined `pyariadne_system` extension from Hybrid and System bindings. Run the smoke tests with `ctest --test-dir build -L python --output-on-failure`; they exercise real System objects.

## Contribution guidelines

If you would like to contribute to Ariadne System, please contact the developer:

- Luca Geretti <luca.geretti@univr.it>

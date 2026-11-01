# Ariadne System tutorial

The C++ and Python tutorials construct the same hybrid automaton, a
continuous-time `VectorField`, and a discrete-time `IteratedMap`. Both
languages check the dimensions and print the models.

The hybrid automaton deliberately uses no continuous state variables. Transitions
therefore need no reset assignments, and both languages can use the same
System functionality without disguising any missing bindings. The examples
define a system; they do not simulate its evolution.

## Build the library

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel --target everything
cmake --install build --prefix install
```

Building the combined Python extension requires Python >= 3.11 with
development headers. Install it with the command above.

## C++ tutorial

Build separately against the installed library:

```bash
cmake -S tutorial -B tutorial/build -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build tutorial/build --parallel
./tutorial/build/tutorial_system
```

On Windows the executable is `tutorial/build/tutorial_system.exe`. Depending
on your platform, add the installation's `lib` directory (or `bin` on
Windows) to the dynamic-library search path before running.

## Python tutorial

Run the **installed** `pyariadne` extension. For a local prefix, set
`PYTHONPATH` to `install/lib/pythonX.Y/site-packages` (or
`install/Lib/site-packages` on Windows), and make sure shared libraries are
accessible. Then run:

```bash
python tutorial/tutorial_system.py
```

The Python script is standalone; it does not require pytest. CI checks both
installed tutorials on macOS, Ubuntu, and Windows Release configurations,
while the Python smoke test also runs under Debug.

# CS598APE HW3

`sandpile.cpp` contains the implementation of finding the identity element of a 2D abelian sandpile.

![Identity sandpile on a 1001x1001 pacman grid](identity-ref.png)

## Building and Running

This project uses Halide as a submodule.
Make sure these are initialized, e.g.

```
git submodule update --init
```

We use `cmake` as the build system.
To compile the program, run:

```bash
cmake -S . -B build && cmake --build build
```

To clean existing build artifacts, run:

```bash
rm -rf build
```

Usage (after compiling):

```bash
./build/sandpile -n <N> [-r]
```

where `N` is the grid size.
Add the `-r` flag it it should be rendered.

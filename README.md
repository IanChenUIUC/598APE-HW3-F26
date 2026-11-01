# CS598APE HW3

This repository contains code for homework 3 of CS598APE. It is relatively
simple in comparison to HW1 and HW2 to ensure you have enough time to work on
the course project. 

`sandpile.c` contains the implementation of finding the identity element of a 2D
abelian sandpile.

![Identity sandpile on a 1001x1001 pacman grid](identity-ref.png)

*Identity sandpile on a 1001x1001 pacman grid*

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

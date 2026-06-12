# aig-lite

A minimal And-Inverter Graph engine written in C++17.

## Building

Requires a C++17 compiler (GCC or Clang).

```bash
cmake -S . -B build
cmake --build build
```

## Running tests

```bash
ctest --test-dir build --output-on-failure
```

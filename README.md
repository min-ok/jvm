# VM

## Build

```
cmake -S . -B build
cmake --build build
```

## Run

```
./build/vm
```

## Commiting

```
clang-format -i src/*/*.cpp src/*/*.h src/*.cpp
```

## Structure

```
docs/          isa.md - instruction set, format.md - program file format
src/core/      values, instructions, program, errors, VM
src/loader/    reads a program file into a Program
src/main.cpp   entry point
tests/         test programs
```

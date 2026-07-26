# Clang-Architecture

`clang-architecture` is a small utilitary based on [Clang](https://clang.llvm.org/)/[LLVM](https://llvm.org/) that aim
to extract and visualize the dependencies in a C/C++ program at the symbol level.

## Architecture

### Requirements

1. Your project can compile with the `clang` compiler.
2. Your project should have a compilation database (`compile_commands.json`)
    - Using CMake, it can be generated using `CMAKE_EXPORT_COMPILE_COMMANDS=ON`

### Usage

Generate an architecture report:

```bash
clang-architecture 
  -o=./architecture.json
  -p=./build/ 
  --extra-arg=-resource-dir=$(clang -print-resource-dir) 
  ./source/*.cpp
```

## Viewer

### Build

The viewer is a simple web application that can be used to visualize the architecture report.
It can be used locally without any server.

However, you need to build the JavaScript sources into a bundle before using it.

```bash
mkdir ./viewer/dist
esbuild ./viewer/javascript/main.js \
    --bundle \
    --format=iife \
    --outfile="./viewer/dist/main.js" \
    --target=es2020 \
    --minify
```

### Usage

You can use the viewer simply by opening the `viewer/dist/index.html` file in a web browser.

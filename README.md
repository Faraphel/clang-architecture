# Clang-Architecture

`clang-architecture` is a small utilitary based on [Clang](https://clang.llvm.org/)/[LLVM](https://llvm.org/) that aim
to extract and visualize the dependencies in a C/C++ program at the symbol level.

## Usage

Generate an architecture report:

```bash
clang-architecture 
  -o=./architecture.json
  -p=./build/ 
  --extra-arg=-resource-dir=$(clang -print-resource-dir) 
  ./source/*.cpp
```

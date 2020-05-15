# VC Intrinsics

## Introduction

VC Intrinsics is a set of new intrinsics on top of code LLVM IR instructions
that represent SIMD semantics of a program targeting GPU. This set is now
used by
* CMC   https://github.com/intel/cm-compiler
* DPC++ https://github.com/intel/llvm
* ISPC  https://github.com/ispc/ispc
frontend compilers and
* IGC backend - https://github.com/intel/intel-graphics-compiler

## License

VC intrinsics is distributed under the MIT license.

You may obtain a copy of the License at:

https://opensource.org/licenses/MIT

## Dependencies

* LLVM Project -  https://github.com/llvm/llvm-project

## Building

cmake -DLLVM_DIR=<llvm install>/lib/cmake/llvm -DCMAKE_INSTALL_PREFIX=<install> <vc-intrinsics>
cmake --build . --target install

## How to provide feedback

Please submit an issue using native github.com interface: https://github.com/intel/vc-intrinsics/issues.

## How to contribute

Create a pull request on github.com with your patch. A maintainer
will contact you if there are questions or concerns.

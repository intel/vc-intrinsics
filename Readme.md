# VC Intrinsics

## Introduction

VC Intrinsics project contains a set of new intrinsics on top of core
LLVM IR instructions that represent SIMD semantics of a program
targeting GPU. This set is now used by

* CMC - https://github.com/intel/cm-compiler
* DPC++ - https://github.com/intel/llvm
* ISPC - https://github.com/ispc/ispc

frontend compilers and

* IGC VC backend - https://github.com/intel/intel-graphics-compiler

## License

VC Intrinsics are distributed under the MIT license.

You may obtain a copy of the License at:

https://opensource.org/licenses/MIT

## Dependencies

### Source code

* LLVM Project - https://github.com/llvm/llvm-project

### Tools

To build libraries:

* CMake - https://cmake.org/ - 3.13.4 or later
* Python - https://www.python.org/ - 2.7 or later
* C++ compiler - anything that can compile LLVM

To build documentation:

* Sphinx - https://www.sphinx-doc.org - 1.5 or later
* GNU Make - https://www.gnu.org/software/make/ - 3.79 or later
* Standard Unix utilities (mkdir, rm, sed)

## Building

VC Intrinsics can be built in two major modes: in-tree and
external. All major LLVM versions starting from LLVM 7 are supported.

LLVM ToT can be used too, but there is no guarantee that it will
always work (because of sudden breaking changes in LLVM C++
API). However, maintainers are trying to fix such issues as fast as
possible.

### In-tree build

For in-tree build VC Intrinsics can be considered as an external LLVM
project. Put VC Intrinsics source directory inside `llvm/projects`
directory or add `-DLLVM_EXTERNAL_PROJECTS="vc-intrinsics"
-DLLVM_EXTERNAL_VC_INTRINSICS_SOURCE_DIR=<vc-intrinsics>` to cmake
command arguments when configuring LLVM.

### External build

To configure VC Intrinsics with prebuilt LLVM run cmake as follows:

```shell
$ cmake -DLLVM_DIR=<llvm install>/lib/cmake/llvm <vc-intrinsics>
```

### Documentation

VC Intrinsics documentation is inside `docs` subdirectory and can be
built using Sphinx. To build html version do the following:

```shell
$ cd docs
$ make -f Makefile.sphinx html
```

This will extract comments from main intrinsics description and
generate readable html output in `_build/html` subdirectory.

## Testing

VC Intrinsics repository contains lit tests that are enabled when
`-DVC_INTR_ENABLE_LIT_TESTS=ON` is passed to cmake command. Lit tests
use LLVM plugins and currently are supported only with dynamic LLVM
(when LLVM is configured with `-DLLVM_LINK_LLVM_DYLIB=ON`). In
external build path to `lit` utility should be specified as follows:
`-DLLVM_EXTERNAL_LIT=<lit>`. Full example with external build:

```shell
$ cmake -DLLVM_DIR=<llvm install>/lib/cmake/llvm -DVC_INTR_ENABLE_LIT_TESTS=ON -DLLVM_EXTERNAL_LIT=<lit> <vc-intrinsics>
```

Target `check-vc-intrinsics` will run lit tests.

## How to provide feedback

Please submit an issue using native github.com interface:
https://github.com/intel/vc-intrinsics/issues.

## How to contribute

Create a pull request on github.com with your patch. A maintainer
will contact you if there are questions or concerns.

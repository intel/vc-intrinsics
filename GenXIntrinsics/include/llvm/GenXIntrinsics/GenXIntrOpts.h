/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

SPDX-License-Identifier: MIT
============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

// This header file defines prototypes for accessor functions that expose passes
// in the GenX Intrinsics transformations library.

#ifndef LLVM_GENX_INTR_OPTS_H
#define LLVM_GENX_INTR_OPTS_H

namespace llvm {

class FunctionPass;
class ModulePass;
class Pass;

//===----------------------------------------------------------------------===//
//
// CMSimdCFLowering - Lower CM SIMD control flow
//
Pass *createCMSimdCFLoweringPass();
Pass *createISPCSimdCFLoweringPass();

//===----------------------------------------------------------------------===//
//
// GenXRestoreIntrAttr - Restore Intrinsics' Attributes
//
Pass *createGenXRestoreIntrAttrPass();

} // End llvm namespace

#endif


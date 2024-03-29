/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

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


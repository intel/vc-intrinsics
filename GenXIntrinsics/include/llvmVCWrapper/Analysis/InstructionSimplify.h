/*========================== begin_copyright_notice ============================

INTEL CONFIDENTIAL

Copyright (C) 2022 Intel Corporation

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they were
provided to you ("License"). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit this
software or the related documents without Intel's prior written permission.

This software and the related documents are provided as is, with no express or
implied warranties, other than those that are expressly stated in the License.

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_ANALYSIS_INSTRUCTIONSIMPLIFY_H
#define VCINTR_ANALYSIS_INSTRUCTIONSIMPLIFY_H

#include <llvm/Analysis/InstructionSimplify.h>

namespace VCINTR {

inline llvm::Value *SimplifyInsertElementInst(llvm::Value *Vec,
                                              llvm::Value *Elt,
                                              llvm::Value *Idx,
                                              const llvm::SimplifyQuery &Q) {
#if VC_INTR_LLVM_VERSION_MAJOR <= 14
  return llvm::SimplifyInsertElementInst(Vec, Elt, Idx, Q);
#else
  return llvm::simplifyInsertElementInst(Vec, Elt, Idx, Q);
#endif
}

inline llvm::Value *SimplifyExtractElementInst(llvm::Value *Vec,
                                               llvm::Value *Idx,
                                               const llvm::SimplifyQuery &Q) {
#if VC_INTR_LLVM_VERSION_MAJOR <= 14
  return llvm::SimplifyExtractElementInst(Vec, Idx, Q);
#else
  return llvm::simplifyExtractElementInst(Vec, Idx, Q);
#endif
}

inline llvm::Value *SimplifyCastInst(unsigned CastOpc, llvm::Value *Op,
                                     llvm::Type *Ty,
                                     const llvm::SimplifyQuery &Q) {
#if VC_INTR_LLVM_VERSION_MAJOR <= 14
  return llvm::SimplifyCastInst(CastOpc, Op, Ty, Q);
#else
  return llvm::simplifyCastInst(CastOpc, Op, Ty, Q);
#endif
}

} // namespace VCINTR

#endif // VCINTR_ANALYSIS_INSTRUCTIONSIMPLIFY_H

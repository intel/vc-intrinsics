/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "llvm/GenXIntrinsics/GenXMetadata.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>

using namespace llvm;

MDNode *llvm::genx::GetOldStyleKernelMD(Function const &F) {
  auto *KernelMD = static_cast<MDNode *>(nullptr);
  auto *KernelMDs = F.getParent()->getNamedMetadata(FunctionMD::GenXKernels);
  if (!KernelMDs)
    return KernelMD;

  for (unsigned I = 0, E = KernelMDs->getNumOperands(); I < E; ++I) {
    auto *Kernel = mdconst::dyn_extract<Function>(
        KernelMDs->getOperand(I)->getOperand(KernelMDOp::FunctionRef));
    if (Kernel == &F) {
      KernelMD = KernelMDs->getOperand(I);
      break;
    }
  }
  return KernelMD;
}

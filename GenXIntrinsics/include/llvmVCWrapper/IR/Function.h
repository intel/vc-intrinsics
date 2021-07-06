/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_FUNCTION_H
#define VCINTR_IR_FUNCTION_H

#include <llvm/IR/Function.h>

namespace VCINTR {

namespace Function {

inline llvm::Function *Create(llvm::FunctionType *FTy,
                              llvm::Function::LinkageTypes Linkage,
                              unsigned AddressSpace, const llvm::Twine &N = "",
                              llvm::Module *M = nullptr) {
#if VC_INTR_LLVM_VERSION_MAJOR <= 7
  // Let's stick to newer LLVM versions interface.
  (void)AddressSpace;
  return llvm::Function::Create(FTy, Linkage, N, M);
#else
  return llvm::Function::Create(FTy, Linkage, AddressSpace, N, M);
#endif
}

} // namespace Function

} // namespace VCINTR

#endif // VCINTR_IR_GLOBALVARIABLE_H

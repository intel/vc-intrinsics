/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_INSTRUCTIONS_H
#define VCINTR_IR_INSTRUCTIONS_H

#include <algorithm>
#include <llvm/IR/Instructions.h>

namespace VCINTR {
namespace ShuffleVectorInst {
auto static constexpr UndefMaskElem = -1;

// LLVM <= 10 does not have ShuffleVectorInst ctor which accepts ArrayRef<int>
// This method returns mask with appropriate type for ShuffleVectorInst ctor
#if VC_INTR_LLVM_VERSION_MAJOR <= 10
static llvm::Constant *getShuffleMask(llvm::ArrayRef<int> Mask,
                                      llvm::LLVMContext &Context) {
  using namespace llvm;
  auto Indices = SmallVector<llvm::Constant *, 8>{};
  auto *Int32Ty = IntegerType::getInt32Ty(Context);
  std::transform(Mask.begin(), Mask.end(), std::back_inserter(Indices),
                 [&](int El) -> llvm::Constant * {
                   if (El == UndefMaskElem)
                     return UndefValue::get(Int32Ty);
                   else
                     return ConstantInt::get(Int32Ty, El);
                 });
  return ConstantVector::get(Indices);
}
#else
static llvm::ArrayRef<int> getShuffleMask(llvm::ArrayRef<int> Mask,
                                          llvm::LLVMContext &Context) {
  return Mask;
}
#endif

} // namespace VCINTR

} // namespace VCINTR

#endif // VCINTR_IR_INSTRUCTIONS_H

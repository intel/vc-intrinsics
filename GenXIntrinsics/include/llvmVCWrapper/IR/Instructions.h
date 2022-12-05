/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_INSTRUCTIONS_H
#define VCINTR_IR_INSTRUCTIONS_H

#include <algorithm>
#include <llvm/ADT/Optional.h>
#include <llvm/IR/Instructions.h>

namespace VCINTR {
namespace ShuffleVectorInst {
auto static constexpr UndefMaskElem = -1;

// LLVM <= 10 does not have ShuffleVectorInst ctor which accepts ArrayRef<int>
// This method returns mask with appropriate type for ShuffleVectorInst ctor
#if VC_INTR_LLVM_VERSION_MAJOR <= 10
inline llvm::Constant *getShuffleMask(llvm::ArrayRef<int> Mask,
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
inline llvm::ArrayRef<int> getShuffleMask(llvm::ArrayRef<int> Mask,
                                          llvm::LLVMContext &Context) {
  return Mask;
}
#endif

} // namespace ShuffleVectorInst

template <class ArgKind>
inline ArgKind &getValue(llvm::Optional<ArgKind> &opt) {
#if VC_INTR_LLVM_VERSION_MAJOR < 15
  return opt.getValue();
#else
  return opt.value();
#endif
}

template <class ArgKind>
inline const ArgKind &getValue(const llvm::Optional<ArgKind> &opt) {
#if VC_INTR_LLVM_VERSION_MAJOR < 15
  return opt.getValue();
#else
  return opt.value();
#endif
}
} // namespace VCINTR

#endif // VCINTR_IR_INSTRUCTIONS_H

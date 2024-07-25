/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_FUNCTION_H
#define VCINTR_IR_FUNCTION_H

#include <llvm/IR/Function.h>

namespace VCINTR {

namespace Function {

inline llvm::Argument *getArg(const llvm::Function &F, unsigned ArgNo) {
  assert(F.arg_size() > ArgNo);
  llvm::Argument *Arg = nullptr;

#if LLVM_VERSION_MAJOR < 10
  // similar to lvm::Function::getArg implementation
  auto ArgIt = F.arg_begin();
  std::advance(ArgIt, ArgNo);
  Arg = const_cast<llvm::Argument *>(&*ArgIt);
#else
  Arg = F.getArg(ArgNo);
#endif

  return Arg;
}

inline void addAttributeAtIndex(llvm::Function &F, unsigned Index,
                                llvm::Attribute Attr) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  F.addAttributeAtIndex(Index, Attr);
#else
  F.addAttribute(Index, Attr);
#endif
}

inline void removeAttributeAtIndex(llvm::Function &F, unsigned Index,
                                   llvm::Attribute::AttrKind Kind) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  F.removeAttributeAtIndex(Index, Kind);
#else
  F.removeAttribute(Index, Kind);
#endif
}

inline void removeAttributeAtIndex(llvm::Function &F, unsigned Index,
                                   llvm::StringRef Kind) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  F.removeAttributeAtIndex(Index, Kind);
#else
  F.removeAttribute(Index, Kind);
#endif
}

} // namespace Function

} // namespace VCINTR

#endif // VCINTR_IR_GLOBALVARIABLE_H

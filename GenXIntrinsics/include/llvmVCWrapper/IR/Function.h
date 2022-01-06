/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_FUNCTION_H
#define VCINTR_IR_FUNCTION_H

#include <llvm/IR/Function.h>

namespace VCINTR {

namespace Function {

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

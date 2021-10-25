/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_ALIGNMENT_H
#define VCINTR_IR_ALIGNMENT_H

#include <llvm/IR/Instructions.h>

namespace VCINTR {

namespace Align {

#if VC_INTR_LLVM_VERSION_MAJOR <= 9
template <class TValue> unsigned getAlign(TValue *Val) {
  return Val->getAlignment();
}
#elif VC_INTR_LLVM_VERSION_MAJOR <= 10
template <class TValue> llvm::MaybeAlign getAlign(TValue *Val) {
  // LLVM 10 instructions accept MaybeAlign but do not provide
  // getMaybeAlignMethod
  return llvm::MaybeAlign(Val->getAlignment());
}
#else
template <class TValue> auto getAlign(TValue *Val) {
  return Val->getAlign();
}
#endif

} // namespace Align

} // namespace VCINTR

#endif // VCINTR_IR_ALIGNMENT_H

/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_GLOBALVALUE_H
#define VCINTR_IR_GLOBALVALUE_H

#include <llvm/IR/GlobalValue.h>

namespace VCINTR {

namespace GlobalValue {

inline unsigned getAddressSpace(const llvm::GlobalValue &GV) {
#if VC_INTR_LLVM_VERSION_MAJOR <= 7
  return GV.getType()->getAddressSpace();
#else
  return GV.getAddressSpace();
#endif
}

} // namespace GlobalValue

} // namespace VCINTR

#endif // VCINTR_IR_GLOBALVARIABLE_H

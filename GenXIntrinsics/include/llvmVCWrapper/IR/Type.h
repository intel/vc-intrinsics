/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_TYPE_H
#define VCINTR_IR_TYPE_H

#include <llvm/IR/Type.h>

namespace VCINTR {
namespace Type {

inline llvm::Type *getNonOpaquePtrEltTy(const llvm::Type *PTy) {
#if VC_INTR_LLVM_VERSION_MAJOR < 14
  return PTy->getPointerElementType();
#elif VC_INTR_LLVM_VERSION_MAJOR < 17
  return PTy->getNonOpaquePointerElementType();
#else
  llvm_unreachable("Pointers no longer have element types");
#endif
}

} // namespace Type
} // namespace VCINTR

#endif // VCINTR_IR_TYPE_H
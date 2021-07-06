/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_INTRINSICS_H
#define VCINTR_IR_INTRINSICS_H

#include <llvm/IR/Intrinsics.h>

namespace VCINTR {

namespace Intrinsic {
inline std::string getName(llvm::Intrinsic::ID Id,
                           llvm::ArrayRef<llvm::Type *> Tys) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 13
  return llvm::Intrinsic::getNameNoUnnamedTypes(Id, Tys);
#else
  return llvm::Intrinsic::getName(Id, Tys);
#endif
}

} // namespace Intrinsic
} // namespace VCINTR

#endif // VCINTR_IR_INTRINSICS_H

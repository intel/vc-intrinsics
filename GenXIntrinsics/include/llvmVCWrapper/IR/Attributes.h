/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_ATTRIBUTES_H
#define VCINTR_IR_ATTRIBUTES_H

#include <llvm/IR/Attributes.h>

namespace VCINTR {

namespace AttributeList {

inline bool hasFnAttr(const llvm::AttributeList &AttrList,
                      llvm::Attribute::AttrKind Kind) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  return AttrList.hasFnAttr(Kind);
#else
  return AttrList.hasFnAttribute(Kind);
#endif
}

inline bool hasFnAttr(const llvm::AttributeList &AttrList,
                      llvm::StringRef Kind) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  return AttrList.hasFnAttr(Kind);
#else
  return AttrList.hasFnAttribute(Kind);
#endif
}

} // namespace AttributeList

} // namespace VCINTR

#endif // VCINTR_IR_ATTRIBUTES_H

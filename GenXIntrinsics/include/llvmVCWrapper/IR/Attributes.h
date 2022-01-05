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

inline bool hasAttributeAtIndex(const llvm::AttributeList &AttrList,
                                unsigned Index,
                                llvm::Attribute::AttrKind Kind) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  return AttrList.hasAttributeAtIndex(Index, Kind);
#else
  return AttrList.hasAttribute(Index, Kind);
#endif
}

inline bool hasAttributeAtIndex(const llvm::AttributeList &AttrList,
                                unsigned Index, llvm::StringRef Kind) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  return AttrList.hasAttributeAtIndex(Index, Kind);
#else
  return AttrList.hasAttribute(Index, Kind);
#endif
}

inline llvm::Attribute getAttributeAtIndex(const llvm::AttributeList &AttrList,
                                           unsigned Index,
                                           llvm::Attribute::AttrKind Kind) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  return AttrList.getAttributeAtIndex(Index, Kind);
#else
  return AttrList.getAttribute(Index, Kind);
#endif
}

inline llvm::Attribute getAttributeAtIndex(const llvm::AttributeList &AttrList,
                                           unsigned Index,
                                           llvm::StringRef Kind) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  return AttrList.getAttributeAtIndex(Index, Kind);
#else
  return AttrList.getAttribute(Index, Kind);
#endif
}

inline llvm::AttributeList
removeAttributeAtIndex(llvm::LLVMContext &C,
                       const llvm::AttributeList &AttrList, unsigned Index,
                       llvm::Attribute::AttrKind Kind) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  return AttrList.removeAttributeAtIndex(C, Index, Kind);
#else
  return AttrList.removeAttribute(C, Index, Kind);
#endif
}

inline llvm::AttributeList
removeAttributeAtIndex(llvm::LLVMContext &C,
                       const llvm::AttributeList &AttrList, unsigned Index,
                       llvm::StringRef Kind) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  return AttrList.removeAttributeAtIndex(C, Index, Kind);
#else
  return AttrList.removeAttribute(C, Index, Kind);
#endif
}

inline llvm::AttributeList
removeAttributesAtIndex(llvm::LLVMContext &C,
                        const llvm::AttributeList &AttrList, unsigned Index,
                        const llvm::AttributeMask &AttrsToRemove) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  return AttrList.removeAttributesAtIndex(C, Index, AttrsToRemove);
#else
  return AttrList.removeAttributes(C, Index, AttrsToRemove);
#endif
}

} // namespace AttributeList

} // namespace VCINTR

#endif // VCINTR_IR_ATTRIBUTES_H

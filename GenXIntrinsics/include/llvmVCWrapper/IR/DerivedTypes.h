/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_DERIVEDYPES_H
#define VCINTR_IR_DERIVEDYPES_H

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>

namespace VCINTR {
// TODO: move this to namespace VectorType and rename to "get"
#if VC_INTR_LLVM_VERSION_MAJOR >= 9
  inline llvm::VectorType *getVectorType(llvm::Type *ElementType,
                                         llvm::ElementCount EC) {
    return llvm::VectorType::get(ElementType, EC);
  }
#endif

  inline llvm::VectorType *getVectorType(llvm::Type *ElementType,
                                         unsigned NumElements) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 11
    return llvm::VectorType::get(ElementType, NumElements, false /*Scalable*/);
#else
    return llvm::VectorType::get(ElementType, NumElements);
#endif
  }

  inline llvm::StructType *getTypeByName(llvm::Module *M,
                                         llvm::StringRef Name) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 12
    return llvm::StructType::getTypeByName(M->getContext(), Name);
#else
    return M->getTypeByName(Name);
#endif
  }

namespace VectorType {

inline unsigned getNumElements(llvm::VectorType *VecType) {
  using namespace llvm;
#if VC_INTR_LLVM_VERSION_MAJOR <= 10
  return VecType->getNumElements();
#else
  auto *FixedVecType = cast<FixedVectorType>(VecType);
  return FixedVecType->getNumElements();
#endif
}

} // namespace VectorType
namespace PointerType {

inline llvm::PointerType *getWithSamePointeeType(llvm::PointerType *PT,
                                                 unsigned AddressSpace) {
#if VC_INTR_LLVM_VERSION_MAJOR < 14
  return llvm::PointerType::get(PT->getElementType(), AddressSpace);
#elif VC_INTR_LLVM_VERSION_MAJOR < 17
  return llvm::PointerType::getWithSamePointeeType(PT, AddressSpace);
#else
  return llvm::PointerType::get(PT->getContext(), AddressSpace);
#endif
}

} // namespace PointerType
} // namespace VCINTR

#endif // VCINTR_IR_DERIVEDYPES_H

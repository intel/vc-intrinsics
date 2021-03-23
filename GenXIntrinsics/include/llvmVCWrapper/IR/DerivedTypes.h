/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/


#ifndef VCINTR_IR_DERIVEDYPES_H
#define VCINTR_IR_DERIVEDYPES_H

#include <llvm/IR/DerivedTypes.h>

namespace VCINTR {
// TODO: move this to namespace VectorType and rename to "get"
#if VC_INTR_LLVM_VERSION_MAJOR >= 9
  static inline llvm::VectorType *getVectorType(llvm::Type *ElementType,
                                                llvm::ElementCount EC) {
    return llvm::VectorType::get(ElementType, EC);
  }
#endif

  static inline llvm::VectorType *getVectorType(llvm::Type *ElementType,
                                                unsigned NumElements) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 11
    return llvm::VectorType::get(ElementType, NumElements, false /*Scalable*/);
#else
    return llvm::VectorType::get(ElementType, NumElements);
#endif
  }

  static inline llvm::StructType *getTypeByName(llvm::Module *M,
                                                llvm::StringRef Name) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 12
    return llvm::StructType::getTypeByName(M->getContext(), Name);
#else
    return M->getTypeByName(Name);
#endif
  }

namespace VectorType {

static unsigned getNumElements(llvm::VectorType *VecType) {
  using namespace llvm;
#if VC_INTR_LLVM_VERSION_MAJOR <= 10
  return VecType->getNumElements();
#else
  auto *FixedVecType = cast<FixedVectorType>(VecType);
  return FixedVecType->getNumElements();
#endif
}

} // namespace VectorType
}

#endif // VCINTR_IR_DERIVEDYPES_H

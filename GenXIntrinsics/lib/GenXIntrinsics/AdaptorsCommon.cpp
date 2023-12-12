/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "AdaptorsCommon.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

#include "llvmVCWrapper/IR/Type.h"

namespace llvm {
namespace genx {
#if VC_INTR_LLVM_VERSION_MAJOR >= 9

static void legalizeAttribute(Argument &Arg, Type *NewType,
                              Attribute::AttrKind Kind) {

  if (!Arg.hasAttribute(Kind) ||
      Arg.getAttribute(Kind).getValueAsType() == NewType)
    return;

  Arg.removeAttr(Kind);
  Arg.addAttr(Attribute::get(Arg.getParent()->getContext(), Kind, NewType));
}

#endif

void legalizeParamAttributes(Function *F) {
  assert(F && "Valid function ptr must be passed");

#if VC_INTR_LLVM_VERSION_MAJOR >= 9
  for (auto &Arg : F->args()) {
    auto *PTy = dyn_cast<PointerType>(Arg.getType());
    if (!PTy)
      continue;

#if VC_INTR_LLVM_VERSION_MAJOR >= 13
#if VC_INTR_LLVM_VERSION_MAJOR < 18
    if (PTy->isOpaque())
#endif // VC_INTR_LLVM_VERSION_MAJOR < 18
      continue;
#endif // VC_INTR_LLVM_VERSION_MAJOR >= 13

    auto *ElemType = VCINTR::Type::getNonOpaquePtrEltTy(PTy);

    legalizeAttribute(Arg, ElemType, Attribute::ByVal);

#if VC_INTR_LLVM_VERSION_MAJOR >= 11
    legalizeAttribute(Arg, ElemType, Attribute::Preallocated);
#if VC_INTR_LLVM_VERSION_MAJOR >= 12
    legalizeAttribute(Arg, ElemType, Attribute::ByRef);
#if VC_INTR_LLVM_VERSION_MAJOR >= 13
    legalizeAttribute(Arg, ElemType, Attribute::InAlloca);
    legalizeAttribute(Arg, ElemType, Attribute::ElementType);
#endif // VC_INTR_LLVM_VERSION_MAJOR >= 13
#endif // VC_INTR_LLVM_VERSION_MAJOR >= 12
#endif // VC_INTR_LLVM_VERSION_MAJOR >= 11
  }
#endif // VC_INTR_LLVM_VERSION_MAJOR >= 9
}
} // namespace genx
} // namespace llvm

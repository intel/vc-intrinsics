/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "AdaptorsCommon.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

#include "llvmVCWrapper/IR/Type.h"

namespace llvm {
namespace genx {
#if VC_INTR_LLVM_VERSION_MAJOR >= 9

static void legalizeAttribute(Argument &Arg, Type *NewType,
                              Attribute::AttrKind Kind) {
  if (!Arg.hasAttribute(Kind))
    return;
  auto *AttrType = Arg.getAttribute(Kind).getValueAsType();
  if (AttrType == NewType)
    return;

#if VC_INTR_LLVM_VERSION_MAJOR >= 14
  if (!AttrType->isIntegerTy(8)) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 17
    return;
#else // VC_INTR_LLVM_VERSION_MAJOR >= 17
  if (cast<PointerType>(Arg.getType())->isOpaque())
    return;
#endif // VC_INTR_LLVM_VERSION_MAJOR >= 17
  }
#endif // VC_INTR_LLVM_VERSION_MAJOR >= 14

  Arg.removeAttr(Kind);
  Arg.addAttr(Attribute::get(Arg.getParent()->getContext(), Kind, NewType));
}

#endif

Type *getPtrElemType(Value *V) {
#if VC_INTR_LLVM_VERSION_MAJOR < 14
  return VCINTR::Type::getNonOpaquePtrEltTy(V->getType());
#else // VC_INTR_LLVM_VERSION_MAJOR < 14
#if VC_INTR_LLVM_VERSION_MAJOR < 17
  auto *PtrTy = cast<PointerType>(V->getType());
  if (!PtrTy->isOpaque())
    return VCINTR::Type::getNonOpaquePtrEltTy(PtrTy);
#endif // VC_INTR_LLVM_VERSION_MAJOR < 17
  auto *Int8Ty = Type::getInt8Ty(V->getContext());
  SmallPtrSet<Type *, 2> ElemTys;
  SmallVector<Value *, 4> Stack;
  Stack.push_back(V);
  while (!Stack.empty()) {
    auto* Current = Stack.back();
    Stack.pop_back();
    for (auto *U : Current->users()) {
      if (ElemTys.size() > 1)
        return Int8Ty;
      auto *I = dyn_cast<Instruction>(U);
      if (!I)
        continue;
      if (auto *LI = dyn_cast<LoadInst>(I)) {
        if (Current == LI->getPointerOperand())
          ElemTys.insert(LI->getType());
      } else if (auto *SI = dyn_cast<StoreInst>(I)) {
        if (Current == SI->getPointerOperand())
          ElemTys.insert(SI->getValueOperand()->getType());
      } else if (auto *GEPI = dyn_cast<GetElementPtrInst>(I)) {
        if (Current == GEPI->getPointerOperand())
          ElemTys.insert(GEPI->getSourceElementType());
      } else if (isa<BitCastInst>(I) || isa<AddrSpaceCastInst>(I)) {
        Stack.push_back(I);
      }
    }
  }
  return ElemTys.empty() ? Int8Ty : *ElemTys.begin();
#endif // VC_INTR_LLVM_VERSION_MAJOR < 14
}

void legalizeParamAttributes(Function *F) {
  assert(F && "Valid function ptr must be passed");

#if VC_INTR_LLVM_VERSION_MAJOR >= 9
  for (auto &Arg : F->args()) {
    auto *PTy = dyn_cast<PointerType>(Arg.getType());
    if (!PTy)
      continue;

    auto *ElemType = getPtrElemType(&Arg);

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

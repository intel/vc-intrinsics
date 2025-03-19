/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This file defines common functions for rewriting single element vectors
// in GenXSPIRV adaptors.

#include "GenXSingleElementVectorUtil.h"

#include "llvm/GenXIntrinsics/GenXIntrinsics.h"
#include "llvm/GenXIntrinsics/GenXMetadata.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"

#include "llvmVCWrapper/Analysis/InstructionSimplify.h"
#include "llvmVCWrapper/IR/Attributes.h"
#include "llvmVCWrapper/Support/Alignment.h"


namespace llvm {
namespace genx {
/// This section contains some arbitrary constants

// Default size for arguments of SEV-free version ShuffleVector instruction
static unsigned constexpr ShuffleVectorSize = 2;

/// This section contains general utils:
///  * For safe iteration over functions and instructions
///  * For vectors operations such as converting constant vector element from
///    llvm::Value to int
///  * For examining pointer types
/// These utils are used across this module but they do not contain
/// any design solutions for removing Single Element Vectors (SEVs)

// Functions with SEVs are deleted from module
// This util allows to continue iteration even after deletion
std::vector<Function *> SEVUtil::getFunctions() {
  auto Functions = std::vector<Function *>{};
  std::transform(M.begin(), M.end(), std::back_inserter(Functions),
                 [](Function &F) { return &F; });
  return Functions;
}

// Globals with SEVs are deleted from module
// This util allows to continue iteration even after deletion
std::vector<GlobalVariable *> SEVUtil::getGlobalVariables() {
  auto Globals = std::vector<GlobalVariable *>{};
  std::transform(M.global_begin(), M.global_end(), std::back_inserter(Globals),
                 [](GlobalVariable &GV) { return &GV; });
  return Globals;
}

// Instructions with SEVs are deleted from module
// This util allows to continue iteration even after deletion
std::vector<Instruction *> SEVUtil::getInstructions(Function &F) {
  auto Instructions = std::vector<Instruction *>{};
  for (auto &&BB : F) {
    std::transform(BB.begin(), BB.end(), std::back_inserter(Instructions),
                   [](Instruction &I) { return &I; });
  }
  return Instructions;
}

// Returns requested vector index as Value*
// It is helpful for creating ExtractElementInst and InsertElementInst
ConstantInt *SEVUtil::getVectorIndex(size_t idx) {
  auto *ITy = IntegerType::getIntNTy(M.getContext(),
                                     M.getDataLayout().getPointerSizeInBits(0));
  return ConstantInt::get(ITy, idx, false);
}

// Returns underlying int from Value*
int64_t SEVUtil::getConstantElement(ConstantInt *Const) {
  assert(!isa<UndefValue>(Const));
  return Const->getSExtValue();
}

// For type U***** returns number of stars and type U in the second argument
size_t SEVUtil::getPointerNesting(Type *Ty, Type **ReturnNested) {
  auto NPtrs = size_t{0};
  auto *NestedTy = Ty;
  if (!VCINTR::Type::isOpaquePointerTy(Ty)) {
    while (isa<PointerType>(NestedTy)) {
      NestedTy = VCINTR::Type::getNonOpaquePtrEltTy(NestedTy);
      ++NPtrs;
    }
  }
  if (ReturnNested)
    *ReturnNested = NestedTy;
  return NPtrs;
}

// For type <n x U****>**** returns total number of stars and type U in the
// second argument
size_t SEVUtil::getPointerVectorNesting(Type *Ty, Type **ReturnNested) {
  Type *NestedTy = nullptr;
  auto Outer = getPointerNesting(Ty, &NestedTy);
  auto VTy = dyn_cast<VectorType>(NestedTy);
  if (!VTy) {
    if (ReturnNested)
      *ReturnNested = NestedTy;
    return Outer;
  }
  auto Inner = getPointerNesting(VTy->getElementType(), &NestedTy);
  if (ReturnNested)
    *ReturnNested = NestedTy;
  return Outer + Inner;
}

// For type <n x U****>**** returns number of stars inside vector
size_t SEVUtil::getInnerPointerVectorNesting(Type *Ty) {
  auto Total = getPointerVectorNesting(Ty);
  auto Outer = getPointerNesting(Ty);
  assert(Total >= Outer);
  return Total - Outer;
}

/// This section contains core utils for Single Element Vectors:
/// * Convertion of types from SEV-rich to SEV-free and vice versa
/// * Detecting types which contain SEVs
/// * Creating intermediate instructions for conversion of SEV-rich and SEV-free
///   values
/// * Finalizing replacement of SEV-rich or SEV-free instruction with its
///   antipod

// Returns SEV-free analogue of Type Ty accordingly to the following scheme:
// <1 x U>**...* ---> U**...*
Type *SEVUtil::getTypeFreeFromSEV(Type *Ty) {
  if (VCINTR::Type::isOpaquePointerTy(Ty))
    return Ty;
  // Pointer types should be "undressed" first
  if (auto *Ptr = dyn_cast<PointerType>(Ty)) {
    auto UTy = getTypeFreeFromSEV(VCINTR::Type::getNonOpaquePtrEltTy(Ptr));
    if (UTy == VCINTR::Type::getNonOpaquePtrEltTy(Ptr))
      return Ptr;
    return PointerType::get(UTy, Ptr->getAddressSpace());
  } else if (auto *VecTy = dyn_cast<VectorType>(Ty)) {
    if (VCINTR::VectorType::getNumElements(VecTy) == 1)
      return VecTy->getElementType();
  } else if (auto *StructTy = dyn_cast<StructType>(Ty)) {
    // If there is a key for this struct type is in SEV-Free to SEV-Rich map it
    // means that the type is already SEV-Free
    if (SEVRichStructMap.find(StructTy) != SEVRichStructMap.end())
      return Ty;
    if (SEVFreeStructTypes.find(StructTy) != SEVFreeStructTypes.end())
      return Ty;
    auto It = SEVFreeStructMap.find(StructTy);
    if (It != SEVFreeStructMap.end())
      return It->second;
    // To handle circle dependencies we create opaque struct type and add it to
    // the map. If this struct or any nested one contains a pointer to the type
    // we are rewriting it will be automatically changed to this incomplete type
    // and traversing will stop
    StructType *NewStructTy = StructType::create(Ty->getContext());
    It = SEVFreeStructMap.insert(std::make_pair(StructTy, NewStructTy)).first;
    bool HasSEV = false;
    std::vector<Type *> NewElements;
    for (auto *ElemTy : StructTy->elements()) {
      Type *NewElemTy = getTypeFreeFromSEV(ElemTy);
      NewElements.push_back(NewElemTy);
      if (!HasSEV && NewElemTy != ElemTy) {
        // If new type is not equal to the old one it doesn't always mean that
        // there is a SEV element in the struct. It could be also temporary
        // unfininished (opaque) struct type or a pointer to it
        auto *TempTy = NewElemTy;
        while (auto *Ptr = dyn_cast<PointerType>(TempTy))
          TempTy = VCINTR::Type::getNonOpaquePtrEltTy(Ptr);
        if (auto *NestedStructTy = dyn_cast<StructType>(TempTy))
          HasSEV = !NestedStructTy->isOpaque();
        else
          HasSEV = true;
      }
    }
    if (HasSEV) {
      NewStructTy->setBody(NewElements);
      SEVRichStructMap.insert(std::make_pair(NewStructTy, StructTy));
      return NewStructTy;
    }
    SEVFreeStructMap.erase(It);
    SEVFreeStructTypes.insert(StructTy);
  }
  return Ty;
}

// Returns SEV-rich analogue of Type Ty accordingly to the following scheme:
// U*...**...* ---> <1 x U*...*>*...*
Type *SEVUtil::getTypeWithSEV(Type *Ty, size_t InnerPointers) {
  if (VCINTR::Type::isOpaquePointerTy(Ty) && InnerPointers > 0)
    return Ty;
  if (auto *VecTy = dyn_cast<VectorType>(Ty)) {
    (void)VecTy;
    assert(InnerPointers == 0);
    assert(VCINTR::VectorType::getNumElements(VecTy) == 1 &&
           "Cannot put vector type inside another vector!");
    return Ty;
  } else if (auto *StructTy = dyn_cast<StructType>(Ty)) {
    auto It = SEVRichStructMap.find(StructTy);
    if (It == SEVRichStructMap.end())
      llvm_unreachable("Unexpected SEV StructType");
    return It->second;
  }
  auto NPtrs = getPointerNesting(Ty);

  assert(InnerPointers <= NPtrs);
  if (InnerPointers == NPtrs)
    return VCINTR::getVectorType(Ty, 1);

  auto *Ptr = cast<PointerType>(Ty);
  auto *UTy =
      getTypeWithSEV(VCINTR::Type::getNonOpaquePtrEltTy(Ptr), InnerPointers);
  return PointerType::get(UTy, Ptr->getAddressSpace());
}

// Returns true if Ty is SEV or it is a pointer to SEV
bool SEVUtil::hasSEV(Type *Ty) { return Ty != getTypeFreeFromSEV(Ty); }

// Returns true if Instruction type or type of any of its arguments has SEV
bool SEVUtil::hasSEV(Instruction *I) {
  if (hasSEV(I->getType()))
    return true;
  if (auto *AI = dyn_cast<AllocaInst>(I))
    if (hasSEV(AI->getAllocatedType()))
      return true;
  if (auto *GEPI = dyn_cast<GetElementPtrInst>(I))
    if (hasSEV(GEPI->getSourceElementType()))
      return true;
  return std::find_if(I->op_begin(), I->op_end(), [this](Use &Op) {
           return this->hasSEV(Op.get()->getType());
         }) != I->op_end();
}

// Returns true if return value or any of arguments have SEV
bool SEVUtil::doesSignatureHaveSEV(Function &F) {
  if (hasSEV(F.getReturnType()))
    return true;
  return std::find_if(F.arg_begin(), F.arg_end(), [this](Argument &Arg) {
           return this->hasSEV(Arg.getType());
         }) != F.arg_end();
}

// This util accepts SEV-rich Value and returns new, SEV-free one
// For pointer types it returns BitCastInst
// For constant vector it returns element of Vector
// For non-constant vectors it ExtractElementInst
Value *SEVUtil::createVectorToScalarValue(Value *Vector,
                                          Instruction *InsertBefore,
                                          size_t idx) {
  assert(hasSEV(Vector->getType()));
  Instruction *Val = nullptr;
  if (isa<UndefValue>(Vector))
    return UndefValue::get(getTypeFreeFromSEV(Vector->getType()));
  else if (isa<PointerType>(Vector->getType()))
    Val = new BitCastInst(Vector, getTypeFreeFromSEV(Vector->getType()),
                          "sev.cast.", InsertBefore);
  else if (auto *Const = dyn_cast<Constant>(Vector))
    return Const->getAggregateElement(idx);
  else {
    Val = ExtractElementInst::Create(Vector, getVectorIndex(idx), "sev.cast.",
                                     InsertBefore);
  }
  if (auto *InVector = dyn_cast<Instruction>(Vector))
    Val->setDebugLoc(InVector->getDebugLoc());
  return Val;
}

// This util accepts SEV-rich Value and returns new, SEV-free one
// For pointer types it returns BitCastInst
// For constant vector it returns element of Vector
// For non-constant vectors it returns ExtractElementInst
Value *SEVUtil::createVectorToScalarValue(Value *Vector, BasicBlock *BB,
                                          size_t idx) {
  assert(hasSEV(Vector->getType()));
  Instruction *Val = nullptr;
  if (isa<UndefValue>(Vector))
    return UndefValue::get(getTypeFreeFromSEV(Vector->getType()));
  else if (isa<PointerType>(Vector->getType()))
    Val = new BitCastInst(Vector, getTypeFreeFromSEV(Vector->getType()),
                          "sev.cast.", BB);
  else if (auto *Const = dyn_cast<Constant>(Vector))
    return Const->getAggregateElement(idx);
  else {
    Val = ExtractElementInst::Create(Vector, getVectorIndex(idx), "sev.cast.",
                                     BB);
  }
  if (auto *InVector = dyn_cast<Instruction>(Vector))
    Val->setDebugLoc(InVector->getDebugLoc());
  return Val;
}

// This util accepts Scalar Value and returns new SEV-rich Value
// For pointer types it returns BitCastInst
// For constant elements it returns constant vector
// For non-constant vectors it returns InsertElementInst
Value *SEVUtil::createScalarToVectorValue(Value *Scalar, Type *RefTy,
                                          Instruction *InsertBefore) {
  if (isa<UndefValue>(Scalar))
    return UndefValue::get(RefTy);
  else if (isa<PointerType>(Scalar->getType()) && isa<PointerType>(RefTy)) {
    auto Inner = getInnerPointerVectorNesting(RefTy);
    return new BitCastInst(Scalar, getTypeWithSEV(Scalar->getType(), Inner),
                           "sev.cast.", InsertBefore);
  } else if (auto *Const = dyn_cast<ConstantInt>(Scalar))
    return ConstantInt::getSigned(RefTy, getConstantElement(Const));
  else {
    return InsertElementInst::Create(UndefValue::get(RefTy), Scalar,
                                     getVectorIndex(0), "sev.cast.",
                                     InsertBefore);
  }
}

// Returns Old Value if it is already SEV-free
// Creates SEV-free value otherwise
Value *SEVUtil::getValueFreeFromSEV(Value *OldV, Instruction *InsertBefore) {
  if (!hasSEV(OldV->getType()))
    return OldV;
  return createVectorToScalarValue(OldV, InsertBefore);
}

// Returns Old Value if it is already SEV free
// Creates SEV-free value otherwise
Value *SEVUtil::getValueFreeFromSEV(Value *OldV, BasicBlock *BB) {
  if (!hasSEV(OldV->getType()))
    return OldV;
  return createVectorToScalarValue(OldV, BB);
}

// Returns Old Value if it is already SEV-rich
// Creates SEV-rich value otherwise
Value *SEVUtil::getValueWithSEV(Value *OldV, Type *RefTy,
                                Instruction *InsertBefore) {
  if (hasSEV(OldV->getType())) {
    assert(RefTy == OldV->getType());
    return OldV;
  }
  return createScalarToVectorValue(OldV, RefTy, InsertBefore);
}

// Returns SEV-free type of new instruction in the first parameter
// Returns SEV-free analogues of old instruction parameteres in the second
// parameter
std::pair<Type *, SEVUtil::ValueCont>
SEVUtil::getOperandsFreeFromSEV(Instruction &OldInst) {
  auto Values = ValueCont{};
  auto *NewRetTy = getTypeFreeFromSEV(OldInst.getType());
  for (auto I = size_t{0}; I < OldInst.getNumOperands(); ++I) {
    auto *Op = OldInst.getOperand(I);
    auto *NewOp = getValueFreeFromSEV(Op, &OldInst);
    Values.push_back(NewOp);
  }
  return {NewRetTy, std::move(Values)};
}

// This util accepts SEV value and inserts its only element to the new
// empty vector of size 2
// Returns this new vector as a result
// For undef vectors it returns new undefs directly without any insertions
//
// Because this function may cause regressions,
// it is used only in specific case of shufflevector instruction
Value *SEVUtil::getTwoElementVectorFromOneElement(Value *V,
                                                  Instruction *InsertBefore) {
  auto *VTy = cast<VectorType>(V->getType());
  auto *NewVTy =
      VCINTR::getVectorType(VTy->getElementType(), ShuffleVectorSize);
  if (isa<UndefValue>(V))
    return UndefValue::get(NewVTy);
  auto *Extract = createVectorToScalarValue(V, InsertBefore);
  auto *Insert = createScalarToVectorValue(Extract, NewVTy, InsertBefore);
  return Insert;
}

// This function finalizes replacement of old instruction with the new one
// After all arguments of OldInst were converted to SEV-rich/free form
// this util moves all properties of OldInst to NewInst and inserts
// a convertion instruction if type of OldInst is not the same as of NewInst
void SEVUtil::replaceAllUsesWith(Instruction *OldInst, Instruction *NewInst) {
  NewInst->takeName(OldInst);
  NewInst->copyMetadata(*OldInst);
  NewInst->copyIRFlags(OldInst);

  auto *ReplaceInst = cast<Value>(NewInst);
  if (!hasSEV(NewInst->getType()) && hasSEV(OldInst->getType()))
    ReplaceInst =
        createScalarToVectorValue(NewInst, OldInst->getType(), OldInst);
  else if (hasSEV(NewInst->getType()) && !hasSEV(OldInst->getType()))
    ReplaceInst = createVectorToScalarValue(NewInst, OldInst);
  OldInst->replaceAllUsesWith(ReplaceInst);
  OldInst->eraseFromParent();
}

/// This section contains utilities for rewriting function signatures:
///  * Generating SEV-rich/free version of given function signature
///  * Fixing return instruction so that theirs type match new signature
///  * Replacing old function arguments with new ones
///  * Replacing call instructions of old function
///  * For convertions from SEV-free to SEV-rich dertemines if specific argument
///    should be SEV, or it was a scalar originally

// This is a final step of replacing an old function
// After new function was generated, this util finds all uses of old function
// and replaces them with use of new one.
void SEVUtil::replaceAllUsesWith(Function &OldF, Function &NewF) {
  assert(OldF.getFunctionType() != NewF.getFunctionType());

  auto Users = SmallVector<User *, 8>{};
  std::transform(OldF.user_begin(), OldF.user_end(), std::back_inserter(Users),
                 [](User *U) { return U; });
  auto IsScalarToVector = doesSignatureHaveSEV(NewF);
  assert(IsScalarToVector == !doesSignatureHaveSEV(OldF));

  for (auto *U : Users) {
    auto *OldInst = cast<CallInst>(U);
    assert(OldInst);
    auto NewParams = SmallVector<Value *, 8>{};

    for (auto &&ArgPair : llvm::zip(OldF.args(), NewF.args())) {
      auto &&OldArg = std::get<0>(ArgPair);
      auto &&NewArg = std::get<1>(ArgPair);
      auto ArgNo = OldArg.getArgNo();
      auto *Op = OldInst->getOperand(ArgNo);
      auto *Conv = Op;
      if (!IsScalarToVector)
        Conv = getValueFreeFromSEV(Op, OldInst);
      else {
        if (OldArg.getType() != NewArg.getType())
          Conv = getValueWithSEV(Op, NewArg.getType(), OldInst);
      }
      NewParams.push_back(Conv);
    }

    auto *NewCall = CallInst::Create(&NewF, NewParams, "", OldInst);
    NewCall->setCallingConv(OldInst->getCallingConv());
    NewCall->setTailCallKind(OldInst->getTailCallKind());
    NewCall->copyIRFlags(OldInst);
    NewCall->copyMetadata(*OldInst);
    NewCall->setAttributes(OldInst->getAttributes());
    replaceAllUsesWith(OldInst, NewCall);
  }
}

// After new function was generated, it still uses arguments from old function
// inside its body. This util moves all properties of old argument to the new
// one and insert convert instructions if needed.
// After that it replaces old argument with the new one
void SEVUtil::replaceAllUsesWith(Argument &OldArg, Argument &NewArg,
                                 Function &NewF) {
  NewArg.takeName(&OldArg);
  auto *OldTy = OldArg.getType();
  auto *NewTy = NewArg.getType();
  if (OldTy == NewTy) {
    OldArg.replaceAllUsesWith(&NewArg);
    return;
  }

  Value *Conv = nullptr;
  auto &&InsPt = NewF.getEntryBlock().front();

  if (hasSEV(OldTy)) {
    assert(!hasSEV(NewTy));
    Conv = createScalarToVectorValue(&NewArg, OldTy, &InsPt);
  } else {
    assert(hasSEV(NewTy));
    assert(!hasSEV(OldTy));
    Conv = createVectorToScalarValue(&NewArg, &InsPt);
  }
  OldArg.replaceAllUsesWith(Conv);
}

// After new function was generated, its return instructions might not match the
// signature. This util inserts convert instructions for returns if needed
void SEVUtil::rewriteSEVReturns(Function &NewF) {
  auto &&Context = NewF.getContext();
  auto Instructions = getInstructions(NewF);
  auto *NewRetTy = NewF.getReturnType();
  bool IsVectorReturn = hasSEV(NewRetTy);

  for (auto *Inst : Instructions) {
    auto *RetInst = dyn_cast<ReturnInst>(Inst);
    if (!RetInst)
      continue;
    auto *RetV = RetInst->getReturnValue();
    Value *Conv = nullptr;
    if (IsVectorReturn) {
      assert(!hasSEV(RetV->getType()));
      Conv = createScalarToVectorValue(RetV, NewRetTy, RetInst);
    } else {
      assert(hasSEV(RetV->getType()));
      Conv = createVectorToScalarValue(RetV, RetInst);
    }
    auto *NewRet = ReturnInst::Create(Context, Conv, RetInst);
    NewRet->takeName(RetInst);
    RetInst->eraseFromParent();
  }
}

// For conversion in SEV-rich to SEV-free direction
// this function adds VCSingleElementVector attribute to argument or function
// if theirs types were modified.
// It is needed for telling reader part that they should be converted back
// For conversion in SEV-free to SEV-rich direction
// this function removes VCSingleElementVector attributes
void SEVUtil::manageSEVAttribute(Function &NewF, Type *OldTy, Type *NewTy,
                                 size_t AttrNo) {
  if (hasSEV(OldTy)) {
    assert(!hasSEV(NewTy));
    auto InnerPtrs = std::to_string(getInnerPointerVectorNesting(OldTy));
    auto Attr = Attribute::get(NewF.getContext(),
                               VCModuleMD::VCSingleElementVector, InnerPtrs);
    VCINTR::Function::addAttributeAtIndex(NewF, AttrNo, Attr);
  } else if (hasSEV(NewTy)) {
    assert(!hasSEV(OldTy));
    VCINTR::Function::removeAttributeAtIndex(NewF, AttrNo,
                                             VCModuleMD::VCSingleElementVector);
  }
}

void SEVUtil::manageSEVAttributes(Function &OldF, Function &NewF) {
  for (Function::arg_iterator ArgIt = NewF.arg_begin(), E = NewF.arg_end();
       ArgIt != E; ++ArgIt) {
    auto ArgNo = ArgIt->getArgNo();
    auto *OldTy = VCINTR::Function::getArg(OldF, ArgNo)->getType();
    auto *NewTy = ArgIt->getType();
    manageSEVAttribute(NewF, OldTy, NewTy, ArgNo + 1);
  }
  manageSEVAttribute(NewF, OldF.getReturnType(), NewF.getReturnType(),
                     AttributeList::ReturnIndex);
}

// For conversion in SEV-free to SEV-rich direction
// this function determines whether return value or argument of function
// should be converted to single element vector
// If true it returns type to convert to. Otherwise it returns currently
// presented type in Function.
Type *SEVUtil::getOriginalType(Function &F, size_t AttrNo) {
  using namespace llvm::GenXIntrinsic;
  auto *FuncTy = F.getFunctionType();
  auto *Ty =
      AttrNo == 0 ? FuncTy->getReturnType() : FuncTy->getParamType(AttrNo - 1);
  auto Attrs = F.getAttributes();
  if (!VCINTR::AttributeList::hasAttributeAtIndex(
          Attrs, AttrNo, VCModuleMD::VCSingleElementVector))
    return Ty;
  NeedCollapse = true;
  auto InnerPtrsStr = VCINTR::AttributeList::getAttributeAtIndex(
                          Attrs, AttrNo, VCModuleMD::VCSingleElementVector)
                          .getValueAsString();
  auto InnerPtrs = InnerPtrsStr.empty() ? 0 : std::stoull(InnerPtrsStr.str());
  return getTypeWithSEV(Ty, InnerPtrs);
}

// Returns function with SEV-rich or SEV-free signature depending on
// IsScalarToVector parameter
// If signature did not change it returns the same function
// This is the first step of rewriting a function
Function &SEVUtil::getSEVSignature(Function &F, bool IsScalarToVector) {
  auto NewParams = SmallVector<Type *, 8>{};
  for (Function::arg_iterator ArgIt = F.arg_begin(), E = F.arg_end();
       ArgIt != E; ++ArgIt) {
    auto ArgNo = ArgIt->getArgNo();
    Type *NewTy = nullptr;
    if (!IsScalarToVector)
      NewTy = getTypeFreeFromSEV(ArgIt->getType());
    else
      NewTy = getOriginalType(F, size_t(ArgNo) + 1);
    NewParams.push_back(NewTy);
  }
  Type *NewRetTy = nullptr;
  if (!IsScalarToVector)
    NewRetTy = getTypeFreeFromSEV(F.getReturnType());
  else
    NewRetTy = getOriginalType(F, AttributeList::ReturnIndex);

  auto *NewFuncTy = FunctionType::get(NewRetTy, NewParams, F.isVarArg());
  if (NewFuncTy == F.getFunctionType())
    return F;

  auto &&NewF =
      *Function::Create(NewFuncTy, F.getLinkage(), F.getAddressSpace());

  assert(doesSignatureHaveSEV(F) || doesSignatureHaveSEV(NewF));
  return NewF;
}

// Completely rewrites function in the entire module to its SEV-rich or SEV-free
// analogue depending on IsScalarToVector parameter
// This is a main util in this section
void SEVUtil::rewriteSEVSignature(Function &F, bool IsScalarToVector) {
  auto &&NewF = getSEVSignature(F, IsScalarToVector);
  if (&NewF == &F)
    return;

  NewF.copyAttributesFrom(&F);
  NewF.takeName(&F);
  NewF.copyMetadata(&F, 0);
#if VC_INTR_LLVM_VERSION_MAJOR >= 18
  NewF.updateAfterNameChange();
#else  // VC_INTR_LLVM_VERSION_MAJOR >= 18
  NewF.recalculateIntrinsicID();
#endif // VC_INTR_LLVM_VERSION_MAJOR >= 18
  F.getParent()->getFunctionList().insert(F.getIterator(), &NewF);
#if VC_INTR_LLVM_VERSION_MAJOR > 15
  NewF.splice(NewF.begin(), &F);
#else
  NewF.getBasicBlockList().splice(NewF.begin(), F.getBasicBlockList());
#endif
  manageSEVAttributes(F, NewF);

  if (NewF.size() > 0) {
    for (auto &&ArgPair : llvm::zip(F.args(), NewF.args()))
      replaceAllUsesWith(std::get<0>(ArgPair), std::get<1>(ArgPair), NewF);
    if (NewF.getReturnType() != F.getReturnType())
      rewriteSEVReturns(NewF);
  }
  replaceAllUsesWith(F, NewF);
  F.eraseFromParent();
}

/// This section contains class for rewriting different types of
/// instructions in SEV-rich to SEV-free direction.
/// Each instruction is rewritten to its SEV-free analogue and
/// guarded with convert instructions for its arguments and uses
/// Convert instructions (BitCastInst, InsertElementInst, ExtractElementInst)
/// are not covered in this section. Instead they are managed with collapsing
/// utils in the next section

void SEVUtil::visit(Function &F) {
  auto Instructions = getInstructions(F);
  for (auto *OldInst : Instructions) {
    if (!hasSEV(OldInst))
      continue;
    auto *NewInst = visit(*OldInst);
    if (NewInst)
      replaceAllUsesWith(OldInst, NewInst);
  }
}

Instruction *SEVUtil::visitStoreInst(StoreInst &OldInst) {
  Type *NewTy = nullptr;
  auto NewVals = ValueCont{};
  std::tie(NewTy, NewVals) = getOperandsFreeFromSEV(OldInst);
  return new llvm::StoreInst(NewVals[0], NewVals[1], OldInst.isVolatile(),
                             VCINTR::Align::getAlign(&OldInst),
                             OldInst.getOrdering(), OldInst.getSyncScopeID(),
                             &OldInst);
}

Instruction *SEVUtil::visitBinaryOperator(BinaryOperator &OldInst) {
  Type *NewTy = nullptr;
  auto NewVals = ValueCont{};
  std::tie(NewTy, NewVals) = getOperandsFreeFromSEV(OldInst);
  return BinaryOperator::Create(OldInst.getOpcode(), NewVals[0], NewVals[1], "",
                                &OldInst);
}

Instruction *SEVUtil::visitCmpInst(CmpInst &OldInst) {
  Type *NewTy = nullptr;
  auto NewVals = ValueCont{};
  std::tie(NewTy, NewVals) = getOperandsFreeFromSEV(OldInst);
  return CmpInst::Create(OldInst.getOpcode(), OldInst.getPredicate(),
                         NewVals[0], NewVals[1], "", &OldInst);
}

Instruction *SEVUtil::visitShuffleVectorInst(ShuffleVectorInst &OldInst) {
  auto Mask = SmallVector<int, 16>{}; // Ensures copy
  OldInst.getShuffleMask(Mask);
  auto *Op0 = OldInst.getOperand(0);
  auto *Op1 = OldInst.getOperand(1);
  auto *Op0Ty = cast<VectorType>(Op0->getType());
  auto *Op1Ty = cast<VectorType>(Op1->getType());

  auto &&Context = OldInst.getContext();
  auto *Int32Ty = IntegerType::getInt32Ty(Context);
  if (Mask.size() == 1) {
    Value *VectorOp = nullptr;
    Value *Idx = nullptr;
    auto IsUndef = Mask[0] == VCINTR::ShuffleVectorInst::UndefMaskElem;
    if (IsUndef)
      VectorOp = UndefValue::get(
          VCINTR::getVectorType(Op0Ty->getElementType(), ShuffleVectorSize));
    else {
      auto IsUsedFirstOperand = static_cast<unsigned>(Mask[0]) <
                                VCINTR::VectorType::getNumElements(Op0Ty);
      VectorOp = IsUsedFirstOperand ? Op0 : Op1;
    }
    if (IsUndef)
      Idx = UndefValue::get(Int32Ty);
    else
      Idx = ConstantInt::get(Int32Ty, Mask[0]);
    return ExtractElementInst::Create(VectorOp, Idx, "", &OldInst);
  }

  auto *NewOp0 = Op0;
  auto *NewOp1 = Op1;
  if (hasSEV(Op0Ty)) {
    NewOp0 = getTwoElementVectorFromOneElement(Op0, &OldInst);
    std::transform(Mask.begin(), Mask.end(), Mask.begin(), [](int El) {
      if (El > 0 && El != VCINTR::ShuffleVectorInst::UndefMaskElem)
        return El + 1;
      return El;
    });
  }
  if (hasSEV(Op1Ty))
    NewOp1 = getTwoElementVectorFromOneElement(Op1, &OldInst);

  return new ShuffleVectorInst(
      NewOp0, NewOp1, VCINTR::ShuffleVectorInst::getShuffleMask(Mask, Context),
      "", &OldInst);
}

Instruction *SEVUtil::visitSelectInst(SelectInst &OldInst) {
  Type *NewTy = nullptr;
  auto NewVals = ValueCont{};
  std::tie(NewTy, NewVals) = getOperandsFreeFromSEV(OldInst);
  return SelectInst::Create(NewVals[0], NewVals[1], NewVals[2], "", &OldInst,
                            &OldInst);
}

Instruction *SEVUtil::visitPHINode(PHINode &OldInst) {
  auto NewTy = getTypeFreeFromSEV(OldInst.getType());
  auto Phi =
      PHINode::Create(NewTy, OldInst.getNumIncomingValues(), "", &OldInst);
  for (auto I = size_t{0}; I < OldInst.getNumIncomingValues(); ++I) {
    auto *V = OldInst.getIncomingValue(I);
    auto *BB = OldInst.getIncomingBlock(I);
    auto *NewV = getValueFreeFromSEV(V, BB);
    Phi->addIncoming(NewV, BB);
  }
  return Phi;
}

Instruction *SEVUtil::visitAllocaInst(AllocaInst &OldInst) {
  auto *NewTy = getTypeFreeFromSEV(OldInst.getAllocatedType());
  return new llvm::AllocaInst(NewTy, OldInst.getType()->getAddressSpace(),
                              OldInst.getArraySize(),
                              VCINTR::Align::getAlign(&OldInst), "", &OldInst);
}

Instruction *SEVUtil::visitCastInst(CastInst &OldInst) {
  Type *NewTy = nullptr;
  auto NewVals = ValueCont{};
  std::tie(NewTy, NewVals) = getOperandsFreeFromSEV(OldInst);
  return CastInst::Create(OldInst.getOpcode(), NewVals[0], NewTy, "", &OldInst);
}

Instruction *SEVUtil::visitLoadInst(LoadInst &OldInst) {
  Type *NewTy = nullptr;
  auto NewVals = ValueCont{};
  std::tie(NewTy, NewVals) = getOperandsFreeFromSEV(OldInst);
  return new llvm::LoadInst(NewTy, NewVals[0], "", OldInst.isVolatile(),
                            VCINTR::Align::getAlign(&OldInst),
                            OldInst.getOrdering(), OldInst.getSyncScopeID(),
                            &OldInst);
}

Instruction *SEVUtil::visitUnaryOperator(UnaryOperator &OldInst) {
  Type *NewTy = nullptr;
  auto NewVals = ValueCont{};
  std::tie(NewTy, NewVals) = getOperandsFreeFromSEV(OldInst);
  return UnaryOperator::Create(OldInst.getOpcode(), NewVals[0], "", &OldInst);
}

Instruction *SEVUtil::visitVAArgInst(VAArgInst &OldInst) {
  Type *NewTy = nullptr;
  auto NewVals = ValueCont{};
  std::tie(NewTy, NewVals) = getOperandsFreeFromSEV(OldInst);
  return new VAArgInst(NewVals[0], NewTy, "", &OldInst);
}

Instruction *SEVUtil::visitExtractValueInst(ExtractValueInst &OldInst) {
  Type *NewTy = nullptr;
  auto NewVals = ValueCont{};
  std::tie(NewTy, NewVals) = getOperandsFreeFromSEV(OldInst);
  return ExtractValueInst::Create(NewVals[0], OldInst.getIndices(), "",
                                  &OldInst);
}

Instruction *SEVUtil::visitGetElementPtrInst(GetElementPtrInst &OldInst) {
  Type *NewTy = nullptr;
  auto NewVals = ValueCont{};
  std::tie(NewTy, NewVals) = getOperandsFreeFromSEV(OldInst);
  std::vector<Value *> IdxList;
  std::transform(NewVals.begin() + 1, NewVals.end(),
                 std::back_inserter(IdxList), [](Value *V) { return V; });
  auto *PointeeTy = getTypeFreeFromSEV(OldInst.getSourceElementType());
  return GetElementPtrInst::Create(PointeeTy, NewVals[0], IdxList, "",
                                   &OldInst);
}

Instruction *SEVUtil::visitExtractElementInst(ExtractElementInst &OldInst) {
  // No processing required
  // Extracts and Inserts will be collapsed later
  return nullptr;
}

Instruction *SEVUtil::visitInsertElementInst(InsertElementInst &OldInst) {
  // No processing required
  // Extracts and Inserts will be collapsed later
  return nullptr;
}

Instruction *SEVUtil::visitInstruction(Instruction &I) {
  // For CallInst this is a bug, because
  // Calls have been processed in rewriteSEVUses function
  // For ReturnInst this is a bug, because
  // Returns have been processed in rewriteSEVReturn function
  // For other cases this assert is due to "visit" method is not implemented
  assert(false && "Oops... Cannot rewrite instruction!");
  return nullptr;
}

/// This section contains utils for rewriting global variables

// For conversion in SEV-rich to SEV-free direction
// this function adds VCSingleElementVector attribute to global var
void SEVUtil::manageSEVAttribute(GlobalVariable &GV, Type *OldTy, Type *NewTy) {
  if (hasSEV(OldTy)) {
    assert(!hasSEV(NewTy));
    auto InnerPtrs = std::to_string(getInnerPointerVectorNesting(OldTy));
    GV.addAttribute(VCModuleMD::VCSingleElementVector, InnerPtrs);
  }
}

GlobalVariable &SEVUtil::createAndTakeFrom(GlobalVariable &GV,
                                           Type *NewTy,
                                           Constant *Initializer) {
  auto *NewGV = new GlobalVariable(
      *GV.getParent(), NewTy, GV.isConstant(), GV.getLinkage(),
      Initializer, "sev.global.", &GV, GV.getThreadLocalMode(),
      GV.getAddressSpace(), GV.isExternallyInitialized());
  auto DebugInfoVec = SmallVector<DIGlobalVariableExpression *, 2>{};
  GV.getDebugInfo(DebugInfoVec);
  NewGV->takeName(&GV);
  NewGV->setAttributes(GV.getAttributes());
  NewGV->copyMetadata(&GV, 0);
  NewGV->setComdat(GV.getComdat());
  NewGV->setAlignment(VCINTR::Align::getAlign(&GV));
  for (auto *DebugInf : DebugInfoVec)
    NewGV->addDebugInfo(DebugInf);
  return *NewGV;
}

void SEVUtil::rewriteGlobalVariable(GlobalVariable &GV) {
  auto *Ty = GV.getValueType();
  auto *NewTy = getTypeFreeFromSEV(Ty);
  if (NewTy == Ty)
    return;
  Constant *Initializer = nullptr;
  if (GV.hasInitializer())
    Initializer = cast<Constant>(createVectorToScalarValue(
        GV.getInitializer(), static_cast<Instruction *>(nullptr)));
  auto &&NewGV = createAndTakeFrom(GV, NewTy, Initializer);
  if (VCINTR::Type::isOpaquePointerTy(GV.getType())) {
    GV.replaceAllUsesWith(&NewGV);
  } else {
    while (GV.use_begin() != GV.use_end()) {
      auto &&Use = GV.use_begin();
      auto *Inst = cast<Instruction>(Use->getUser());
      auto *V = createScalarToVectorValue(&NewGV, GV.getType(), Inst);
      *Use = V;
    }
  }
  manageSEVAttribute(NewGV, Ty, NewTy);
  GV.eraseFromParent();
}

void SEVUtil::restoreGlobalVariable(GlobalVariable &GV) {
  auto *Ty = GV.getValueType();
  if (!GV.hasAttribute(VCModuleMD::VCSingleElementVector))
    return;
  NeedCollapse = true;
  auto InnerPtrsStr =
      GV.getAttribute(VCModuleMD::VCSingleElementVector).getValueAsString();
  auto InnerPtrs = InnerPtrsStr.empty() ? 0 : std::stoull(InnerPtrsStr.str());
  auto *NewTy = getTypeWithSEV(Ty, InnerPtrs);
  if (NewTy == Ty)
    return;
  Constant *Initializer = nullptr;
  if (GV.hasInitializer())
    Initializer = cast<Constant>(createScalarToVectorValue(
        GV.getInitializer(), NewTy,
        static_cast<Instruction *>(nullptr)));
  auto &&NewGV = createAndTakeFrom(GV, NewTy, Initializer);
  if (VCINTR::Type::isOpaquePointerTy(GV.getType())) {
    GV.replaceAllUsesWith(&NewGV);
  } else {
    while (GV.use_begin() != GV.use_end()) {
      auto &&Use = GV.use_begin();
      auto *Inst = cast<Instruction>(Use->getUser());
      auto *V = createVectorToScalarValue(&NewGV, Inst);
      *Use = V;
    }
  }
  manageSEVAttribute(NewGV, Ty, NewTy);
  GV.eraseFromParent();
}

void SEVUtil::rewriteGlobalVariables(bool IsScalarToVector) {
  auto &&Globals = getGlobalVariables();
  for (auto *GV : Globals) {
    if (IsScalarToVector)
      restoreGlobalVariable(*GV);
    else
      rewriteGlobalVariable(*GV);
  }
}

/// This section contains utils for collapsing pairs of convertion instructions
/// After rewriting all insructions in the module there are lots of pairs
/// Extract-insert and bitcast-bitcast conversions left
/// These utilities eliminate such pairs

void SEVUtil::collapseBitcastInst(BitCastInst *BitCast,
                                  bool CollapseCannotFail) {
  if (BitCast->user_empty()) {
    BitCast->eraseFromParent();
    return;
  }
  auto &&Q = SimplifyQuery(M.getDataLayout());
  auto *ReplaceWith = VCINTR::SimplifyCastInst(
      BitCast->getOpcode(), BitCast->getOperand(0), BitCast->getType(), Q);
  if (!CollapseCannotFail && !ReplaceWith)
    return;
  assert(ReplaceWith && "Oops... Cannot collapse BitCast instruction!");
  BitCast->replaceAllUsesWith(ReplaceWith);
  BitCast->eraseFromParent();
}

// After rewriting instructions from SEV-rich/free form to SEV-free/rich one
// There are lots of auxiliary pairs of bitcasts left, like these:
// %b = bitcast <Ty> %a to <U>
// %d = bitcast <U> %b to <Ty>
// %e = some_user_of_T <Ty> %d
//
// This util collapses such pairs of bitcasts in two iterations:
// First iteration will remove %d
// Second iteration will remove %b
void SEVUtil::collapseBitcastInstructions(Function &F,
                                          bool CollapseCannotFail) {
  for (size_t i = 0; i < 2; i++) {
    auto Instructions = getInstructions(F);
    for (auto *I : Instructions) {
      if (auto *BitCast = dyn_cast<BitCastInst>(I)) {
        auto HasSEV = hasSEV(BitCast->getOperand(0)->getType()) ||
                      hasSEV(BitCast->getType());
        collapseBitcastInst(BitCast, i && CollapseCannotFail && HasSEV);
      }
    }
  }
}

void SEVUtil::collapseExtractInst(ExtractElementInst *Extract,
                                  bool CollapseCannotFail) {
  if (Extract->user_empty()) {
    Extract->eraseFromParent();
    return;
  }
  auto &&Q = SimplifyQuery(M.getDataLayout());
  auto *ReplaceWith = VCINTR::SimplifyExtractElementInst(
      Extract->getOperand(0), Extract->getOperand(1), Q);
  if (!CollapseCannotFail && !ReplaceWith)
    return;
  assert(ReplaceWith && "Oops... Cannot collapse ExtractElement instruction");
  Extract->replaceAllUsesWith(ReplaceWith);
  Extract->eraseFromParent();
}

void SEVUtil::collapseInsertInst(InsertElementInst *Insert,
                                 bool CollapseCannotFail) {
  if (Insert->user_empty()) {
    Insert->eraseFromParent();
    return;
  }
  auto &&Q = SimplifyQuery(M.getDataLayout());
  auto *ReplaceWith = VCINTR::SimplifyInsertElementInst(
      Insert->getOperand(0), Insert->getOperand(1), Insert->getOperand(2), Q);

  // SimplifyInsertElementInst provides too simple analysis
  // which does not work in some cases handled below:
  if (!ReplaceWith && hasSEV(Insert->getType())) {
    auto *Scal = Insert->getOperand(1);
    auto *VecTy = cast<VectorType>(Insert->getType());
    if (auto *Extract = dyn_cast<ExtractElementInst>(Scal)) {
      if (hasSEV(Extract->getOperand(0)->getType()))
        ReplaceWith = Extract->getOperand(0);
    } else if (isa<UndefValue>(Scal))
      ReplaceWith = UndefValue::get(VecTy);
    else if (auto *Const = dyn_cast<ConstantInt>(Scal))
      ReplaceWith = ConstantInt::get(VecTy, getConstantElement(Const));
  }

  if (!CollapseCannotFail && !ReplaceWith)
    return;
  assert(ReplaceWith && "Oops... Cannot collapse InsertElement instruction");
  Insert->replaceAllUsesWith(ReplaceWith);
  Insert->eraseFromParent();
}

// After rewriting instructions from SEV-free form to SEV-rich one
// There are lots of auxiliary pairs of insert-extract
// instructions left, like these:
// 1.
// %v = insertelement <1 x Ty> %u, <Ty> %element, <i32> %zero_idx
// %s = extractlement <1 x Ty> %v, <i32> %zero_idx
// %e = some_user_of_T <Ty> %s
// 2.
// %s = extractlement <n x Ty> %ConstantVector, <i32> %ConstantIdx
// %e = some_user_of_T <Ty> %s
// This utility removes excessive ExtractElement instructions
//
// After rewriting instructions from SEV-rich form to SEV-free one
// collapseInsertInstructions utility leaves lots of ExtractElement instructions
// with no users. This utility removes them as well
void SEVUtil::collapseExtractInstructions(Function &F,
                                          bool CollapseCannotFail) {
  auto Instructions = getInstructions(F);
  for (auto *I : Instructions) {
    if (auto *Extract = dyn_cast<ExtractElementInst>(I)) {
      auto OpTy = I->getOperand(0)->getType();
      collapseExtractInst(Extract, CollapseCannotFail && hasSEV(OpTy));
    }
  }
}

// After rewriting instructions from SEV-rich form to SEV-free one
// There are lots of auxiliary pairs of extract-insert
// instructions left, like these:
// 1.
// %s = extractlement <1 x Ty> %v, <i32> %idx
// %g = insertelement <1 x Ty> %u, <Ty> %s, <i32> %idx
// %e = some_user_of_1T <1 x Ty> %g
// 2.
// %g = insertelement <1 x Ty> %v, <Ty> %ConstantElement, <i32> %idx
// %e = some_user_of_T <Ty> %g
// This utility removes excessive InsertElement instructions
//
// After rewriting instructions from SEV-free form to SEV-rich one
// collapseExtractInstructions utility leaves lots of InsertElement instructions
// with no users. This utility removes them as well
void SEVUtil::collapseInsertInstructions(Function &F, bool CollapseCannotFail) {
  auto Instructions = getInstructions(F);
  for (auto *I : Instructions)
    if (auto *Insert = dyn_cast<InsertElementInst>(I))
      collapseInsertInst(Insert, CollapseCannotFail && hasSEV(I->getType()));
}

/// This section contains upper-level functions
/// for calling in GenXSPIRV adaptors
/// They either remove or restore Single Element Vectors in the module

void SEVUtil::rewriteSEVs() {
  rewriteGlobalVariables(/*IsScalarToVector=*/false);

  auto Functions = getFunctions();
  for (auto *F : Functions)
    rewriteSEVSignature(*F, /*IsScalarToVector=*/false);
  // Functions container should be refreshed after signatures rewriting
  Functions = getFunctions();

  visit(M);

  for (auto *F : Functions) {
    collapseExtractInstructions(*F);
    collapseInsertInstructions(*F);
  }

  for (auto *F : Functions)
    collapseBitcastInstructions(*F);
}

void SEVUtil::restoreSEVs() {
  rewriteGlobalVariables(/*IsScalarToVector=*/true);

  auto Functions = getFunctions();
  for (auto *F : Functions)
    rewriteSEVSignature(*F, /*IsScalarToVector=*/true);
  // Functions container should be refreshed after signatures rewriting
  Functions = getFunctions();

  if (!NeedCollapse)
    return;

  for (auto *F : Functions) {
    // When converting in SEV-free to SEV-rich direction
    // Collapsing of instructions may fail, because only call instructions were
    // rewritten. All other instructions were left intact.
    collapseInsertInstructions(*F, /*CollapseCannotFail=*/false);
    collapseExtractInstructions(*F, /*CollapseCannotFail=*/false);
  }

  for (auto *F : Functions)
    collapseBitcastInstructions(*F, /*CollapseCannotFail=*/false);
}

} // namespace genx
} // namespace llvm

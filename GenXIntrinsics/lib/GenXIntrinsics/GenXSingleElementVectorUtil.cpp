/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This file defines common functions for rewriting single element vectors
// in GenXSPIRV adaptors.

#include "GenXSingleElementVectorUtil.h"
#include "llvm/GenXIntrinsics/GenXIntrinsics.h"
#include "llvm/GenXIntrinsics/GenXMetadata.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#include "llvmVCWrapper/Analysis/InstructionSimplify.h"
#include "llvmVCWrapper/IR/Attributes.h"
#include "llvmVCWrapper/IR/DerivedTypes.h"
#include "llvmVCWrapper/IR/Function.h"
#include "llvmVCWrapper/IR/Instructions.h"
#include "llvmVCWrapper/IR/Type.h"
#include "llvmVCWrapper/Support/Alignment.h"

#include <unordered_map>

namespace llvm {
namespace genx {
/// This section contains some arbitrary constants

// Default size for arguments of SEV-free version ShuffleVector instruction
auto static constexpr ShuffleVectorSize = static_cast<unsigned>(2);

/// This section contains general utils:
///  * For safe iteration over functions and instructions
///  * For vectors operations such as converting constant vector element from
///    llvm::Value to int
///  * For examining pointer types
/// These utils are used across this module but they do not contain
/// any design solutions for removing Single Element Vectors (SEVs)

// Functions with SEVs are deleted from module
// This util allows to continue iteration even after deletion
static std::vector<Function *> getFunctions(Module &M) {
  auto Functions = std::vector<Function *>{};
  std::transform(M.begin(), M.end(), std::back_inserter(Functions),
                 [](Function &F) { return &F; });
  return Functions;
}

// Globals with SEVs are deleted from module
// This util allows to continue iteration even after deletion
static std::vector<GlobalVariable *> getGlobalVariables(Module &M) {
  auto Globals = std::vector<GlobalVariable *>{};
  std::transform(M.global_begin(), M.global_end(), std::back_inserter(Globals),
                 [](GlobalVariable &GV) { return &GV; });
  return Globals;
}

// Instructions with SEVs are deleted from module
// This util allows to continue iteration even after deletion
static std::vector<Instruction *> getInstructions(Function &F) {
  auto Instructions = std::vector<Instruction *>{};
  for (auto &&BB : F) {
    std::transform(BB.begin(), BB.end(), std::back_inserter(Instructions),
                   [](Instruction &I) { return &I; });
  }
  return Instructions;
}

// Returns requested vector index as Value*
// It is helpful for creating ExtractElementInst and InsertElementInst
static ConstantInt *getVectorIndex(Module &M, size_t idx) {
  auto *ITy = IntegerType::getIntNTy(M.getContext(),
                                     M.getDataLayout().getPointerSizeInBits(0));
  return ConstantInt::get(ITy, idx, false);
}

// Returns underlying int from Value*
static int64_t getConstantElement(ConstantInt *Const) {
  assert(!isa<UndefValue>(Const));
  return Const->getSExtValue();
}

// For type U***** returns number of stars and type U in the second argument
static size_t getPointerNesting(Type *T, Type **ReturnNested = nullptr) {
  auto NPtrs = size_t{0};
  auto *NestedType = T;
  while (dyn_cast<PointerType>(NestedType)) {
    NestedType = VCINTR::Type::getNonOpaquePtrEltTy(NestedType);
    ++NPtrs;
  }
  if (ReturnNested)
    *ReturnNested = NestedType;
  return NPtrs;
}

// For type <n x U****>**** returns total number of stars and type U in the
// second argument
static size_t getPointerVectorNesting(Type *T, Type **ReturnNested = nullptr) {
  auto *NestedT = static_cast<Type *>(nullptr);
  auto Outer = getPointerNesting(T, &NestedT);
  auto VT = dyn_cast<VectorType>(NestedT);
  if (!VT) {
    if (ReturnNested)
      *ReturnNested = NestedT;
    return Outer;
  }
  auto Inner = getPointerNesting(VT->getElementType(), &NestedT);
  if (ReturnNested)
    *ReturnNested = NestedT;
  return Outer + Inner;
}

// For type <n x U****>**** returns number of stars inside vector
static size_t getInnerPointerVectorNesting(Type *T) {
  auto Total = getPointerVectorNesting(T);
  auto Outer = getPointerNesting(T);
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

static std::unordered_map<StructType*, StructType*> SEVFreeStructMap;
static std::unordered_map<StructType*, StructType*> SEVRichStructMap;

// Returns SEV-free analogue of Type T accordingly to the following scheme:
// <1 x U>**...* ---> U**...*
static Type *getTypeFreeFromSingleElementVector(Type *T) {
  // Pointer types should be "undressed" first
  if (auto *Ptr = dyn_cast<PointerType>(T)) {
    auto UT = getTypeFreeFromSingleElementVector(VCINTR::Type::getNonOpaquePtrEltTy(Ptr));
    if (UT == VCINTR::Type::getNonOpaquePtrEltTy(Ptr))
      return Ptr;
    return PointerType::get(UT, Ptr->getAddressSpace());
  } else if (auto *VecTy = dyn_cast<VectorType>(T)) {
    if (VCINTR::VectorType::getNumElements(VecTy) == 1)
      return VecTy->getElementType();
  } else if (auto *StructTy = dyn_cast<StructType>(T)) {
    // If there is a key for this struct type is in SEV-Free to SEV-Rich map it
    // means that the type is already SEV-Free
    if (SEVRichStructMap.find(StructTy) != SEVRichStructMap.end())
      return T;
    auto It = SEVFreeStructMap.find(StructTy);
    if (It != SEVFreeStructMap.end())
      return It->second;
    // To handle circle dependencies we create opaque struct type and add it to
    // the map. If this struct or any nested one contains a pointer to the type
    // we are rewriting it will be automatically changed to this incomplete type
    // and traversing will stop
    StructType *NewStructTy = StructType::create(T->getContext());
    It = SEVFreeStructMap.insert(std::make_pair(StructTy, NewStructTy)).first;
    bool HasSEV = false;
    std::vector<Type *> NewElements;
    for (auto *ElemTy : StructTy->elements()) {
      Type *NewElemTy = getTypeFreeFromSingleElementVector(ElemTy);
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
  }
  return T;
}

// Returns SEV-rich analogue of Type T accordingly to the following scheme:
// U*...**...* ---> <1 x U*...*>*...*
static Type *getTypeWithSingleElementVector(Type *T, size_t InnerPointers = 0) {
  if (auto *VecTy = dyn_cast<VectorType>(T)) {
    (void) VecTy;
    assert(InnerPointers == 0);
    assert(VCINTR::VectorType::getNumElements(VecTy) == 1 &&
           "Cannot put vector type inside another vector!");
    return T;
  } else if (auto *StructTy = dyn_cast<StructType>(T)) {
    auto It = SEVRichStructMap.find(StructTy);
    if (It == SEVRichStructMap.end())
      llvm_unreachable("Unexpected SEV StructType");
    return It->second;
  }
  auto NPtrs = getPointerNesting(T);

  assert(InnerPointers <= NPtrs);
  if (InnerPointers == NPtrs)
    return VCINTR::getVectorType(T, 1);

  auto *Ptr = cast<PointerType>(T);
  auto *UT = getTypeWithSingleElementVector(VCINTR::Type::getNonOpaquePtrEltTy(Ptr),
                                            InnerPointers);
  return PointerType::get(UT, Ptr->getAddressSpace());
}

// Returns true if T is SEV or it is a pointer to SEV
static bool hasSingleElementVector(Type *T) {
  return T != getTypeFreeFromSingleElementVector(T);
}

// Returns true if Instruction type or type of any of its arguments has SEV
static bool hasSingleElementVector(Instruction *I) {
  if (hasSingleElementVector(I->getType()))
    return true;
  return std::find_if(I->op_begin(), I->op_end(), [](Use &Op) {
           return hasSingleElementVector(Op.get()->getType());
         }) != I->op_end();
}

// Returns true if return value or any of arguments have SEV
static bool doesSignatureHaveSingleElementVector(Function &F) {
  if (hasSingleElementVector(F.getReturnType()))
    return true;
  return std::find_if(F.arg_begin(), F.arg_end(), [](Argument &Arg) {
           return hasSingleElementVector(Arg.getType());
         }) != F.arg_end();
}

// This util accepts SEV-rich Value and returns new, SEV-free one
// For pointer types it returns BitCastInst
// For constant vector it returns element of Vector
// For non-constant vectors it ExtractElementInst
static Value *createVectorToScalarValue(Value *Vector,
                                        Instruction *InsertBefore,
                                        size_t idx = 0) {
  assert(hasSingleElementVector(Vector->getType()));
  Instruction *Val = nullptr;
  if (isa<UndefValue>(Vector))
    return UndefValue::get(
        getTypeFreeFromSingleElementVector(Vector->getType()));
  else if (isa<PointerType>(Vector->getType()))
    Val = new BitCastInst(Vector,
                          getTypeFreeFromSingleElementVector(Vector->getType()),
                          "sev.cast.", InsertBefore);
  else if (auto *Const = dyn_cast<Constant>(Vector))
    return Const->getAggregateElement(idx);
  else {
    auto *M = InsertBefore->getModule();
    Val = ExtractElementInst::Create(Vector, getVectorIndex(*M, idx),
                                     "sev.cast.", InsertBefore);
  }
  if (auto *InVector = dyn_cast<Instruction>(Vector))
    Val->setDebugLoc(InVector->getDebugLoc());
  return Val;
}

// This util accepts SEV-rich Value and returns new, SEV-free one
// For pointer types it returns BitCastInst
// For constant vector it returns element of Vector
// For non-constant vectors it returns ExtractElementInst
static Value *createVectorToScalarValue(Value *Vector, BasicBlock *BB,
                                        size_t idx = 0) {
  assert(hasSingleElementVector(Vector->getType()));
  Instruction *Val = nullptr;
  if (isa<UndefValue>(Vector))
    return UndefValue::get(
        getTypeFreeFromSingleElementVector(Vector->getType()));
  else if (isa<PointerType>(Vector->getType()))
    Val = new BitCastInst(Vector,
                          getTypeFreeFromSingleElementVector(Vector->getType()),
                          "sev.cast.", BB);
  else if (auto *Const = dyn_cast<Constant>(Vector))
    return Const->getAggregateElement(idx);
  else {
    auto *M = BB->getModule();
    Val = ExtractElementInst::Create(Vector, getVectorIndex(*M, idx),
                                     "sev.cast.", BB);
  }
  if (auto *InVector = dyn_cast<Instruction>(Vector))
    Val->setDebugLoc(InVector->getDebugLoc());
  return Val;
}

// This util accepts Scalar Value and returns new SEV-rich Value
// For pointer types it returns BitCastInst
// For constant elements it returns constant vector
// For non-constant vectors it returns InsertElementInst
static Value *createScalarToVectorValue(Value *Scalar, Type *ReferenceType,
                                        Instruction *InsertBefore) {
  if (isa<UndefValue>(Scalar))
    return UndefValue::get(ReferenceType);
  else if (isa<PointerType>(Scalar->getType()) &&
           isa<PointerType>(ReferenceType)) {
    auto Inner = getInnerPointerVectorNesting(ReferenceType);
    return new BitCastInst(
        Scalar, getTypeWithSingleElementVector(Scalar->getType(), Inner),
        "sev.cast.", InsertBefore);
  } else if (auto *Const = dyn_cast<ConstantInt>(Scalar))
    return ConstantInt::getSigned(ReferenceType, getConstantElement(Const));
  else {
    auto *M = InsertBefore->getModule();
    return InsertElementInst::Create(UndefValue::get(ReferenceType), Scalar,
                                     getVectorIndex(*M, 0), "sev.cast.",
                                     InsertBefore);
  }
}

// Returns Old Value if it is already SEV-free
// Creates SEV-free value otherwise
static Value *getValueFreeFromSingleElementVector(Value *OldV,
                                                  Instruction *InsertBefore) {
  if (!hasSingleElementVector(OldV->getType()))
    return OldV;
  return createVectorToScalarValue(OldV, InsertBefore);
}

// Returns Old Value if it is already SEV free
// Creates SEV-free value otherwise
static Value *getValueFreeFromSingleElementVector(Value *OldV, BasicBlock *BB) {
  if (!hasSingleElementVector(OldV->getType()))
    return OldV;
  return createVectorToScalarValue(OldV, BB);
}

// Returns Old Value if it is already SEV-rich
// Creates SEV-rich value otherwise
static Value *getValueWithSingleElementVector(Value *OldV, Type *ReferenceType,
                                              Instruction *InsertBefore) {
  if (hasSingleElementVector(OldV->getType())) {
    assert(ReferenceType == OldV->getType());
    return OldV;
  }
  return createScalarToVectorValue(OldV, ReferenceType, InsertBefore);
}

// Returns SEV-free type of new instruction in the first parameter
// Returns SEV-free analogues of old instruction parameteres in the second
//   parameter
using ValueCont = SmallVector<Value *, 4>;
static std::pair<llvm::Type *, ValueCont>
getOperandsFreeFromSingleElementVector(Instruction &OldInst) {
  auto Values = ValueCont{};
  auto *NewRetT = getTypeFreeFromSingleElementVector(OldInst.getType());
  for (auto I = size_t{0}; I < OldInst.getNumOperands(); ++I) {
    auto *Op = OldInst.getOperand(I);
    auto *NewOp = getValueFreeFromSingleElementVector(Op, &OldInst);
    Values.push_back(NewOp);
  }
  return {NewRetT, std::move(Values)};
}

// This util accepts SEV value and inserts its only element to the new
// empty vector of size 2
// Returns this new vector as a result
// For undef vectors it returns new undefs directly without any insertions
//
// Because this function may cause regressions,
// it is used only in specific case of shufflevector instruction
static Value *getTwoElementVectorFromOneElement(Value *V,
                                                Instruction *InsertBefore) {
  auto *VT = cast<VectorType>(V->getType());
  auto *NewVT = VCINTR::getVectorType(VT->getElementType(), ShuffleVectorSize);
  if (isa<UndefValue>(V))
    return UndefValue::get(NewVT);
  auto *Extract = createVectorToScalarValue(V, InsertBefore);
  auto *Insert = createScalarToVectorValue(Extract, NewVT, InsertBefore);
  return Insert;
}

// This function finalizes replacement of old instruction with the new one
// After all arguments of OldInst were converted to SEV-rich/free form
// this util moves all properties of OldInst to NewInst and inserts
// a convertion instruction if type of OldInst is not the same as of NewInst
static void replaceAllUsesWith(Instruction *OldInst, Instruction *NewInst) {
  NewInst->takeName(OldInst);
  NewInst->copyMetadata(*OldInst);
  NewInst->copyIRFlags(OldInst);

  auto *ReplaceInst = static_cast<Value *>(NewInst);
  if (!hasSingleElementVector(NewInst->getType()) &&
      hasSingleElementVector(OldInst->getType()))
    ReplaceInst =
        createScalarToVectorValue(NewInst, OldInst->getType(), OldInst);
  else if (hasSingleElementVector(NewInst->getType()) &&
           !hasSingleElementVector(OldInst->getType()))
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
static void replaceAllUsesWith(Function &OldF, Function &NewF) {
  assert(OldF.getType() != NewF.getType());

  auto Users = SmallVector<User *, 8>{};
  std::transform(OldF.user_begin(), OldF.user_end(), std::back_inserter(Users),
                 [](User *U) { return U; });
  auto IsScalarToVector = doesSignatureHaveSingleElementVector(NewF);
  assert(IsScalarToVector == !doesSignatureHaveSingleElementVector(OldF));

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
        Conv = getValueFreeFromSingleElementVector(Op, OldInst);
      else {
        if (OldArg.getType() != NewArg.getType())
          Conv = getValueWithSingleElementVector(Op, NewArg.getType(), OldInst);
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
static void replaceAllUsesWith(Argument &OldArg, Argument &NewArg,
                               Function &NewF) {
  NewArg.takeName(&OldArg);
  auto *OldTy = OldArg.getType();
  auto *NewTy = NewArg.getType();
  if (OldTy == NewTy) {
    OldArg.replaceAllUsesWith(&NewArg);
    return;
  }

  auto *Conv = static_cast<Value *>(nullptr);
  auto &&InsPt = NewF.getEntryBlock().front();

  if (hasSingleElementVector(OldTy)) {
    assert(!hasSingleElementVector(NewTy));
    Conv = createScalarToVectorValue(&NewArg, OldTy, &InsPt);
  } else {
    assert(hasSingleElementVector(NewTy));
    assert(!hasSingleElementVector(OldTy));
    Conv = createVectorToScalarValue(&NewArg, &InsPt);
  }
  OldArg.replaceAllUsesWith(Conv);
}

// After new function was generated, its return instructions might not match the
// signature. This util inserts convert instructions for returns if needed
static void rewriteSingleElementVectorReturns(Function &NewF) {
  auto &&Context = NewF.getContext();
  auto Instructions = getInstructions(NewF);
  auto *NewRetType = NewF.getReturnType();
  bool IsVectorReturn = hasSingleElementVector(NewRetType);

  for (auto *Inst : Instructions) {
    auto *RetInst = dyn_cast<ReturnInst>(Inst);
    if (!RetInst)
      continue;
    auto *RetV = RetInst->getReturnValue();
    auto *Conv = static_cast<Value *>(nullptr);
    if (IsVectorReturn) {
      assert(!hasSingleElementVector(RetV->getType()));
      Conv = createScalarToVectorValue(RetV, NewRetType, RetInst);
    } else {
      assert(hasSingleElementVector(RetV->getType()));
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
static void manageSingleElementVectorAttribute(Function &NewF, Type *OldT,
                                               Type *NewT, size_t AttrNo) {
  if (hasSingleElementVector(OldT)) {
    assert(!hasSingleElementVector(NewT));
    auto InnerPtrs = std::to_string(getInnerPointerVectorNesting(OldT));
    auto Attr = Attribute::get(NewF.getContext(),
                               VCModuleMD::VCSingleElementVector, InnerPtrs);
    VCINTR::Function::addAttributeAtIndex(NewF, AttrNo, Attr);
  } else if (hasSingleElementVector(NewT)) {
    assert(!hasSingleElementVector(OldT));
    VCINTR::Function::removeAttributeAtIndex(NewF, AttrNo,
                                             VCModuleMD::VCSingleElementVector);
  }
}

static void manageSingleElementVectorAttributes(Function &OldF,
                                                Function &NewF) {
  for (Function::arg_iterator ArgIt = NewF.arg_begin(), E = NewF.arg_end();
       ArgIt != E; ++ArgIt) {
    auto ArgNo = static_cast<size_t>(ArgIt->getArgNo());
    auto *OldT = (OldF.arg_begin() + ArgNo)->getType();
    auto *NewT = ArgIt->getType();
    manageSingleElementVectorAttribute(NewF, OldT, NewT, ArgNo + 1);
  }
  manageSingleElementVectorAttribute(NewF, OldF.getReturnType(),
                                     NewF.getReturnType(),
                                     AttributeList::ReturnIndex);
}

// For conversion in SEV-free to SEV-rich direction
// this function determines whether return value or argument of function
// should be converted to single element vector
// If true it returns type to convert to. Otherwise it returns currently
// presented type in Function.
static Type *getOriginalType(Function &F, size_t AttrNo) {
  using namespace llvm::GenXIntrinsic;
  auto *FuncT = F.getFunctionType();
  auto *T =
      AttrNo == 0 ? FuncT->getReturnType() : FuncT->getParamType(AttrNo - 1);
  auto Attrs = F.getAttributes();
  if (!VCINTR::AttributeList::hasAttributeAtIndex(
          Attrs, AttrNo, VCModuleMD::VCSingleElementVector))
    return T;
  auto InnerPtrsStr = VCINTR::AttributeList::getAttributeAtIndex(
                          Attrs, AttrNo, VCModuleMD::VCSingleElementVector)
                          .getValueAsString();
  auto InnerPtrs = InnerPtrsStr.empty() ? 0 : std::stoull(InnerPtrsStr.str());
  return getTypeWithSingleElementVector(T, InnerPtrs);
}

// Returns function with SEV-rich or SEV-free signature depending on
// IsScalarToVector parameter
// If signature did not change it returns the same function
// This is the first step of rewriting a function
static Function &getSingleElementVectorSignature(Function &F,
                                                 bool IsScalarToVector) {
  auto NewParams = SmallVector<Type *, 8>{};
  for (Function::arg_iterator ArgIt = F.arg_begin(), E = F.arg_end();
       ArgIt != E; ++ArgIt) {
    auto ArgNo = ArgIt->getArgNo();
    auto *NewT = static_cast<Type *>(nullptr);
    if (!IsScalarToVector)
      NewT = getTypeFreeFromSingleElementVector(ArgIt->getType());
    else
      NewT = getOriginalType(F, size_t(ArgNo) + 1);
    NewParams.push_back(NewT);
  }
  auto *NewReturnType = static_cast<Type *>(nullptr);
  if (!IsScalarToVector)
    NewReturnType = getTypeFreeFromSingleElementVector(F.getReturnType());
  else
    NewReturnType = getOriginalType(F, AttributeList::ReturnIndex);

  auto *NewFunctionType =
      FunctionType::get(NewReturnType, NewParams, F.isVarArg());
  if (NewFunctionType == F.getFunctionType())
    return F;

  auto &&NewF =
      *Function::Create(NewFunctionType, F.getLinkage(), F.getAddressSpace());

  assert(doesSignatureHaveSingleElementVector(F) ||
         doesSignatureHaveSingleElementVector(NewF));
  return NewF;
}

// Completely rewrites function in the entire module to its SEV-rich or SEV-free
// analogue depending on IsScalarToVector parameter
// This is a main util in this section
static void rewriteSingleElementVectorSignature(Function &F,
                                                bool IsScalarToVector) {
  auto &&NewF = getSingleElementVectorSignature(F, IsScalarToVector);
  if (&NewF == &F)
    return;

  NewF.copyAttributesFrom(&F);
  NewF.takeName(&F);
  NewF.copyMetadata(&F, 0);
  NewF.recalculateIntrinsicID();
  F.getParent()->getFunctionList().insert(F.getIterator(), &NewF);
#if VC_INTR_LLVM_VERSION_MAJOR > 15
  NewF.splice(NewF.begin(), &F);
#else
  NewF.getBasicBlockList().splice(NewF.begin(), F.getBasicBlockList());
#endif
  manageSingleElementVectorAttributes(F, NewF);

  if (NewF.size() > 0) {
    for (auto &&ArgPair : llvm::zip(F.args(), NewF.args()))
      replaceAllUsesWith(std::get<0>(ArgPair), std::get<1>(ArgPair), NewF);
    if (NewF.getReturnType() != F.getReturnType())
      rewriteSingleElementVectorReturns(NewF);
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

class SingleElementVectorInstRewriter
    : public InstVisitor<SingleElementVectorInstRewriter, Instruction *> {
public:
  using InstVisitor<SingleElementVectorInstRewriter, Instruction *>::visit;

  void visit(Function &F) {
    auto Instructions = getInstructions(F);
    for (auto *OldInst : Instructions) {
      if (!hasSingleElementVector(OldInst))
        continue;
      auto *NewInst = visit(*OldInst);
      if (NewInst)
        replaceAllUsesWith(OldInst, NewInst);
    }
  }

  Instruction *visitStoreInst(StoreInst &OldInst) {
    auto *NewT = static_cast<llvm::Type *>(nullptr);
    auto NewVals = ValueCont{};
    std::tie(NewT, NewVals) = getOperandsFreeFromSingleElementVector(OldInst);
    return new llvm::StoreInst(NewVals[0], NewVals[1], OldInst.isVolatile(),
                               VCINTR::Align::getAlign(&OldInst),
                               OldInst.getOrdering(), OldInst.getSyncScopeID(),
                               &OldInst);
  }
  Instruction *visitBinaryOperator(BinaryOperator &OldInst) {
    auto *NewT = static_cast<llvm::Type *>(nullptr);
    auto NewVals = ValueCont{};
    std::tie(NewT, NewVals) = getOperandsFreeFromSingleElementVector(OldInst);
    return BinaryOperator::Create(OldInst.getOpcode(), NewVals[0], NewVals[1],
                                  "", &OldInst);
  }
  Instruction *visitCmpInst(CmpInst &OldInst) {
    auto *NewT = static_cast<llvm::Type *>(nullptr);
    auto NewVals = ValueCont{};
    std::tie(NewT, NewVals) = getOperandsFreeFromSingleElementVector(OldInst);
    return CmpInst::Create(OldInst.getOpcode(), OldInst.getPredicate(),
                           NewVals[0], NewVals[1], "", &OldInst);
  }
  Instruction *visitShuffleVectorInst(ShuffleVectorInst &OldInst) {
    auto Mask = SmallVector<int, 16>{}; // Ensures copy
    OldInst.getShuffleMask(Mask);
    auto *Op0 = OldInst.getOperand(0);
    auto *Op1 = OldInst.getOperand(1);
    auto *Op0T = cast<VectorType>(Op0->getType());
    auto *Op1T = cast<VectorType>(Op1->getType());

    auto &&Context = OldInst.getContext();
    auto *Int32Ty = IntegerType::getInt32Ty(Context);
    if (Mask.size() == 1) {
      auto *VectorOp = static_cast<Value *>(nullptr);
      auto *Idx = static_cast<Value *>(nullptr);
      auto IsUndef = Mask[0] == VCINTR::ShuffleVectorInst::UndefMaskElem;
      if (IsUndef)
        VectorOp = UndefValue::get(
            VCINTR::getVectorType(Op0T->getElementType(), ShuffleVectorSize));
      else {
        auto IsUsedFirstOperand = static_cast<unsigned>(Mask[0]) <
                                  VCINTR::VectorType::getNumElements(Op0T);
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
    if (hasSingleElementVector(Op0T)) {
      NewOp0 = getTwoElementVectorFromOneElement(Op0, &OldInst);
      std::transform(Mask.begin(), Mask.end(), Mask.begin(), [](int El) {
        if (El > 0 && El != VCINTR::ShuffleVectorInst::UndefMaskElem)
          return El + 1;
        return El;
      });
    }
    if (hasSingleElementVector(Op1T))
      NewOp1 = getTwoElementVectorFromOneElement(Op1, &OldInst);

    return new ShuffleVectorInst(
        NewOp0, NewOp1,
        VCINTR::ShuffleVectorInst::getShuffleMask(Mask, Context), "", &OldInst);
  }
  Instruction *visitSelectInst(SelectInst &OldInst) {
    auto *NewT = static_cast<llvm::Type *>(nullptr);
    auto NewVals = ValueCont{};
    std::tie(NewT, NewVals) = getOperandsFreeFromSingleElementVector(OldInst);
    return SelectInst::Create(NewVals[0], NewVals[1], NewVals[2], "", &OldInst,
                              &OldInst);
  }
  Instruction *visitPHINode(PHINode &OldInst) {
    auto NewT = getTypeFreeFromSingleElementVector(OldInst.getType());
    auto Phi =
        PHINode::Create(NewT, OldInst.getNumIncomingValues(), "", &OldInst);
    for (auto I = size_t{0}; I < OldInst.getNumIncomingValues(); ++I) {
      auto *V = OldInst.getIncomingValue(I);
      auto *BB = OldInst.getIncomingBlock(I);
      auto *NewV = getValueFreeFromSingleElementVector(V, BB);
      Phi->addIncoming(NewV, BB);
    }
    return Phi;
  }
  Instruction *visitAllocaInst(AllocaInst &OldInst) {
    auto *NewT = getTypeFreeFromSingleElementVector(OldInst.getAllocatedType());
    return new llvm::AllocaInst(
        NewT, OldInst.getType()->getAddressSpace(), OldInst.getArraySize(),
        VCINTR::Align::getAlign(&OldInst), "", &OldInst);
  }
  Instruction *visitCastInst(CastInst &OldInst) {
    auto *NewT = static_cast<llvm::Type *>(nullptr);
    auto NewVals = ValueCont{};
    std::tie(NewT, NewVals) = getOperandsFreeFromSingleElementVector(OldInst);
    return CastInst::Create(OldInst.getOpcode(), NewVals[0], NewT, "",
                            &OldInst);
  }
  Instruction *visitLoadInst(LoadInst &OldInst) {
    auto *NewT = static_cast<llvm::Type *>(nullptr);
    auto NewVals = ValueCont{};
    std::tie(NewT, NewVals) = getOperandsFreeFromSingleElementVector(OldInst);
    return new llvm::LoadInst(NewT, NewVals[0], "", OldInst.isVolatile(),
                              VCINTR::Align::getAlign(&OldInst),
                              OldInst.getOrdering(), OldInst.getSyncScopeID(),
                              &OldInst);
  }
  Instruction *visitUnaryOperator(UnaryOperator &OldInst) {
    auto *NewT = static_cast<llvm::Type *>(nullptr);
    auto NewVals = ValueCont{};
    std::tie(NewT, NewVals) = getOperandsFreeFromSingleElementVector(OldInst);
    return UnaryOperator::Create(OldInst.getOpcode(), NewVals[0], "", &OldInst);
  }
  Instruction *visitVAArgInst(VAArgInst &OldInst) {
    auto *NewT = static_cast<llvm::Type *>(nullptr);
    auto NewVals = ValueCont{};
    std::tie(NewT, NewVals) = getOperandsFreeFromSingleElementVector(OldInst);
    return new VAArgInst(NewVals[0], NewT, "", &OldInst);
  }
  Instruction *visitExtractValueInst(ExtractValueInst &OldInst) {
    auto *NewT = static_cast<llvm::Type *>(nullptr);
    auto NewVals = ValueCont{};
    std::tie(NewT, NewVals) = getOperandsFreeFromSingleElementVector(OldInst);
    return ExtractValueInst::Create(NewVals[0], OldInst.getIndices(), "",
                                    &OldInst);
  }
  Instruction *visitGetElementPtrInst(GetElementPtrInst &OldInst) {
    auto *NewT = static_cast<llvm::Type *>(nullptr);
    auto NewVals = ValueCont{};
    std::tie(NewT, NewVals) = getOperandsFreeFromSingleElementVector(OldInst);
    std::vector<Value *> IdxList;
    std::transform(NewVals.begin() + 1, NewVals.end(),
                   std::back_inserter(IdxList), [](Value *V) { return V; });
    auto *PointeeType = VCINTR::Type::getNonOpaquePtrEltTy(
        cast<PointerType>(NewVals[0]->getType()->getScalarType()));
    return GetElementPtrInst::Create(PointeeType, NewVals[0], IdxList, "",
                                     &OldInst);
  }
  Instruction *visitExtractElementInst(ExtractElementInst &OldInst) {
    // No processing required
    // Extracts and Inserts will be collapsed later
    return nullptr;
  }
  Instruction *visitInsertElementInst(InsertElementInst &OldInst) {
    // No processing required
    // Extracts and Inserts will be collapsed later
    return nullptr;
  }
  Instruction *visitInstruction(Instruction &I) {
    // For CallInst this is a bug, because
    // Calls have been processed in rewriteSingleElementVectorUses function
    // For ReturnInst this is a bug, because
    // Returns have been processed in rewriteSingleElementVectorReturn function
    // For other cases this assert is due to "visit" method is not implemented
    assert(false && "Oops... Cannot rewrite instruction!");
    return nullptr;
  }
};

/// This section contains utils for rewriting global variables

// For conversion in SEV-rich to SEV-free direction
// this function adds VCSingleElementVector attribute to global var
static void manageSingleElementVectorAttribute(GlobalVariable &GV, Type *OldT,
                                               Type *NewT) {
  if (hasSingleElementVector(OldT)) {
    assert(!hasSingleElementVector(NewT));
    auto InnerPtrs = std::to_string(getInnerPointerVectorNesting(OldT));
    GV.addAttribute(VCModuleMD::VCSingleElementVector, InnerPtrs);
  }
}

static GlobalVariable &createAndTakeFrom(GlobalVariable &GV, PointerType *NewT,
                                         Constant *Initializer) {
  auto *NewGV = new GlobalVariable(
      *GV.getParent(), VCINTR::Type::getNonOpaquePtrEltTy(NewT), GV.isConstant(),
      GV.getLinkage(), Initializer, "sev.global.", &GV, GV.getThreadLocalMode(),
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

static void rewriteGlobalVariable(GlobalVariable &GV) {
  auto *T = cast<PointerType>(GV.getType());
  auto *NewT = cast<PointerType>(getTypeFreeFromSingleElementVector(T));
  if (NewT == T)
    return;
  auto *Initializer = static_cast<Constant *>(nullptr);
  if (GV.hasInitializer())
    Initializer = cast<Constant>(createVectorToScalarValue(
        GV.getInitializer(), static_cast<Instruction *>(nullptr)));
  auto &&NewGV = createAndTakeFrom(GV, NewT, Initializer);
  while (GV.use_begin() != GV.use_end()) {
    auto &&Use = GV.use_begin();
    auto *Inst = cast<Instruction>(Use->getUser());
    auto *V = createScalarToVectorValue(&NewGV, T, Inst);
    *Use = V;
  }
  manageSingleElementVectorAttribute(NewGV, T, NewT);
  GV.eraseFromParent();
}

static void restoreGlobalVariable(GlobalVariable &GV) {
  auto *T = cast<PointerType>(GV.getType());
  if (!GV.hasAttribute(VCModuleMD::VCSingleElementVector))
    return;
  auto InnerPtrsStr =
      GV.getAttribute(VCModuleMD::VCSingleElementVector).getValueAsString();
  auto InnerPtrs = InnerPtrsStr.empty() ? 0 : std::stoull(InnerPtrsStr.str());
  auto *NewT = cast<PointerType>(getTypeWithSingleElementVector(T, InnerPtrs));
  if (NewT == T)
    return;
  auto *Initializer = static_cast<Constant *>(nullptr);
  if (GV.hasInitializer())
    Initializer = cast<Constant>(createScalarToVectorValue(
        GV.getInitializer(), VCINTR::Type::getNonOpaquePtrEltTy(NewT),
        static_cast<Instruction *>(nullptr)));
  auto &&NewGV = createAndTakeFrom(GV, NewT, Initializer);
  while (GV.use_begin() != GV.use_end()) {
    auto &&Use = GV.use_begin();
    auto *Inst = cast<Instruction>(Use->getUser());
    auto *V = createVectorToScalarValue(&NewGV, Inst);
    *Use = V;
  }
  manageSingleElementVectorAttribute(NewGV, T, NewT);
  GV.eraseFromParent();
}

static void rewriteGlobalVariables(Module &M, bool IsScalarToVector = false) {
  auto &&Globals = getGlobalVariables(M);
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

static void collapseBitcastInst(BitCastInst *BitCast, bool CollapseCannotFail) {
  if (BitCast->user_empty()) {
    BitCast->eraseFromParent();
    return;
  }
  auto &&M = *BitCast->getModule();
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
// %b = bitcast <T> %a to <U>
// %d = bitcast <U> %b to <T>
// %e = some_user_of_T <T> %d
//
// This util collapses such pairs of bitcasts in two iterations:
// First iteration will remove %d
// Second iteration will remove %b
static void collapseBitcastInstructions(Function &F,
                                        bool CollapseCannotFail = true) {
  for (auto i = size_t{0}; i < 2; ++i) {
    auto Instructions = getInstructions(F);
    for (auto *I : Instructions) {
      if (auto *BitCast = dyn_cast<BitCastInst>(I)) {
        auto HasSEV =
            hasSingleElementVector(BitCast->getOperand(0)->getType()) ||
            hasSingleElementVector(BitCast->getType());
        collapseBitcastInst(BitCast, i && CollapseCannotFail && HasSEV);
      }
    }
  }
}

static void collapseExtractInst(ExtractElementInst *Extract,
                                bool CollapseCannotFail) {
  if (Extract->user_empty()) {
    Extract->eraseFromParent();
    return;
  }
  auto &&M = *Extract->getModule();
  auto &&Q = SimplifyQuery(M.getDataLayout());
  auto *ReplaceWith = VCINTR::SimplifyExtractElementInst(
      Extract->getOperand(0), Extract->getOperand(1), Q);
  if (!CollapseCannotFail && !ReplaceWith)
    return;
  assert(ReplaceWith && "Oops... Cannot collapse ExtractElement instruction");
  Extract->replaceAllUsesWith(ReplaceWith);
  Extract->eraseFromParent();
}

static void collapseInsertInst(InsertElementInst *Insert,
                               bool CollapseCannotFail) {
  if (Insert->user_empty()) {
    Insert->eraseFromParent();
    return;
  }
  auto &&M = *Insert->getModule();
  auto &&Q = SimplifyQuery(M.getDataLayout());
  auto *ReplaceWith = VCINTR::SimplifyInsertElementInst(
      Insert->getOperand(0), Insert->getOperand(1), Insert->getOperand(2), Q);

  // SimplifyInsertElementInst provides too simple analysis
  // which does not work in some cases handled below:
  if (!ReplaceWith && hasSingleElementVector(Insert->getType())) {
    auto *Scal = Insert->getOperand(1);
    auto *VecTy = cast<VectorType>(Insert->getType());
    if (auto *Extract = dyn_cast<ExtractElementInst>(Scal)) {
      if (hasSingleElementVector(Extract->getOperand(0)->getType()))
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
// %v = insertelement <1 x T> %u, <T> %element, <i32> %zero_idx
// %s = extractlement <1 x T> %v, <i32> %zero_idx
// %e = some_user_of_T <T> %s
// 2.
// %s = extractlement <n x T> %ConstantVector, <i32> %ConstantIdx
// %e = some_user_of_T <T> %s
// This utility removes excessive ExtractElement instructions
//
// After rewriting instructions from SEV-rich form to SEV-free one
// collapseInsertInstructions utility leaves lots of ExtractElement instructions
// with no users. This utility removes them as well
static void collapseExtractInstructions(Function &F,
                                        bool CollapseCannotFail = true) {
  auto Instructions = getInstructions(F);
  for (auto *I : Instructions) {
    if (auto *Extract = dyn_cast<ExtractElementInst>(I)) {
      auto OpT = I->getOperand(0)->getType();
      collapseExtractInst(Extract,
                          CollapseCannotFail && hasSingleElementVector(OpT));
    }
  }
}

// After rewriting instructions from SEV-rich form to SEV-free one
// There are lots of auxiliary pairs of extract-insert
// instructions left, like these:
// 1.
// %s = extractlement <1 x T> %v, <i32> %idx
// %g = insertelement <1 x T> %u, <T> %s, <i32> %idx
// %e = some_user_of_1T <1 x T> %g
// 2.
// %g = insertelement <1 x T> %v, <T> %ConstantElement, <i32> %idx
// %e = some_user_of_T <T> %g
// This utility removes excessive InsertElement instructions
//
// After rewriting instructions from SEV-free form to SEV-rich one
// collapseExtractInstructions utility leaves lots of InsertElement instructions
// with no users. This utility removes them as well
static void collapseInsertInstructions(Function &F,
                                       bool CollapseCannotFail = true) {
  auto Instructions = getInstructions(F);
  for (auto *I : Instructions)
    if (auto *Insert = dyn_cast<InsertElementInst>(I))
      collapseInsertInst(Insert, CollapseCannotFail &&
                                     hasSingleElementVector(I->getType()));
}

/// This section contains upper-level functions
/// for calling in GenXSPIRV adaptors
/// They either remove or restore Single Element Vectors in the module

void rewriteSingleElementVectors(Module &M) {
  rewriteGlobalVariables(M, /*IsScalarToVector=*/false);

  auto Functions = getFunctions(M);
  for (auto *F : Functions)
    rewriteSingleElementVectorSignature(*F, /*IsScalarToVector=*/false);
  // Functions container should be refreshed after signatures rewriting
  Functions = getFunctions(M);

  SingleElementVectorInstRewriter{}.visit(M);

  for (auto *F : Functions) {
    collapseExtractInstructions(*F);
    collapseInsertInstructions(*F);
  }

  for (auto *F : Functions)
    collapseBitcastInstructions(*F);
}

void restoreSingleElementVectors(Module &M) {
  rewriteGlobalVariables(M, /*IsScalarToVector=*/true);

  auto Functions = getFunctions(M);
  for (auto *F : Functions)
    rewriteSingleElementVectorSignature(*F, /*IsScalarToVector=*/true);
  // Functions container should be refreshed after signatures rewriting
  Functions = getFunctions(M);

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

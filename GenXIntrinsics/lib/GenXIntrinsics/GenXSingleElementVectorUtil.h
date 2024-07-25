/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This file declares class for rewriting single element vectors
// in GenXSPIRV adaptors.

#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Module.h"

#include "llvmVCWrapper/IR/DerivedTypes.h"
#include "llvmVCWrapper/IR/Function.h"
#include "llvmVCWrapper/IR/Instructions.h"
#include "llvmVCWrapper/IR/Type.h"

#include <unordered_map>
#include <unordered_set>

namespace llvm {
namespace genx {

class SEVUtil : public InstVisitor<SEVUtil, Instruction *> {
private:
  using ValueCont = SmallVector<Value *, 4>;
  using InstVisitor<SEVUtil, Instruction *>::visit;

  Module &M;

  bool NeedCollapse = false;

  std::unordered_map<StructType *, StructType *> SEVFreeStructMap;
  std::unordered_map<StructType *, StructType *> SEVRichStructMap;
  std::unordered_set<StructType *> SEVFreeStructTypes;

  std::vector<Function *> getFunctions();
  std::vector<GlobalVariable *> getGlobalVariables();
  std::vector<Instruction *> getInstructions(Function &F);
  ConstantInt *getVectorIndex(size_t idx);
  int64_t getConstantElement(ConstantInt *Const);
  size_t getPointerNesting(Type *Ty, Type **ReturnNested = nullptr);
  size_t getPointerVectorNesting(Type *Ty, Type **ReturnNested = nullptr);
  size_t getInnerPointerVectorNesting(Type *Ty);
  Type *getTypeFreeFromSEV(Type *Ty);
  Type *getTypeWithSEV(Type *Ty, size_t InnerPointers = 0);
  bool hasSEV(Type *Ty);
  bool hasSEV(Instruction *I);
  bool doesSignatureHaveSEV(Function &F);
  Value *createVectorToScalarValue(Value *Vector, Instruction *InsertBefore,
                                   size_t idx = 0);
  Value *createVectorToScalarValue(Value *Vector, BasicBlock *BB,
                                   size_t idx = 0);
  Value *createScalarToVectorValue(Value *Scalar, Type *RefTy,
                                   Instruction *InsertBefore);
  Value *getValueFreeFromSEV(Value *OldV, Instruction *InsertBefore);
  Value *getValueFreeFromSEV(Value *OldV, BasicBlock *BB);
  Value *getValueWithSEV(Value *OldV, Type *RefTy, Instruction *InsertBefore);
  std::pair<llvm::Type *, ValueCont>
  getOperandsFreeFromSEV(Instruction &OldInst);
  Value *getTwoElementVectorFromOneElement(Value *V, Instruction *InsertBefore);
  void replaceAllUsesWith(Instruction *OldInst, Instruction *NewInst);
  void replaceAllUsesWith(Function &OldF, Function &NewF);
  void replaceAllUsesWith(Argument &OldArg, Argument &NewArg, Function &NewF);
  void rewriteSEVReturns(Function &NewF);
  void manageSEVAttribute(Function &NewF, Type *OldTy, Type *NewTy,
                          size_t AttrNo);
  void manageSEVAttributes(Function &OldF, Function &NewF);
  Type *getOriginalType(Function &F, size_t AttrNo);
  Function &getSEVSignature(Function &F, bool IsScalarToVector);
  void rewriteSEVSignature(Function &F, bool IsScalarToVector);
  void visit(Function &F);
  Instruction *visitStoreInst(StoreInst &OldInst);
  Instruction *visitBinaryOperator(BinaryOperator &OldInst);
  Instruction *visitCmpInst(CmpInst &OldInst);
  Instruction *visitShuffleVectorInst(ShuffleVectorInst &OldInst);
  Instruction *visitSelectInst(SelectInst &OldInst);
  Instruction *visitPHINode(PHINode &OldInst);
  Instruction *visitAllocaInst(AllocaInst &OldInst);
  Instruction *visitCastInst(CastInst &OldInst);
  Instruction *visitLoadInst(LoadInst &OldInst);
  Instruction *visitUnaryOperator(UnaryOperator &OldInst);
  Instruction *visitVAArgInst(VAArgInst &OldInst);
  Instruction *visitExtractValueInst(ExtractValueInst &OldInst);
  Instruction *visitGetElementPtrInst(GetElementPtrInst &OldInst);
  Instruction *visitExtractElementInst(ExtractElementInst &OldInst);
  Instruction *visitInsertElementInst(InsertElementInst &OldInst);
  Instruction *visitInstruction(Instruction &I);
  void manageSEVAttribute(GlobalVariable &GV, Type *OldTy, Type *NewTy);
  GlobalVariable &createAndTakeFrom(GlobalVariable &GV, PointerType *NewTy,
                                    Constant *Initializer);
  void rewriteGlobalVariable(GlobalVariable &GV);
  void restoreGlobalVariable(GlobalVariable &GV);
  void rewriteGlobalVariables(bool IsScalarToVector = false);
  void collapseBitcastInst(BitCastInst *BitCast, bool CollapseCannotFail);
  void collapseBitcastInstructions(Function &F, bool CollapseCannotFail = true);
  void collapseExtractInst(ExtractElementInst *Extract,
                           bool CollapseCannotFail);
  void collapseInsertInst(InsertElementInst *Insert, bool CollapseCannotFail);
  void collapseExtractInstructions(Function &F, bool CollapseCannotFail = true);
  void collapseInsertInstructions(Function &F, bool CollapseCannotFail = true);

public:
  SEVUtil(Module &InM) : M{InM} {}

  void rewriteSEVs();
  void restoreSEVs();

  friend class InstVisitor<SEVUtil, Instruction *>;
};

} // namespace genx
} // namespace llvm

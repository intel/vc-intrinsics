/*========================== begin_copyright_notice ============================

Copyright (C) 2019-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This is the worker class to lowers CM SIMD control flow into a form where
// the IR reflects the semantics. See CMSimdCFLowering.cpp for details.

#ifndef CMSIMDCF_LOWER_H
#define CMSIMDCF_LOWER_H

#include "llvm/PassRegistry.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include <algorithm>
#include <map>
#include <set>

namespace llvm {

void initializeCMSimdCFLoweringPass(PassRegistry &);

// The worker class for lowering CM SIMD CF
class CMSimdCFLower {
  Function *F = {};
  // A map giving the basic blocks ending with a simd branch, and the simd
  // width of each one.
  MapVector<BasicBlock *, unsigned> SimdBranches;
  // A map giving the basic blocks to be predicated, and the simd width of
  // each one.
  MapVector<BasicBlock *, unsigned> PredicatedBlocks;
  // The join points, together with the simd width of each one.
  MapVector<BasicBlock *, unsigned> JoinPoints;
  // Mapping of join points to their correspond goto BBs
  std::map<BasicBlock *, BasicBlock *> JoinToGoto;
  // The JIP for each simd branch and join point.
  std::map<BasicBlock *, BasicBlock *> JIPs;
  // Subroutines that are predicated, mapping to the simd width.
  std::map<Function *, unsigned> PredicatedSubroutines;
  // Execution mask variable.
  GlobalVariable *EMVar;
  // Resume mask for each join point.
  std::map<BasicBlock *, AllocaInst *> RMAddrs;
  // Set of intrinsic calls (other than wrregion) that have been predicated.
  std::set<Value *> AlreadyPredicated;
  // Mask for shufflevector to extract part of EM.
  SmallVector<Constant *, 32> ShuffleMask;
  // Original predicate for an instruction (if it was changed with AND respect
  // to EM)
  std::map<Instruction *, Value *> OriginalPred;
  // Replicate mask for provided number of channels
  Value *replicateMask(Value *EM, Instruction *InsertBefore, unsigned SimdWidth,
                       unsigned NumChannels = 1);

  void eraseInstruction(Instruction *I) {
    assert(!AlreadyPredicated.count(I) &&
           "Shouldn't erase this instruction as it's predicated");
    I->eraseFromParent();
  }

public:
  static const unsigned MAX_SIMD_CF_WIDTH = 32;

  CMSimdCFLower(GlobalVariable *EMask) : EMVar(EMask) {}

  static CallInst *isSimdCFAny(Value *V);
  static Use *getSimdConditionUse(Value *Cond);

  void processFunction(Function *F);

private:
  bool findSimdBranches(unsigned CMWidth);
  void determinePredicatedBlocks();
  void markPredicatedBranches();
  void fixSimdBranches();
  void findAndSplitJoinPoints();
  void determineJIPs();
  void determineJIP(BasicBlock *BB, std::map<BasicBlock *, unsigned> *Numbers, bool IsJoin);

  // Methods to add predication to the code
  void predicateCode(unsigned CMWidth);
  void predicateBlock(BasicBlock *BB, unsigned SimdWidth);
  void predicateInst(Instruction *Inst, unsigned SimdWidth);
  void rewritePredication(CallInst *CI, unsigned SimdWidth);
  void predicateStore(Instruction *SI, unsigned SimdWidth);
  void predicateSend(CallInst *CI, unsigned IntrinsicID, unsigned SimdWidth);
  void predicateScatterGather(CallInst *CI, unsigned SimdWidth, unsigned PredOperandNum);
  CallInst *predicateWrRegion(CallInst *WrR, unsigned SimdWidth);
  void predicateCall(CallInst *CI, unsigned SimdWidth);

  void lowerSimdCF();
  void lowerUnmaskOps();
  unsigned deduceNumChannels(Instruction *SI);
  Instruction *loadExecutionMask(Instruction *InsertBefore, unsigned SimdWidth);
  Value *getRMAddr(BasicBlock *JP, unsigned SimdWidth);
};

} // namespace

#endif

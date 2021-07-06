/*========================== begin_copyright_notice ============================

Copyright (C) 2019-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//===----------------------------------------------------------------------===//
//
// This file defines a set of enums which allow processing of intrinsic
// functions.  Values of these enum types are returned by
// GenXIntrinsic::getGenXIntrinsicID.
//
//===----------------------------------------------------------------------===//

#ifndef GENX_INTRINSIC_INTERFACE_H
#define GENX_INTRINSIC_INTERFACE_H

#include "llvm/IR/Module.h"

#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Instructions.h"
#include "llvm/GenXIntrinsics/GenXVersion.h"

namespace llvm {

namespace GenXIntrinsic {
enum ID : unsigned {
  not_genx_intrinsic = Intrinsic::num_intrinsics,
#define GET_INTRINSIC_ENUM_VALUES
#include "llvm/GenXIntrinsics/GenXIntrinsicDescription.gen"
#undef GET_INTRINSIC_ENUM_VALUES
  num_genx_intrinsics,
  // note that Intrinsic::not_intrinsic means that it is not a LLVM intrinsic
  not_any_intrinsic
};

namespace GenXResult {
  enum ResultIndexes {
    IdxAddc_Add    = 1,
    IdxAddc_Carry  = 0,
    IdxSubb_Sub    = 1,
    IdxSubb_Borrow = 0
  };
}


namespace GenXRegion {
enum {
  // Operands in both rdregion and wrregion:
  OldValueOperandNum = 0,
  // Operands in rdregion:
  RdVStrideOperandNum = 1,
  RdWidthOperandNum = 2,
  RdStrideOperandNum = 3,
  RdIndexOperandNum = 4,
  // Operands in wrregion:
  NewValueOperandNum = 1,
  WrVStrideOperandNum = 2,
  WrWidthOperandNum = 3,
  WrStrideOperandNum = 4,
  WrIndexOperandNum = 5,
  PredicateOperandNum = 7
};
} // namespace GenXRegion

static inline const char *getGenXIntrinsicPrefix() { return "llvm.genx."; }

ID getGenXIntrinsicID(const Function *F);

/// Utility function to get the genx_intrinsic ID if V is a GenXIntrinsic call.
/// V is allowed to be 0.
static inline ID getGenXIntrinsicID(const Value *V) {
  if (V)
    if (const CallInst *CI = dyn_cast<CallInst>(V))
      if (Function *Callee = CI->getCalledFunction())
        return getGenXIntrinsicID(Callee);
  return GenXIntrinsic::not_genx_intrinsic;
}

/// GenXIntrinsic::isGenXIntrinsic(ID) - Is GenX intrinsic
/// NOTE that this is include not_genx_intrinsic
/// BUT DOES NOT include not_any_intrinsic
static inline bool isGenXIntrinsic(unsigned ID) {
  return ID >= not_genx_intrinsic && ID < num_genx_intrinsics;
}

/// GenXIntrinsic::isGenXIntrinsic(CF) - Returns true if
/// the function's name starts with "llvm.genx.".
/// It's possible for this function to return true while getGenXIntrinsicID()
/// returns GenXIntrinsic::not_genx_intrinsic!
static inline bool isGenXIntrinsic(const Function *CF) {
  return CF->getName().startswith(getGenXIntrinsicPrefix());
}

/// GenXIntrinsic::isGenXIntrinsic(V) - Returns true if
/// the function's name starts with "llvm.genx.".
/// It's possible for this function to return true while getGenXIntrinsicID()
/// returns GenXIntrinsic::not_genx_intrinsic!
static inline bool isGenXIntrinsic(const Value *V) {
  if (V)
    if (const CallInst *CI = dyn_cast<CallInst>(V))
      if (Function *Callee = CI->getCalledFunction())
        return isGenXIntrinsic(Callee);
  return false;
}

/// GenXIntrinsic::isGenXNonTrivialIntrinsic(ID) - Is GenX intrinsic,
/// which is not equal to not_genx_intrinsic or not_any_intrinsic
static inline bool isGenXNonTrivialIntrinsic(unsigned ID) {
  return ID > not_genx_intrinsic && ID < num_genx_intrinsics;
}

/// GenXIntrinsic::isGenXNonTrivialIntrinsic(CF) - Returns true if
/// CF is genx intrinsic, not equal to not_any_intrinsic or not_genx_intrinsic
static inline bool isGenXNonTrivialIntrinsic(const Function *CF) {
  return isGenXNonTrivialIntrinsic(getGenXIntrinsicID(CF));
}

/// GenXIntrinsic::isGenXNonTrivialIntrinsic(V) - Returns true if
/// V is genx intrinsic, not equal to not_any_intrinsic or not_genx_intrinsic
static inline bool isGenXNonTrivialIntrinsic(const Value *V) {
  return isGenXNonTrivialIntrinsic(getGenXIntrinsicID(V));
}

/// GenXIntrinsic::getGenXName(ID) - Return the LLVM name for a GenX intrinsic,
/// such as "llvm.genx.lane.id".
std::string getGenXName(ID id, ArrayRef<Type *> Tys = None);

ID lookupGenXIntrinsicID(StringRef Name);

AttributeList getAttributes(LLVMContext &C, ID id);

/// GenXIntrinsic::getGenXType(ID) - Return the function type for an intrinsic.
FunctionType *getGenXType(LLVMContext &Context, GenXIntrinsic::ID id,
                          ArrayRef<Type *> Tys = None);

/// GenXIntrinsic::getGenXDeclaration(M, ID) - Create or insert a GenX LLVM
/// Function declaration for an intrinsic, and return it.
///
/// The Tys parameter is for intrinsics with overloaded types (e.g., those
/// using iAny, fAny, vAny, or iPTRAny).  For a declaration of an overloaded
/// intrinsic, Tys must provide exactly one type for each overloaded type in
/// the intrinsic.
Function *getGenXDeclaration(Module *M, ID id, ArrayRef<Type *> Tys = None);

void getIntrinsicInfoTableEntries(
    GenXIntrinsic::ID id,
    SmallVectorImpl<Intrinsic::IITDescriptor> &T);

/// GenXIntrinsic::resetGenXAttributes(F) - recalculates attributes
/// of a CM intrinsic by setting the default values (as per
/// intrinsic definition).
///
/// F is required to be a GenX intrinsic function
void resetGenXAttributes(Function* F);

/// GenXIntrinsic::getAnyIntrinsicID(F) - Return LLVM or GenX intrinsic ID
/// If is not intrinsic returns not_any_intrinsic
/// Note that Function::getIntrinsicID returns ONLY LLVM intrinsics
static inline unsigned getAnyIntrinsicID(const Function *F) {
  if (isGenXNonTrivialIntrinsic(F))
    return getGenXIntrinsicID(F);
  else {
    assert(F);
    unsigned IID = F->getIntrinsicID();
    if (IID == Intrinsic::not_intrinsic)
      return GenXIntrinsic::not_any_intrinsic;
    else
      return IID;
  }
}

/// Utility function to get the LLVM or GenX intrinsic ID if V is an intrinsic
/// call.
/// V is allowed to be 0.
static inline unsigned getAnyIntrinsicID(const Value *V) {
  if (V)
    if (const CallInst *CI = dyn_cast<CallInst>(V))
      if (Function *Callee = CI->getCalledFunction())
        return getAnyIntrinsicID(Callee);
  return GenXIntrinsic::not_any_intrinsic;
}

/// GenXIntrinsic::isAnyIntrinsic(ID) - Is any intrinsic
/// including not_any_intrinsic
static inline bool isAnyIntrinsic(unsigned id) {
  assert(id != not_genx_intrinsic && id != Intrinsic::not_intrinsic &&
         "Do not use this method with getGenXIntrinsicID or getIntrinsicID!");
  return id < num_genx_intrinsics || id == not_any_intrinsic;
}

/// GenXIntrinsic::isAnyNonTrivialIntrinsic(id) - Is GenX or LLVM intrinsic,
/// which is not equal to not_any_intrinsic
static inline bool isAnyNonTrivialIntrinsic(unsigned id) {
  assert(id != not_genx_intrinsic && id != Intrinsic::not_intrinsic &&
         "Do not use this method with getGenXIntrinsicID or getIntrinsicID!");
  return id <  num_genx_intrinsics &&
         id != not_any_intrinsic;
}

/// GenXIntrinsic::isAnyNonTrivialIntrinsic(ID) - Is GenX or LLVM intrinsic,
/// which is not equal to not_genx_intrinsic, not_any_intrinsic or not_intrinsic
static inline bool isAnyNonTrivialIntrinsic(const Function *CF) {
  return isAnyNonTrivialIntrinsic(getAnyIntrinsicID(CF));
}

/// Utility function to check if V is LLVM or GenX intrinsic call,
/// which is not not_intrinsic, not_genx_intrinsic or not_any_intrinsic
/// V is allowed to be 0.
static inline bool isAnyNonTrivialIntrinsic(const Value *V) {
  return isAnyNonTrivialIntrinsic(getAnyIntrinsicID(V));
}

/// GenXIntrinsic::getAnyName(ID) - Return the LLVM name for LLVM or GenX
/// intrinsic, such as "llvm.genx.lane.id".
std::string getAnyName(unsigned id, ArrayRef<Type *> Tys = None);

/// GenXIntrinsic::getAnyType(ID) - Return the function type for an intrinsic.
static inline FunctionType *getAnyType(LLVMContext &Context, unsigned id,
                                       ArrayRef<Type *> Tys = None) {
  assert(isAnyNonTrivialIntrinsic(id));
  if (isGenXIntrinsic(id))
    return getGenXType(Context, (ID)id, Tys);
  else
    return Intrinsic::getType(Context, (Intrinsic::ID)id, Tys);
}

/// GenXIntrinsic::isSupportedPlatform(CPU, ID) - Return true if GenxIntrinsic
// is supported by current platform
bool isSupportedPlatform(const std::string &CPU, unsigned id);

/// GenXIntrinsic::isOverloadedArg(ID, ArgNum) - Return true if ArgNum
/// in intrinsic overloaded
bool isOverloadedArg(unsigned IntrinID, unsigned ArgNum);

/// GenXIntrinsic::isOverloadedRet(ID) - Return true if return type
/// in intrinsic is overloaded
bool isOverloadedRet(unsigned IntrinID);

/// GenXIntrinsic::getAnyDeclaration(M, ID) - Create or insert a LLVM
/// Function declaration for an intrinsic, and return it.
///
/// The Tys parameter is for intrinsics with overloaded types (e.g., those
/// using iAny, fAny, vAny, or iPTRAny).  For a declaration of an overloaded
/// intrinsic, Tys must provide exactly one type for each overloaded type in
/// the intrinsic.
static inline Function *getAnyDeclaration(Module *M, unsigned id,
                                          ArrayRef<Type *> Tys = None) {
  assert(isAnyNonTrivialIntrinsic(id));
  if (isGenXIntrinsic(id)) {
    return getGenXDeclaration(M, (ID)id, Tys);
  } else {
    return Intrinsic::getDeclaration(M, (Intrinsic::ID)id, Tys);
  }
}

/// GenXIntrinsic::getGenXMulIID(S1, S2) - returns GenXIntrinsic::ID for
/// the enx_XXmul opertation, where XX is is defined by the input arguments
/// which represent signs of the operands
static inline GenXIntrinsic::ID getGenXMulIID(bool LHSign, bool RHSign) {
  return LHSign
             ? (RHSign ? GenXIntrinsic::genx_ssmul : GenXIntrinsic::genx_sumul)
             : (RHSign ? GenXIntrinsic::genx_usmul : GenXIntrinsic::genx_uumul);
}

static inline bool isRdRegion(unsigned IntrinID) {
  switch (IntrinID) {
  case GenXIntrinsic::genx_rdregioni:
  case GenXIntrinsic::genx_rdregionf:
    return true;
  default:
    return false;
  }
}

static inline bool isRdRegion(const Function *F) {
  return isRdRegion(getGenXIntrinsicID(F));
}

static inline bool isRdRegion(const Value *V) {
  return isRdRegion(getGenXIntrinsicID(V));
}

static inline bool isWrRegion(unsigned IntrinID) {
  switch (IntrinID) {
  case GenXIntrinsic::genx_wrregioni:
  case GenXIntrinsic::genx_wrregionf:
  case GenXIntrinsic::genx_wrconstregion:
    return true;
  default:
    return false;
  }
}

static inline bool isWrRegion(const Function *F) {
  return isWrRegion(getGenXIntrinsicID(F));
}

static inline bool isWrRegion(const Value *V) {
  return isWrRegion(getGenXIntrinsicID(V));
}

static inline bool isAbs(unsigned IntrinID) {
    if (IntrinID == GenXIntrinsic::genx_absf || IntrinID == GenXIntrinsic::genx_absi)
        return true;
    return false;
}

static inline bool isAbs(const Function *F) {
    return isAbs(getGenXIntrinsicID(F));
}

static inline bool isAbs(const Value *V) {
    return isAbs(getGenXIntrinsicID(V));
}

static inline bool isIntegerSat(unsigned IID) {
    switch (IID) {
    case GenXIntrinsic::genx_sstrunc_sat:
    case GenXIntrinsic::genx_sutrunc_sat:
    case GenXIntrinsic::genx_ustrunc_sat:
    case GenXIntrinsic::genx_uutrunc_sat:
        return true;
    default:
        return false;
    }
}

static inline bool isIntegerSat(const Function *F) {
  return isIntegerSat(getGenXIntrinsicID(F));
}

static inline bool isIntegerSat(const Value *V) {
  return isIntegerSat(getGenXIntrinsicID(V));
}

static inline bool isVLoad(unsigned IntrinID) {
  return IntrinID == GenXIntrinsic::genx_vload;
}

static inline bool isVLoad(const Function *F) {
  return isVLoad(getGenXIntrinsicID(F));
}

static inline bool isVLoad(const Value *V) {
  return isVLoad(getGenXIntrinsicID(V));
}

static inline bool isVStore(unsigned IntrinID) {
  return IntrinID == GenXIntrinsic::genx_vstore;
}

static inline bool isVStore(const Function *F) {
  return isVStore(getGenXIntrinsicID(F));
}

static inline bool isVStore(const Value *V) {
  return isVStore(getGenXIntrinsicID(V));
}

static inline bool isVLoadStore(unsigned IntrinID) {
  return isVLoad(IntrinID) || isVStore(IntrinID);
}

static inline bool isVLoadStore(const Function *F) {
  return isVLoadStore(getGenXIntrinsicID(F));
}

static inline bool isVLoadStore(const Value *V) {
  return isVLoadStore(getGenXIntrinsicID(V));
}

static inline bool isReadPredefReg(unsigned IntrinID) {
  return IntrinID == GenXIntrinsic::genx_read_predef_reg;
}

static inline bool isReadPredefReg(const Function *F) {
  return isReadPredefReg(getGenXIntrinsicID(F));
}

static inline bool isReadPredefReg(const Value *V) {
  return isReadPredefReg(getGenXIntrinsicID(V));
}

static inline bool isWritePredefReg(unsigned IntrinID) {
  return IntrinID == GenXIntrinsic::genx_write_predef_reg;
}

static inline bool isWritePredefReg(const Function *F) {
  return isWritePredefReg(getGenXIntrinsicID(F));
}

static inline bool isWritePredefReg(const Value *V) {
  return isWritePredefReg(getGenXIntrinsicID(V));
}

static inline bool isReadWritePredefReg(unsigned IntrinID) {
  return isWritePredefReg(IntrinID) || isReadPredefReg(IntrinID);
}

static inline bool isReadWritePredefReg(const Value *V) {
  return isWritePredefReg(getGenXIntrinsicID(V)) ||
         isReadPredefReg(getGenXIntrinsicID(V));
}

static inline bool isReadWritePredefReg(const Function *F) {
  return isWritePredefReg(getGenXIntrinsicID(F)) ||
         isReadPredefReg(getGenXIntrinsicID(F));
}


} // namespace GenXIntrinsic

// todo: delete this
namespace GenXIntrinsic {
AttributeList getAttributes(LLVMContext &C, ID id);

} // namespace GenXIntrinsic

} // namespace llvm

#endif

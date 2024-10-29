/*========================== begin_copyright_notice ============================

Copyright (C) 2019-2023 Intel Corporation

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
  IdxAddc_Add = 1,
  IdxAddc_Carry = 0,
  IdxSubb_Sub = 1,
  IdxSubb_Borrow = 0,
  IdxAdd3c_Add = 1,
  IdxAdd3c_Carry = 0
};
}

// The number of elements to load per address (vector size)
// NOTE: taken from cmc/support
enum class LSCVectorSize : uint8_t {
  N0 = 0,
  N1 = 1,  // 1 element
  N2 = 2,  // 2 element
  N3 = 3,  // 3 element
  N4 = 4,  // 4 element
  N8 = 5,  // 8 element
  N16 = 6, // 16 element
  N32 = 7, // 32 element
  N64 = 8  // 64 element
};

enum class LSCDataSize : uint8_t {
  Invalid,
  D8,
  D16,
  D32,
  D64,
  D8U32,
  D16U32,
  D16U32H,
};

enum class LSCDataOrder : uint8_t {
  Invalid,
  NonTranspose,
  Transpose
};

enum class LSCCategory : uint8_t {
  Load,
  Load2D,
  Prefetch,
  Prefetch2D,
  Store,
  Store2D,
  Load2DTyped,
  Store2DTyped,
  Fence,
  LegacyAtomic,
  Atomic,
  NotLSC
};

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

inline const char *getGenXIntrinsicPrefix() { return "llvm.genx."; }

ID getGenXIntrinsicID(const Function *F);

/// Utility function to get the genx_intrinsic ID if V is a GenXIntrinsic call.
/// V is allowed to be 0.
inline ID getGenXIntrinsicID(const Value *V) {
  if (V)
    if (const CallInst *CI = dyn_cast<CallInst>(V))
      if (Function *Callee = CI->getCalledFunction())
        return getGenXIntrinsicID(Callee);
  return GenXIntrinsic::not_genx_intrinsic;
}

/// GenXIntrinsic::isGenXIntrinsic(ID) - Is GenX intrinsic
/// NOTE that this is include not_genx_intrinsic
/// BUT DOES NOT include not_any_intrinsic
inline bool isGenXIntrinsic(unsigned ID) {
  return ID >= not_genx_intrinsic && ID < num_genx_intrinsics;
}

/// GenXIntrinsic::isGenXIntrinsic(CF) - Returns true if
/// the function's name starts with "llvm.genx.".
/// It's possible for this function to return true while getGenXIntrinsicID()
/// returns GenXIntrinsic::not_genx_intrinsic!
bool isGenXIntrinsic(const Function *CF);

/// GenXIntrinsic::isGenXIntrinsic(V) - Returns true if
/// the function's name starts with "llvm.genx.".
/// It's possible for this function to return true while getGenXIntrinsicID()
/// returns GenXIntrinsic::not_genx_intrinsic!
inline bool isGenXIntrinsic(const Value *V) {
  if (V)
    if (const CallInst *CI = dyn_cast<CallInst>(V))
      if (Function *Callee = CI->getCalledFunction())
        return isGenXIntrinsic(Callee);
  return false;
}

/// GenXIntrinsic::isGenXNonTrivialIntrinsic(ID) - Is GenX intrinsic,
/// which is not equal to not_genx_intrinsic or not_any_intrinsic
inline bool isGenXNonTrivialIntrinsic(unsigned ID) {
  return ID > not_genx_intrinsic && ID < num_genx_intrinsics;
}

/// GenXIntrinsic::isGenXNonTrivialIntrinsic(CF) - Returns true if
/// CF is genx intrinsic, not equal to not_any_intrinsic or not_genx_intrinsic
inline bool isGenXNonTrivialIntrinsic(const Function *CF) {
  return isGenXNonTrivialIntrinsic(getGenXIntrinsicID(CF));
}

/// GenXIntrinsic::isGenXNonTrivialIntrinsic(V) - Returns true if
/// V is genx intrinsic, not equal to not_any_intrinsic or not_genx_intrinsic
inline bool isGenXNonTrivialIntrinsic(const Value *V) {
  return isGenXNonTrivialIntrinsic(getGenXIntrinsicID(V));
}

/// GenXIntrinsic::getGenXName(ID) - Return the LLVM name for a GenX intrinsic,
/// such as "llvm.genx.lane.id".
std::string getGenXName(ID id, ArrayRef<Type *> Tys = {});

ID lookupGenXIntrinsicID(StringRef Name);

AttributeList getAttributes(LLVMContext &C, ID id);

/// GenXIntrinsic::getGenXType(ID) - Return the function type for an intrinsic.
FunctionType *getGenXType(LLVMContext &Context, GenXIntrinsic::ID id,
                          ArrayRef<Type *> Tys = {});

/// GenXIntrinsic::getGenXDeclaration(M, ID) - Create or insert a GenX LLVM
/// Function declaration for an intrinsic, and return it.
///
/// The Tys parameter is for intrinsics with overloaded types (e.g., those
/// using iAny, fAny, vAny, or iPTRAny).  For a declaration of an overloaded
/// intrinsic, Tys must provide exactly one type for each overloaded type in
/// the intrinsic.
Function *getGenXDeclaration(Module *M, ID id, ArrayRef<Type *> Tys = {});

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
inline unsigned getAnyIntrinsicID(const Function *F) {
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
inline unsigned getAnyIntrinsicID(const Value *V) {
  if (V)
    if (const CallInst *CI = dyn_cast<CallInst>(V))
      if (Function *Callee = CI->getCalledFunction())
        return getAnyIntrinsicID(Callee);
  return GenXIntrinsic::not_any_intrinsic;
}

/// GenXIntrinsic::isAnyIntrinsic(ID) - Is any intrinsic
/// including not_any_intrinsic
inline bool isAnyIntrinsic(unsigned id) {
  assert(id != not_genx_intrinsic && id != Intrinsic::not_intrinsic &&
         "Do not use this method with getGenXIntrinsicID or getIntrinsicID!");
  return id < num_genx_intrinsics || id == not_any_intrinsic;
}

/// GenXIntrinsic::isAnyNonTrivialIntrinsic(id) - Is GenX or LLVM intrinsic,
/// which is not equal to not_any_intrinsic
inline bool isAnyNonTrivialIntrinsic(unsigned id) {
  assert(id != not_genx_intrinsic && id != Intrinsic::not_intrinsic &&
         "Do not use this method with getGenXIntrinsicID or getIntrinsicID!");
  return id <  num_genx_intrinsics &&
         id != not_any_intrinsic;
}

/// GenXIntrinsic::isAnyNonTrivialIntrinsic(ID) - Is GenX or LLVM intrinsic,
/// which is not equal to not_genx_intrinsic, not_any_intrinsic or not_intrinsic
inline bool isAnyNonTrivialIntrinsic(const Function *CF) {
  return isAnyNonTrivialIntrinsic(getAnyIntrinsicID(CF));
}

/// Utility function to check if V is LLVM or GenX intrinsic call,
/// which is not not_intrinsic, not_genx_intrinsic or not_any_intrinsic
/// V is allowed to be 0.
inline bool isAnyNonTrivialIntrinsic(const Value *V) {
  return isAnyNonTrivialIntrinsic(getAnyIntrinsicID(V));
}

/// GenXIntrinsic::getAnyName(ID) - Return the LLVM name for LLVM or GenX
/// intrinsic, such as "llvm.genx.lane.id".
std::string getAnyName(unsigned id, ArrayRef<Type *> Tys = {});

/// GenXIntrinsic::getAnyType(ID) - Return the function type for an intrinsic.
inline FunctionType *getAnyType(LLVMContext &Context, unsigned id,
                                ArrayRef<Type *> Tys = {}) {
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
Function *getAnyDeclaration(Module *M, unsigned id,
                                   ArrayRef<Type *> Tys = {});

/// GenXIntrinsic::getGenXMulIID(S1, S2) - returns GenXIntrinsic::ID for
/// the enx_XXmul opertation, where XX is is defined by the input arguments
/// which represent signs of the operands
inline GenXIntrinsic::ID getGenXMulIID(bool LHSign, bool RHSign) {
  return LHSign
             ? (RHSign ? GenXIntrinsic::genx_ssmul : GenXIntrinsic::genx_sumul)
             : (RHSign ? GenXIntrinsic::genx_usmul : GenXIntrinsic::genx_uumul);
}

inline bool isRdRegion(unsigned IntrinID) {
  switch (IntrinID) {
  case GenXIntrinsic::genx_rdregioni:
  case GenXIntrinsic::genx_rdregionf:
    return true;
  default:
    return false;
  }
}

inline bool isRdRegion(const Function *F) {
  return isRdRegion(getGenXIntrinsicID(F));
}

inline bool isRdRegion(const Value *V) {
  return isRdRegion(getGenXIntrinsicID(V));
}

inline bool isWrRegion(unsigned IntrinID) {
  switch (IntrinID) {
  case GenXIntrinsic::genx_wrregioni:
  case GenXIntrinsic::genx_wrregionf:
  case GenXIntrinsic::genx_wrconstregion:
    return true;
  default:
    return false;
  }
}

inline bool isWrRegion(const Function *F) {
  return isWrRegion(getGenXIntrinsicID(F));
}

inline bool isWrRegion(const Value *V) {
  return isWrRegion(getGenXIntrinsicID(V));
}

inline bool isAbs(unsigned IntrinID) {
    if (IntrinID == GenXIntrinsic::genx_absf || IntrinID == GenXIntrinsic::genx_absi)
        return true;
    return false;
}

inline bool isAbs(const Function *F) {
    return isAbs(getGenXIntrinsicID(F));
}

inline bool isAbs(const Value *V) {
    return isAbs(getGenXIntrinsicID(V));
}

inline bool isIntegerSat(unsigned IID) {
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

inline bool isIntegerSat(const Function *F) {
  return isIntegerSat(getGenXIntrinsicID(F));
}

inline bool isIntegerSat(const Value *V) {
  return isIntegerSat(getGenXIntrinsicID(V));
}

inline bool isVLoad(unsigned IntrinID) {
  return IntrinID == GenXIntrinsic::genx_vload;
}

inline bool isVLoad(const Function *F) {
  return isVLoad(getGenXIntrinsicID(F));
}

inline bool isVLoad(const Value *V) {
  return isVLoad(getGenXIntrinsicID(V));
}

inline bool isVStore(unsigned IntrinID) {
  return IntrinID == GenXIntrinsic::genx_vstore;
}

inline bool isVStore(const Function *F) {
  return isVStore(getGenXIntrinsicID(F));
}

inline bool isVStore(const Value *V) {
  return isVStore(getGenXIntrinsicID(V));
}

inline bool isVLoadStore(unsigned IntrinID) {
  return isVLoad(IntrinID) || isVStore(IntrinID);
}

inline bool isVLoadStore(const Function *F) {
  return isVLoadStore(getGenXIntrinsicID(F));
}

inline bool isVLoadStore(const Value *V) {
  return isVLoadStore(getGenXIntrinsicID(V));
}

inline bool isReadPredefReg(unsigned IntrinID) {
  return IntrinID == GenXIntrinsic::genx_read_predef_reg;
}

inline bool isReadPredefReg(const Function *F) {
  return isReadPredefReg(getGenXIntrinsicID(F));
}

inline bool isReadPredefReg(const Value *V) {
  return isReadPredefReg(getGenXIntrinsicID(V));
}

inline bool isWritePredefReg(unsigned IntrinID) {
  return IntrinID == GenXIntrinsic::genx_write_predef_reg;
}

inline bool isWritePredefReg(const Function *F) {
  return isWritePredefReg(getGenXIntrinsicID(F));
}

inline bool isWritePredefReg(const Value *V) {
  return isWritePredefReg(getGenXIntrinsicID(V));
}

inline bool isReadWritePredefReg(unsigned IntrinID) {
  return isWritePredefReg(IntrinID) || isReadPredefReg(IntrinID);
}

inline bool isReadWritePredefReg(const Value *V) {
  return isWritePredefReg(getGenXIntrinsicID(V)) ||
         isReadPredefReg(getGenXIntrinsicID(V));
}

inline bool isReadWritePredefReg(const Function *F) {
  return isWritePredefReg(getGenXIntrinsicID(F)) ||
         isReadPredefReg(getGenXIntrinsicID(F));
}

inline LSCCategory getLSCCategory(unsigned IntrinID) {
  switch(IntrinID) {
    case GenXIntrinsic::genx_lsc_load_bti:
    case GenXIntrinsic::genx_lsc_load_stateless:
    case GenXIntrinsic::genx_lsc_load_slm:
    case GenXIntrinsic::genx_lsc_load_bindless:
    case GenXIntrinsic::genx_lsc_load_quad_bti:
    case GenXIntrinsic::genx_lsc_load_quad_slm:
    case GenXIntrinsic::genx_lsc_load_quad_stateless:
    case GenXIntrinsic::genx_lsc_load_merge_bti:
    case GenXIntrinsic::genx_lsc_load_merge_stateless:
    case GenXIntrinsic::genx_lsc_load_merge_slm:
    case GenXIntrinsic::genx_lsc_load_merge_bindless:
    case GenXIntrinsic::genx_lsc_load_merge_quad_bti:
    case GenXIntrinsic::genx_lsc_load_merge_quad_slm:
    case GenXIntrinsic::genx_lsc_load_merge_quad_stateless:
      return LSCCategory::Load;
    case GenXIntrinsic::genx_lsc_load2d_stateless:
      return LSCCategory::Load2D;
    case GenXIntrinsic::genx_lsc_load2d_typed_bti:
      return LSCCategory::Load2DTyped;
    case GenXIntrinsic::genx_lsc_prefetch_bti:
    case GenXIntrinsic::genx_lsc_prefetch_stateless:
      return LSCCategory::Prefetch;
    case GenXIntrinsic::genx_lsc_prefetch2d_stateless:
      return LSCCategory::Prefetch2D;
    case GenXIntrinsic::genx_lsc_store_bti:
    case GenXIntrinsic::genx_lsc_store_stateless:
    case GenXIntrinsic::genx_lsc_store_slm:
    case GenXIntrinsic::genx_lsc_store_bindless:
    case GenXIntrinsic::genx_lsc_store_quad_bti:
    case GenXIntrinsic::genx_lsc_store_quad_slm:
    case GenXIntrinsic::genx_lsc_store_quad_stateless:
      return LSCCategory::Store;
    case GenXIntrinsic::genx_lsc_store2d_stateless:
      return LSCCategory::Store2D;
    case GenXIntrinsic::genx_lsc_store2d_typed_bti:
      return LSCCategory::Store2DTyped;
    case GenXIntrinsic::genx_lsc_fence:
      return LSCCategory::Fence;
    case GenXIntrinsic::genx_lsc_atomic_bti:
    case GenXIntrinsic::genx_lsc_atomic_stateless:
    case GenXIntrinsic::genx_lsc_atomic_slm:
    case GenXIntrinsic::genx_lsc_atomic_bindless:
      return LSCCategory::LegacyAtomic;
    case GenXIntrinsic::genx_lsc_xatomic_bti:
    case GenXIntrinsic::genx_lsc_xatomic_stateless:
    case GenXIntrinsic::genx_lsc_xatomic_slm:
    case GenXIntrinsic::genx_lsc_xatomic_bindless:
      return LSCCategory::Atomic;
    default:
      return LSCCategory::NotLSC;
  }
}

inline LSCCategory getLSCCategory(const Value *V) {
  return getLSCCategory(getGenXIntrinsicID(V));
}

inline LSCCategory getLSCCategory(const Function *F) {
  return getLSCCategory(getGenXIntrinsicID(F));
}

inline bool isLSCLoad(unsigned IntrinID) {
  return getLSCCategory(IntrinID) == LSCCategory::Load;
}

inline bool isLSCLoad(const Value *V) {
  return isLSCLoad(getGenXIntrinsicID(V));
}

inline bool isLSCLoad(const Function *F) {
  return isLSCLoad(getGenXIntrinsicID(F));
}

inline bool isLSCLoad2D(unsigned IntrinID) {
  return getLSCCategory(IntrinID) == LSCCategory::Load2D;
}

inline bool isLSCLoad2D(const Value *V) {
  return isLSCLoad2D(getGenXIntrinsicID(V));
}

inline bool isLSCLoad2D(const Function *F) {
  return isLSCLoad2D(getGenXIntrinsicID(F));
}

inline bool isLSCLoad2DTyped(unsigned IntrinID) {
  return getLSCCategory(IntrinID) == LSCCategory::Load2DTyped;
}

inline bool isLSCLoad2DTyped(const Value *V) {
  return isLSCLoad2DTyped(getGenXIntrinsicID(V));
}

inline bool isLSCLoad2DTyped(const Function *F) {
  return isLSCLoad2DTyped(getGenXIntrinsicID(F));
}

inline bool isLSCPrefetch(unsigned IntrinID) {
  return getLSCCategory(IntrinID) == LSCCategory::Prefetch;
}

inline bool isLSCPrefetch(const Value *V) {
  return isLSCPrefetch(getGenXIntrinsicID(V));
}

inline bool isLSCPrefetch(const Function *F) {
  return isLSCPrefetch(getGenXIntrinsicID(F));
}

inline bool isLSCPrefetch2D(unsigned IntrinID) {
  return getLSCCategory(IntrinID) == LSCCategory::Prefetch2D;
}

inline bool isLSCPrefetch2D(const Value *V) {
  return isLSCPrefetch2D(getGenXIntrinsicID(V));
}

inline bool isLSCPrefetch2D(const Function *F) {
  return isLSCPrefetch2D(getGenXIntrinsicID(F));
}

inline bool isLSCStore(unsigned IntrinID) {
  return getLSCCategory(IntrinID) == LSCCategory::Store;
}

inline bool isLSCStore(const Value *V) {
  return isLSCStore(getGenXIntrinsicID(V));
}

inline bool isLSCStore(const Function *F) {
  return isLSCStore(getGenXIntrinsicID(F));
}

inline bool isLSCStore2D(unsigned IntrinID) {
  return getLSCCategory(IntrinID) == LSCCategory::Store2D;
}

inline bool isLSCStore2D(const Value *V) {
  return isLSCStore2D(getGenXIntrinsicID(V));
}

inline bool isLSCStore2D(const Function *F) {
  return isLSCStore2D(getGenXIntrinsicID(F));
}

inline bool isLSCStore2DTyped(unsigned IntrinID) {
  return getLSCCategory(IntrinID) == LSCCategory::Store2DTyped;
}

inline bool isLSCStore2DTyped(const Value *V) {
  return isLSCStore2DTyped(getGenXIntrinsicID(V));
}

inline bool isLSCStore2DTyped(const Function *F) {
  return isLSCStore2DTyped(getGenXIntrinsicID(F));
}

inline bool isLSCFence(unsigned IntrinID) {
  return getLSCCategory(IntrinID) == LSCCategory::Fence;
}

inline bool isLSCFence(const Value *V) {
  return isLSCFence(getGenXIntrinsicID(V));
}

inline bool isLSCFence(const Function *F) {
  return isLSCFence(getGenXIntrinsicID(F));
}

inline bool isLSCLegacyAtomic(unsigned IntrinID) {
  return getLSCCategory(IntrinID) == LSCCategory::LegacyAtomic;
}

inline bool isLSCLegacyAtomic(const Value *V) {
  return isLSCLegacyAtomic(getGenXIntrinsicID(V));
}

inline bool isLSCLegacyAtomic(const Function *F) {
  return isLSCLegacyAtomic(getGenXIntrinsicID(F));
}

inline bool isLSCAtomic(unsigned IntrinID) {
  return getLSCCategory(IntrinID) == LSCCategory::Atomic;
}

inline bool isLSCAtomic(const Value *V) {
  return isLSCAtomic(getGenXIntrinsicID(V));
}

inline bool isLSCAtomic(const Function *F) {
  return isLSCAtomic(getGenXIntrinsicID(F));
}

inline bool isLSC(unsigned IntrinID) {
  return getLSCCategory(IntrinID) != LSCCategory::NotLSC;
}

inline bool isLSC(const Value *V) {
  return isLSC(getGenXIntrinsicID(V));
}

inline bool isLSC(const Function *F) {
  return isLSC(getGenXIntrinsicID(F));
}

inline bool isLSC2D(unsigned IntrinID) {
  switch (getLSCCategory(IntrinID)) {
    case LSCCategory::Load2D:
    case LSCCategory::Prefetch2D:
    case LSCCategory::Store2D:
    case LSCCategory::Load2DTyped:
    case LSCCategory::Store2DTyped:
      return true;
    case LSCCategory::Load:
    case LSCCategory::Prefetch:
    case LSCCategory::Store:
    case LSCCategory::Fence:
    case LSCCategory::LegacyAtomic:
    case LSCCategory::Atomic:
    case LSCCategory::NotLSC:
      return false;
  }
  llvm_unreachable("Unknown LSC category");
}

inline bool isLSC2D(const Value *V) {
  return isLSC2D(getGenXIntrinsicID(V));
}

inline bool isLSC2D(const Function *F) {
  return isLSC2D(getGenXIntrinsicID(F));
}

inline bool isLSCTyped(unsigned IntrinID) {
  switch (getLSCCategory(IntrinID)) {
    case LSCCategory::Load2DTyped:
    case LSCCategory::Store2DTyped:
      return true;
    case LSCCategory::Store2D:
    case LSCCategory::Load:
    case LSCCategory::Load2D:
    case LSCCategory::Prefetch:
    case LSCCategory::Prefetch2D:
    case LSCCategory::Store:
    case LSCCategory::Fence:
    case LSCCategory::LegacyAtomic:
    case LSCCategory::Atomic:
    case LSCCategory::NotLSC:
      return false;
  }
  llvm_unreachable("Unknown LSC category");
}

inline bool isLSCTyped(const Value *V) {
  return isLSCTyped(getGenXIntrinsicID(V));
}

inline bool isLSCTyped(const Function *F) {
  return isLSCTyped(getGenXIntrinsicID(F));
}

// Dependency from visa_igc_common_header.
// Converts vector size into LSC-appropriate code.
inline LSCVectorSize getLSCVectorSize(unsigned N) {
  switch (N) {
  case 0:
    return LSCVectorSize::N0;
  case 1:
    return LSCVectorSize::N1;
  case 2:
    return LSCVectorSize::N2;
  case 3:
    return LSCVectorSize::N3;
  case 4:
    return LSCVectorSize::N4;
  case 8:
    return LSCVectorSize::N8;
  case 16:
    return LSCVectorSize::N16;
  case 32:
    return LSCVectorSize::N32;
  case 64:
    return LSCVectorSize::N64;
  }
  llvm_unreachable("Unknown vector size");
}
// Gets encoded vector size for LSC instruction.
inline uint8_t getEncodedLSCVectorSize(unsigned N) {
  return static_cast<uint8_t>(getLSCVectorSize(N));
}

// Functions in this namespace return argument index for LSC instruction.
namespace LSCArgIdx {
constexpr int Invalid = -1;
// Returns VectorSize index.
inline int getLSCVectorSize(LSCCategory Cat) {
  switch (Cat) {
  case LSCCategory::Load:
  case LSCCategory::Prefetch:
  case LSCCategory::Store:
  case LSCCategory::Atomic:
    return 7;
  case LSCCategory::LegacyAtomic:
    return 8;
  case LSCCategory::Prefetch2D:
  case LSCCategory::Load2D:
  case LSCCategory::Store2D:
  case LSCCategory::Load2DTyped:
  case LSCCategory::Store2DTyped:
  case LSCCategory::Fence:
  case LSCCategory::NotLSC:
    llvm_unreachable("no such argument");
    return Invalid;
  }
  return Invalid;
}
// Returns VectorSize index.
inline int getLSCVectorSize(unsigned IID) {
  return LSCArgIdx::getLSCVectorSize(getLSCCategory(IID));
}

// Returns DataSize index.
inline int getLSCDataSize(LSCCategory Cat) {
  switch (Cat) {
  case LSCCategory::Load:
  case LSCCategory::Prefetch:
  case LSCCategory::Store:
  case LSCCategory::LegacyAtomic:
  case LSCCategory::Atomic:
    return 6;
  case LSCCategory::Load2D:
  case LSCCategory::Prefetch2D:
  case LSCCategory::Store2D:
    return 3;
  case LSCCategory::Fence:
  case LSCCategory::Load2DTyped:
  case LSCCategory::Store2DTyped:
  case LSCCategory::NotLSC:
    llvm_unreachable("no such argument");
    return Invalid;
  }
  return Invalid;
}
// Returns DataSize index.
inline int getLSCDataSize(unsigned IID) {
  return LSCArgIdx::getLSCDataSize(getLSCCategory(IID));
}

// Returns immediate offset index.
inline int getLSCImmOffset(LSCCategory Cat) {
  switch (Cat) {
  case LSCCategory::Load:
  case LSCCategory::Prefetch:
  case LSCCategory::Store:
  case LSCCategory::LegacyAtomic:
  case LSCCategory::Atomic:
    return 5;
  case LSCCategory::Prefetch2D:
  case LSCCategory::Load2D:
  case LSCCategory::Store2D:
  case LSCCategory::Load2DTyped:
  case LSCCategory::Store2DTyped:
  case LSCCategory::Fence:
  case LSCCategory::NotLSC:
    llvm_unreachable("no such argument");
    return Invalid;
  }
  return Invalid;
}
// Returns immediate offset index.
inline int getLSCImmOffset(unsigned IID) {
  return LSCArgIdx::getLSCImmOffset(getLSCCategory(IID));
}

// Returns data order index.
inline int getLSCDataOrder(LSCCategory Cat) {
  switch (Cat) {
  case LSCCategory::Load:
  case LSCCategory::Prefetch:
  case LSCCategory::Store:
  case LSCCategory::Atomic:
    return 8;
  case LSCCategory::LegacyAtomic:
    return 7;
  case LSCCategory::Load2D:
  case LSCCategory::Prefetch2D:
  case LSCCategory::Store2D:
    return 4;
  case LSCCategory::Fence:
  case LSCCategory::Load2DTyped:
  case LSCCategory::Store2DTyped:
  case LSCCategory::NotLSC:
    llvm_unreachable("no such argument");
    return Invalid;
  }
  return Invalid;
}
// Returns data order index.
inline int getLSCDataOrder(unsigned IID) {
  return LSCArgIdx::getLSCDataOrder(getLSCCategory(IID));
}

// Returns width index.
inline int getLSCWidth(LSCCategory Cat) {
  switch (Cat) {
  case LSCCategory::Load:
  case LSCCategory::Prefetch:
  case LSCCategory::Store:
  case LSCCategory::Fence:
  case LSCCategory::LegacyAtomic:
  case LSCCategory::Atomic:
  case LSCCategory::Load2D:
  case LSCCategory::Prefetch2D:
  case LSCCategory::Store2D:
  case LSCCategory::Load2DTyped:
  case LSCCategory::Store2DTyped:
    return 0;
  case LSCCategory::NotLSC:
    llvm_unreachable("no such argument");
    return Invalid;
  }
  return Invalid;
}
// Returns width index.
inline int getLSCWidth(unsigned IID) {
  return LSCArgIdx::getLSCWidth(getLSCCategory(IID));
}

} // namespace LSCArgIdx

inline unsigned getLSCNumVectorElements(LSCVectorSize VS) {
  switch (VS) {
    case LSCVectorSize::N0:
      break;
    case LSCVectorSize::N1:
      return 1;
    case LSCVectorSize::N2:
      return 2;
    case LSCVectorSize::N3:
      return 3;
    case LSCVectorSize::N4:
      return 4;
    case LSCVectorSize::N8:
      return 8;
    case LSCVectorSize::N16:
      return 16;
    case LSCVectorSize::N32:
      return 32;
    case LSCVectorSize::N64:
      return 64;
  }
  llvm_unreachable("Unknown vector size");
}

LSCVectorSize getLSCVectorSize(const Instruction *I);

inline unsigned getLSCNumVectorElements(const Instruction *I) {
  return GenXIntrinsic::getLSCNumVectorElements(getLSCVectorSize(I));
}

inline unsigned getLSCDataBitsRegister(LSCDataSize DS) {
  switch(DS) {
    case LSCDataSize::Invalid:
      break;
    case LSCDataSize::D8:
      return 8;
    case LSCDataSize::D16:
      return 16;
    case LSCDataSize::D32:
    case LSCDataSize::D8U32:
    case LSCDataSize::D16U32:
    case LSCDataSize::D16U32H:
      return 32;
    case LSCDataSize::D64:
      return 64;
  }
  llvm_unreachable("Unknown data size");
}

inline unsigned getLSCDataBitsMemory(LSCDataSize DS) {
  switch(DS) {
    case LSCDataSize::Invalid:
      break;
    case LSCDataSize::D8:
    case LSCDataSize::D8U32:
      return 8;
    case LSCDataSize::D16:
    case LSCDataSize::D16U32:
    case LSCDataSize::D16U32H:
      return 16;
    case LSCDataSize::D32:
      return 32;
    case LSCDataSize::D64:
      return 64;
  }
  llvm_unreachable("Unknown data size");
}

LSCDataSize getLSCDataSize(const Instruction *I);

inline unsigned getLSCDataBitsRegister(const Instruction *I) {
  return getLSCDataBitsRegister(getLSCDataSize(I));
}

inline unsigned getLSCDataBitsMemory(const Instruction *I) {
  return getLSCDataBitsMemory(getLSCDataSize(I));
}

LSCDataOrder getLSCDataOrder(const Instruction *I);

inline bool isLSCNonTransposed(const Instruction *I) {
  return getLSCDataOrder(I) == LSCDataOrder::NonTranspose;
}

inline bool isLSCTransposed(const Instruction *I) {
  return getLSCDataOrder(I) == LSCDataOrder::Transpose;
}

unsigned getLSCWidth(const Instruction *I);

} // namespace GenXIntrinsic

// todo: delete this
namespace GenXIntrinsic {
AttributeList getAttributes(LLVMContext &C, ID id);

} // namespace GenXIntrinsic

} // namespace llvm

#endif

/*========================== begin_copyright_notice ============================

Copyright (C) 2019-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

// Originated from llvm source lib/IR/Function.cpp
// Function.cpp - Implement the Global object classes

// Implementation of methods declared in llvm/GenXIntrinsics/GenXIntrinsics.h

#include "llvm/GenXIntrinsics/GenXIntrinsics.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/CodeGen/ValueTypes.h>

#include "llvmVCWrapper/IR/DerivedTypes.h"
#include "llvmVCWrapper/IR/Intrinsics.h"
#include "llvmVCWrapper/IR/Type.h"

#include <cstring>
#include <map>

using namespace llvm;

static cl::opt<bool> EnableGenXIntrinsicsCache(
    "enable-genx-intrinsics-cache", cl::init(true), cl::Hidden,
    cl::desc("Enable metadata caching of genx intrinsics"));

#define MANGLE(STR) (STR)

/// Intrinsic::isOverloaded(ID) - Returns true if the intrinsic can be
/// overloaded.
static bool isOverloaded(GenXIntrinsic::ID id);

/// getIntrinsicInfoTableEntries - Return the IIT table descriptor for the
/// specified intrinsic into an array of IITDescriptors.
///
void
getIntrinsicInfoTableEntries(GenXIntrinsic::ID id,
                             SmallVectorImpl<Intrinsic::IITDescriptor> &T);

/// IIT_Info - These are enumerators that describe the entries returned by the
/// getIntrinsicInfoTableEntries function.
///
/// NOTE: This must be kept in synch with the copy in TblGen/IntrinsicEmitter!
enum IIT_Info {
  // Common values should be encoded with 0-15.
  IIT_Done = 0,
  IIT_I1   = 1,
  IIT_I8   = 2,
  IIT_I16  = 3,
  IIT_I32  = 4,
  IIT_I64  = 5,
  IIT_F16  = 6,
  IIT_F32  = 7,
  IIT_F64  = 8,
  IIT_V2   = 9,
  IIT_V4   = 10,
  IIT_V8   = 11,
  IIT_V16  = 12,
  IIT_V32  = 13,
  IIT_PTR  = 14,
  IIT_ARG  = 15,

  // Values from 16+ are only encodable with the inefficient encoding.
  IIT_V64  = 16,
  IIT_MMX  = 17,
  IIT_TOKEN = 18,
  IIT_METADATA = 19,
  IIT_EMPTYSTRUCT = 20,
  IIT_STRUCT2 = 21,
  IIT_STRUCT3 = 22,
  IIT_STRUCT4 = 23,
  IIT_STRUCT5 = 24,
  IIT_EXTEND_ARG = 25,
  IIT_TRUNC_ARG = 26,
  IIT_ANYPTR = 27,
  IIT_V1   = 28,
  IIT_VARARG = 29,
  IIT_HALF_VEC_ARG = 30,
  IIT_SAME_VEC_WIDTH_ARG = 31,
#if VC_INTR_LLVM_VERSION_MAJOR < 18
  IIT_PTR_TO_ARG = 32,
  IIT_PTR_TO_ELT = 33,
#endif
  IIT_VEC_OF_ANYPTRS_TO_ELT = 34,
  IIT_I128 = 35,
  IIT_V512 = 36,
  IIT_V1024 = 37,
  IIT_STRUCT6 = 38,
  IIT_STRUCT7 = 39,
  IIT_STRUCT8 = 40,
  IIT_F128 = 41
};

static void
DecodeIITType(unsigned &NextElt, ArrayRef<unsigned char> Infos,
              SmallVectorImpl<Intrinsic::IITDescriptor> &OutputTable) {
  using namespace Intrinsic;

  IIT_Info Info = IIT_Info(Infos[NextElt++]);
  unsigned StructElts = 2;

  switch (Info) {
  case IIT_Done:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Void, 0));
    return;
  case IIT_VARARG:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::VarArg, 0));
    return;
  case IIT_MMX:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::MMX, 0));
    return;
  case IIT_TOKEN:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Token, 0));
    return;
  case IIT_METADATA:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Metadata, 0));
    return;
  case IIT_F16:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Half, 0));
    return;
  case IIT_F32:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Float, 0));
    return;
  case IIT_F64:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Double, 0));
    return;
  case IIT_F128:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Quad, 0));
    return;
  case IIT_I1:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Integer, 1));
    return;
  case IIT_I8:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Integer, 8));
    return;
  case IIT_I16:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Integer,16));
    return;
  case IIT_I32:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Integer, 32));
    return;
  case IIT_I64:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Integer, 64));
    return;
  case IIT_I128:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Integer, 128));
    return;
  case IIT_V1:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Vector, 1));
    DecodeIITType(NextElt, Infos, OutputTable);
    return;
  case IIT_V2:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Vector, 2));
    DecodeIITType(NextElt, Infos, OutputTable);
    return;
  case IIT_V4:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Vector, 4));
    DecodeIITType(NextElt, Infos, OutputTable);
    return;
  case IIT_V8:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Vector, 8));
    DecodeIITType(NextElt, Infos, OutputTable);
    return;
  case IIT_V16:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Vector, 16));
    DecodeIITType(NextElt, Infos, OutputTable);
    return;
  case IIT_V32:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Vector, 32));
    DecodeIITType(NextElt, Infos, OutputTable);
    return;
  case IIT_V64:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Vector, 64));
    DecodeIITType(NextElt, Infos, OutputTable);
    return;
  case IIT_V512:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Vector, 512));
    DecodeIITType(NextElt, Infos, OutputTable);
    return;
  case IIT_V1024:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Vector, 1024));
    DecodeIITType(NextElt, Infos, OutputTable);
    return;
  case IIT_PTR:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Pointer, 0));
    DecodeIITType(NextElt, Infos, OutputTable);
    return;
  case IIT_ANYPTR: {  // [ANYPTR addrspace, subtype]
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Pointer,
                                             Infos[NextElt++]));
    DecodeIITType(NextElt, Infos, OutputTable);
    return;
  }
  case IIT_ARG: {
    unsigned ArgInfo = (NextElt == Infos.size() ? 0 : Infos[NextElt++]);
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Argument, ArgInfo));
    return;
  }
  case IIT_EXTEND_ARG: {
    unsigned ArgInfo = (NextElt == Infos.size() ? 0 : Infos[NextElt++]);
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::ExtendArgument,
                                             ArgInfo));
    return;
  }
  case IIT_TRUNC_ARG: {
    unsigned ArgInfo = (NextElt == Infos.size() ? 0 : Infos[NextElt++]);
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::TruncArgument,
                                             ArgInfo));
    return;
  }
  case IIT_HALF_VEC_ARG: {
    unsigned ArgInfo = (NextElt == Infos.size() ? 0 : Infos[NextElt++]);
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::HalfVecArgument,
                                             ArgInfo));
    return;
  }
  case IIT_SAME_VEC_WIDTH_ARG: {
    unsigned ArgInfo = (NextElt == Infos.size() ? 0 : Infos[NextElt++]);
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::SameVecWidthArgument,
                                             ArgInfo));
    return;
  }
#if VC_INTR_LLVM_VERSION_MAJOR < 18
  case IIT_PTR_TO_ARG: {
    unsigned ArgInfo = (NextElt == Infos.size() ? 0 : Infos[NextElt++]);
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::PtrToArgument,
                                             ArgInfo));
    return;
  }
  case IIT_PTR_TO_ELT: {
    unsigned ArgInfo = (NextElt == Infos.size() ? 0 : Infos[NextElt++]);
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::PtrToElt, ArgInfo));
    return;
  }
#endif
  case IIT_VEC_OF_ANYPTRS_TO_ELT: {
    unsigned short ArgNo = (NextElt == Infos.size() ? 0 : Infos[NextElt++]);
    unsigned short RefNo = (NextElt == Infos.size() ? 0 : Infos[NextElt++]);
    OutputTable.push_back(
        IITDescriptor::get(IITDescriptor::VecOfAnyPtrsToElt, ArgNo, RefNo));
    return;
  }
  case IIT_EMPTYSTRUCT:
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Struct, 0));
    return;
  case IIT_STRUCT8: ++StructElts; LLVM_FALLTHROUGH;
  case IIT_STRUCT7: ++StructElts; LLVM_FALLTHROUGH;
  case IIT_STRUCT6: ++StructElts; LLVM_FALLTHROUGH;
  case IIT_STRUCT5: ++StructElts; LLVM_FALLTHROUGH;
  case IIT_STRUCT4: ++StructElts; LLVM_FALLTHROUGH;
  case IIT_STRUCT3: ++StructElts; LLVM_FALLTHROUGH;
  case IIT_STRUCT2: {
    OutputTable.push_back(IITDescriptor::get(IITDescriptor::Struct,StructElts));

    for (unsigned i = 0; i != StructElts; ++i)
      DecodeIITType(NextElt, Infos, OutputTable);
    return;
  }
  }
  llvm_unreachable("unhandled");
}

static Type *DecodeFixedType(ArrayRef<Intrinsic::IITDescriptor> &Infos,
                             ArrayRef<Type*> Tys, LLVMContext &Context) {
  using namespace Intrinsic;

  IITDescriptor D = Infos.front();
  Infos = Infos.slice(1);

  switch (D.Kind) {
  case IITDescriptor::Void: return Type::getVoidTy(Context);
  case IITDescriptor::VarArg: return Type::getVoidTy(Context);
  case IITDescriptor::MMX: return Type::getX86_MMXTy(Context);
  case IITDescriptor::Token: return Type::getTokenTy(Context);
  case IITDescriptor::Metadata: return Type::getMetadataTy(Context);
  case IITDescriptor::Half: return Type::getHalfTy(Context);
  case IITDescriptor::Float: return Type::getFloatTy(Context);
  case IITDescriptor::Double: return Type::getDoubleTy(Context);
  case IITDescriptor::Quad: return Type::getFP128Ty(Context);

  case IITDescriptor::Integer:
    return IntegerType::get(Context, D.Integer_Width);
  case IITDescriptor::Vector:
    return VCINTR::getVectorType(DecodeFixedType(Infos, Tys, Context),D.Vector_Width);
  case IITDescriptor::Pointer:
    return PointerType::get(DecodeFixedType(Infos, Tys, Context),
                            D.Pointer_AddressSpace);
  case IITDescriptor::Struct: {
    SmallVector<Type *, 8> Elts;
    for (unsigned i = 0, e = D.Struct_NumElements; i != e; ++i)
      Elts.push_back(DecodeFixedType(Infos, Tys, Context));
    return StructType::get(Context, Elts);
  }
  case IITDescriptor::Argument:
    return Tys[D.getArgumentNumber()];
  case IITDescriptor::ExtendArgument: {
    Type *Ty = Tys[D.getArgumentNumber()];
    if (VectorType *VTy = dyn_cast<VectorType>(Ty))
      return VectorType::getExtendedElementVectorType(VTy);

    return IntegerType::get(Context, 2 * cast<IntegerType>(Ty)->getBitWidth());
  }
  case IITDescriptor::TruncArgument: {
    Type *Ty = Tys[D.getArgumentNumber()];
    if (VectorType *VTy = dyn_cast<VectorType>(Ty))
      return VectorType::getTruncatedElementVectorType(VTy);

    IntegerType *ITy = cast<IntegerType>(Ty);
    assert(ITy->getBitWidth() % 2 == 0);
    return IntegerType::get(Context, ITy->getBitWidth() / 2);
  }
  case IITDescriptor::HalfVecArgument:
    return VectorType::getHalfElementsVectorType(cast<VectorType>(
                                                  Tys[D.getArgumentNumber()]));
  case IITDescriptor::SameVecWidthArgument: {
    Type *EltTy = DecodeFixedType(Infos, Tys, Context);
    Type *Ty = Tys[D.getArgumentNumber()];
    if (VectorType *VTy = dyn_cast<VectorType>(Ty)) {
      return VCINTR::getVectorType(EltTy,
                                   VCINTR::VectorType::getNumElements(VTy));
    }
    llvm_unreachable("unhandled");
  }
#if VC_INTR_LLVM_VERSION_MAJOR < 18
  case IITDescriptor::PtrToArgument: {
    Type *Ty = Tys[D.getArgumentNumber()];
    return PointerType::getUnqual(Ty);
  }
  case IITDescriptor::PtrToElt: {
    Type *Ty = Tys[D.getArgumentNumber()];
    VectorType *VTy = dyn_cast<VectorType>(Ty);
    if (!VTy)
      llvm_unreachable("Expected an argument of Vector Type");
    Type *EltTy = cast<VectorType>(VTy)->getElementType();
    return PointerType::getUnqual(EltTy);
  }
#endif
  case IITDescriptor::VecOfAnyPtrsToElt:
    // Return the overloaded type (which determines the pointers address space)
    return Tys[D.getOverloadArgNumber()];
  default:
    break;
  }
  llvm_unreachable("unhandled");
}

#define GET_INTRINSIC_GENERATOR_GLOBAL
#include "llvm/GenXIntrinsics/GenXIntrinsicDescription.gen"
#undef GET_INTRINSIC_GENERATOR_GLOBAL

void GenXIntrinsic::getIntrinsicInfoTableEntries(
    GenXIntrinsic::ID id, SmallVectorImpl<Intrinsic::IITDescriptor> &T) {
  assert(id > GenXIntrinsic::not_genx_intrinsic);
  id = static_cast<GenXIntrinsic::ID>(id - GenXIntrinsic::not_genx_intrinsic);
  assert(id < sizeof(IIT_Table) / sizeof(*IIT_Table));

  // Check to see if the intrinsic's type was expressible by the table.
  unsigned TableVal = IIT_Table[id - 1];

  // Decode the TableVal into an array of IITValues.
  SmallVector<unsigned char, 8> IITValues;
  ArrayRef<unsigned char> IITEntries;
  unsigned NextElt = 0;
  if ((TableVal >> 31) != 0) {
    // This is an offset into the IIT_LongEncodingTable.
    IITEntries = IIT_LongEncodingTable;

    // Strip sentinel bit.
    NextElt = (TableVal << 1) >> 1;
  } else {
    // Decode the TableVal into an array of IITValues.  If the entry was encoded
    // into a single word in the table itself, decode it now.
    do {
      IITValues.push_back(TableVal & 0xF);
      TableVal >>= 4;
    } while (TableVal);

    IITEntries = IITValues;
    NextElt = 0;
  }

  // Okay, decode the table into the output vector of IITDescriptors.
  DecodeIITType(NextElt, IITEntries, T);
  while (NextElt != IITEntries.size() && IITEntries[NextElt] != 0)
    DecodeIITType(NextElt, IITEntries, T);
}

/// Returns a stable mangling for the type specified for use in the name
/// mangling scheme used by 'any' types in intrinsic signatures.  The mangling
/// of named types is simply their name.  Manglings for unnamed types consist
/// of a prefix ('p' for pointers, 'a' for arrays, 'f_' for functions)
/// combined with the mangling of their component types.  A vararg function
/// type will have a suffix of 'vararg'.  Since function types can contain
/// other function types, we close a function type mangling with suffix 'f'
/// which can't be confused with it's prefix.  This ensures we don't have
/// collisions between two unrelated function types. Otherwise, you might
/// parse ffXX as f(fXX) or f(fX)X.  (X is a placeholder for any other type.)
static std::string getMangledTypeStr(Type *Ty) {
  std::string Result;
  if (PointerType *PTyp = dyn_cast<PointerType>(Ty)) {
    Result += "p" + llvm::utostr(PTyp->getAddressSpace());
#if VC_INTR_LLVM_VERSION_MAJOR >= 13
    if (PTyp->isOpaque())
      return Result;
#endif // VC_INTR_LLVM_VERSION_MAJOR >= 13
    Result += getMangledTypeStr(VCINTR::Type::getNonOpaquePtrEltTy(PTyp));
  } else if (ArrayType *ATyp = dyn_cast<ArrayType>(Ty)) {
    Result += "a" + llvm::utostr(ATyp->getNumElements()) +
              getMangledTypeStr(ATyp->getElementType());
  } else if (StructType *STyp = dyn_cast<StructType>(Ty)) {
    if (!STyp->isLiteral())
      Result += STyp->getName();
    else {
      Result += "s" + llvm::utostr(STyp->getNumElements());
      for (unsigned int i = 0; i < STyp->getNumElements(); i++)
        Result += getMangledTypeStr(STyp->getElementType(i));
    }
  } else if (FunctionType *FT = dyn_cast<FunctionType>(Ty)) {
    Result += "f_" + getMangledTypeStr(FT->getReturnType());
    for (size_t i = 0; i < FT->getNumParams(); i++)
      Result += getMangledTypeStr(FT->getParamType(i));
    if (FT->isVarArg())
      Result += "vararg";
    // Ensure nested function types are distinguishable.
    Result += "f";
  } else if (isa<VectorType>(Ty)) {
    Result += "v" +
              utostr(VCINTR::VectorType::getNumElements(cast<VectorType>(Ty))) +
              getMangledTypeStr(cast<VectorType>(Ty)->getElementType());
#if VC_INTR_LLVM_VERSION_MAJOR >= 16
  } else if (auto *TargetTy = dyn_cast<TargetExtType>(Ty)) {
    Result += "t_" + TargetTy->getName().str();
    for (auto *PTy : TargetTy->type_params())
      Result += "_" + getMangledTypeStr(PTy);
    for (auto I : TargetTy->int_params())
      Result += "_" + llvm::utostr(I);
#endif // VC_INTR_LLVM_VERSION_MAJOR >= 16
  } else if (Ty) {
    Result += EVT::getEVT(Ty).getEVTString();
  }

  return Result;
}

static const char * const GenXIntrinsicNameTable[] = {
    "not_genx_intrinsic",
#define GET_INTRINSIC_NAME_TABLE
#include "llvm/GenXIntrinsics/GenXIntrinsicDescription.gen"
#undef GET_INTRINSIC_NAME_TABLE
  };

bool isOverloaded(GenXIntrinsic::ID id) {
  assert(isGenXIntrinsic(id) && "Invalid intrinsic ID!");
  id = static_cast<GenXIntrinsic::ID>(id - GenXIntrinsic::not_genx_intrinsic);
#define GET_INTRINSIC_OVERLOAD_TABLE
#include "llvm/GenXIntrinsics/GenXIntrinsicDescription.gen"
#undef GET_INTRINSIC_OVERLOAD_TABLE
}

/// This defines the "getAttributes(ID id)" method.
#define GET_INTRINSIC_ATTRIBUTES
#include "llvm/GenXIntrinsics/GenXIntrinsicDescription.gen"
#undef GET_INTRINSIC_ATTRIBUTES

static StringRef GenXIntrinsicMDName{ "genx_intrinsic_id" };

bool GenXIntrinsic::isSupportedPlatform(const std::string &CPU, unsigned id) {
#define GET_INTRINSIC_PLATFORMS
#include "llvm/GenXIntrinsics/GenXIntrinsicDescription.gen"
#undef GET_INTRINSIC_PLATFORMS
  assert(SupportedIntrinsics.find(CPU) != SupportedIntrinsics.end() &&
         "Unknown Platform");
  assert(GenXIntrinsic::isGenXIntrinsic(id) &&
         "this function should be used only for GenXIntrinsics");
  auto PlatformInfoIt = SupportedIntrinsics.find(CPU);
  if (PlatformInfoIt == SupportedIntrinsics.end())
    return false;
  const auto &IntrinsicInfo = PlatformInfoIt->second;
  size_t IntrinsicIdx = id - GenXIntrinsic::ID::not_genx_intrinsic - 1;
  if (IntrinsicIdx < IntrinsicInfo.size())
    return IntrinsicInfo[IntrinsicIdx];
  return false;
}

/// Table of per-target intrinsic name tables.
#define GET_INTRINSIC_TARGET_DATA
#include "llvm/GenXIntrinsics/GenXIntrinsicDescription.gen"
#undef GET_INTRINSIC_TARGET_DATA

bool GenXIntrinsic::isOverloadedArg(unsigned IntrinID, unsigned ArgNum) {
#define GET_INTRINSIC_OVERLOAD_ARGS_TABLE
#include "llvm/GenXIntrinsics/GenXIntrinsicDescription.gen"
#undef GET_INTRINSIC_OVERLOAD_ARGS_TABLE
}

bool GenXIntrinsic::isOverloadedRet(unsigned IntrinID) {
#define GET_INTRINSIC_OVERLOAD_RET_TABLE
#include "llvm/GenXIntrinsics/GenXIntrinsicDescription.gen"
#undef GET_INTRINSIC_OVERLOAD_RET_TABLE
}

/// Find the segment of \c IntrinsicNameTable for intrinsics with the same
/// target as \c Name, or the generic table if \c Name is not target specific.
///
/// Returns the relevant slice of \c IntrinsicNameTable
static ArrayRef<const char *> findTargetSubtable(StringRef Name) {
  assert(Name.startswith("llvm.genx."));

  ArrayRef<IntrinsicTargetInfo> Targets(TargetInfos);
  // Drop "llvm." and take the first dotted component. That will be the target
  // if this is target specific.
  StringRef Target = Name.drop_front(5).split('.').first;
  auto It = std::lower_bound(Targets.begin(), Targets.end(), Target,
                             [](const IntrinsicTargetInfo &TI,
                                StringRef Target) { return TI.Name < Target; });
  // We've either found the target or just fall back to the generic set, which
  // is always first.
  const auto &TI = It != Targets.end() && It->Name == Target ? *It : Targets[0];
  return ArrayRef<const char *>(&GenXIntrinsicNameTable[1] + TI.Offset, TI.Count);
}

GenXIntrinsic::ID GenXIntrinsic::getGenXIntrinsicID(const Function *F) {
  assert(F);
  llvm::StringRef Name = F->getName();
  if (!Name.startswith(getGenXIntrinsicPrefix()))
    return GenXIntrinsic::not_genx_intrinsic;

  // Check metadata cache.
  if (auto *MD = F->getMetadata(GenXIntrinsicMDName)) {
    assert(MD->getNumOperands() == 1 && "Invalid intrinsic metadata");
    auto Val = cast<ValueAsMetadata>(MD->getOperand(0))->getValue();
    GenXIntrinsic::ID Id =
        static_cast<GenXIntrinsic::ID>(cast<ConstantInt>(Val)->getZExtValue());

    // we need to check that metadata is correct and can be actually used
    if (isGenXIntrinsic(Id)) {
      const char *NamePrefix =
          GenXIntrinsicNameTable[Id - GenXIntrinsic::not_genx_intrinsic];
      if (Name.startswith(NamePrefix))
        return Id;
    }
  }

  // Fallback to string lookup.
  auto ID = lookupGenXIntrinsicID(Name);
  assert(ID != GenXIntrinsic::not_genx_intrinsic && "Intrinsic not found!");
  return ID;
}

std::string GenXIntrinsic::getGenXName(GenXIntrinsic::ID id,
                                       ArrayRef<Type *> Tys) {
  assert(isGenXIntrinsic(id) && "Invalid intrinsic ID!");
  assert(Tys.empty() ||
         (isOverloaded(id) && "Non-overloadable intrinsic was overloaded!"));
  id = static_cast<GenXIntrinsic::ID>(id - GenXIntrinsic::not_genx_intrinsic);
  std::string Result(GenXIntrinsicNameTable[id]);
  for (Type *Ty : Tys) {
    Result += "." + getMangledTypeStr(Ty);
  }
  return Result;
}

GenXIntrinsic::ID GenXIntrinsic::lookupGenXIntrinsicID(StringRef Name) {
  ArrayRef<const char *> NameTable = findTargetSubtable(Name);
  int Idx = Intrinsic::lookupLLVMIntrinsicByName(NameTable, Name);
  if (Idx == -1)
    return GenXIntrinsic::not_genx_intrinsic;

  // Intrinsic IDs correspond to the location in IntrinsicNameTable, but we have
  // an index into a sub-table.
  int Adjust = NameTable.data() - GenXIntrinsicNameTable;
  auto ID = static_cast<GenXIntrinsic::ID>(Idx + Adjust + GenXIntrinsic::not_genx_intrinsic);

  // If the intrinsic is not overloaded, require an exact match. If it is
  // overloaded, require either exact or prefix match.
  assert(Name.size() >= strlen(NameTable[Idx]) &&
         "Expected either exact or prefix match");
  assert((Name.size() == strlen(NameTable[Idx])) ||
         (isOverloaded(ID) && "Non-overloadable intrinsic was overloaded!"));
  return ID;
}

FunctionType *GenXIntrinsic::getGenXType(LLVMContext &Context,
                                         GenXIntrinsic::ID id,
                                         ArrayRef<Type *> Tys) {
  SmallVector<Intrinsic::IITDescriptor, 8> Table;
  getIntrinsicInfoTableEntries(id, Table);

  ArrayRef<Intrinsic::IITDescriptor> TableRef = Table;
  Type *ResultTy = DecodeFixedType(TableRef, Tys, Context);

  SmallVector<Type *, 8> ArgTys;
  while (!TableRef.empty())
    ArgTys.push_back(DecodeFixedType(TableRef, Tys, Context));

  // DecodeFixedType returns Void for IITDescriptor::Void and
  // IITDescriptor::VarArg If we see void type as the type of the last argument,
  // it is vararg intrinsic
  if (!ArgTys.empty() && ArgTys.back()->isVoidTy()) {
    ArgTys.pop_back();
    return FunctionType::get(ResultTy, ArgTys, true);
  }
  return FunctionType::get(ResultTy, ArgTys, false);
}

#ifndef NDEBUG
// Sanity check for intrinsic types.
// After translation from SPIRV literal structures become identified.
// However, if intrinsic returns multiple values, then it returns
// literal structure.
// Having this, compatible intrinsics will have same argument types
// and either same return types or layout identical structure types.
static bool isCompatibleIntrinsicSignature(FunctionType *DecodedType,
                                           FunctionType *FoundType) {
  if (DecodedType == FoundType)
    return true;

  if (DecodedType->params() != FoundType->params())
    return false;

  // Return types are different. Check for structures.
  auto *DecStrTy = dyn_cast<StructType>(DecodedType->getReturnType());
  auto *FoundStrTy = dyn_cast<StructType>(FoundType->getReturnType());
  if (!DecStrTy || !FoundStrTy)
    return false;

  return DecStrTy->isLayoutIdentical(FoundStrTy);
}
#endif

Function *GenXIntrinsic::getGenXDeclaration(Module *M, GenXIntrinsic::ID id,
                                            ArrayRef<Type *> Tys) {
  assert(isGenXNonTrivialIntrinsic(id));
  assert(Tys.empty() ||
         (isOverloaded(id) && "Non-overloadable intrinsic was overloaded!"));

  auto GenXName = getGenXName(id, Tys);
  FunctionType *FTy = getGenXType(M->getContext(), id, Tys);
  Function *F = M->getFunction(GenXName);
  if (!F)
    F = Function::Create(FTy, GlobalVariable::ExternalLinkage, GenXName, M);

  assert(isCompatibleIntrinsicSignature(FTy, F->getFunctionType()) &&
         "Module contains intrinsic declaration with incompatible type!");

  resetGenXAttributes(F);
  return F;
}

void GenXIntrinsic::resetGenXAttributes(Function *F) {

  assert(F);

  GenXIntrinsic::ID GXID = getGenXIntrinsicID(F);

  assert(GXID != GenXIntrinsic::not_genx_intrinsic);

  // Since Function::isIntrinsic() will return true due to llvm. prefix,
  // Module::getOrInsertFunction fails to add the attributes. explicitly adding
  // the attribute to handle this problem. This since is setup on the function
  // declaration, attribute assignment is global and hence this approach
  // suffices.
  F->setAttributes(GenXIntrinsic::getAttributes(F->getContext(), GXID));

  // Cache intrinsic ID in metadata.
  if (EnableGenXIntrinsicsCache && !F->hasMetadata(GenXIntrinsicMDName)) {
    LLVMContext &Ctx = F->getContext();
    auto *Ty = IntegerType::getInt32Ty(Ctx);
    auto *Cached = ConstantInt::get(Ty, GXID);
    auto *MD = MDNode::get(Ctx, {ConstantAsMetadata::get(Cached)});
    F->addMetadata(GenXIntrinsicMDName, *MD);
  }
}

std::string GenXIntrinsic::getAnyName(unsigned id, ArrayRef<Type *> Tys) {
  assert(isAnyIntrinsic(id));
  if (id == not_any_intrinsic) {
    std::string Result("not_any_intrinsic");
    for (Type *Ty : Tys) {
      Result += "." + getMangledTypeStr(Ty);
    }
    return Result;
  } else if (isGenXIntrinsic(id))
    return getGenXName((GenXIntrinsic::ID)id, Tys);
  else
    return VCINTR::Intrinsic::getName((Intrinsic::ID)id, Tys);
}

GenXIntrinsic::LSCVectorSize GenXIntrinsic::getLSCVectorSize(
    const Instruction *I) {
  assert(isLSC(I));
  const int VectorSizeIdx = LSCArgIdx::getLSCVectorSize(getLSCCategory(I));
  if (VectorSizeIdx == LSCArgIdx::Invalid)
    return LSCVectorSize::N0;
  return static_cast<LSCVectorSize>(
      cast<ConstantInt>(I->getOperand(VectorSizeIdx))->getZExtValue());
}

GenXIntrinsic::LSCDataSize GenXIntrinsic::getLSCDataSize(
    const Instruction *I) {
  assert(isLSC(I));
  const int DataSizeIdx = LSCArgIdx::getLSCDataSize(getLSCCategory(I));
  if (DataSizeIdx == LSCArgIdx::Invalid)
    return LSCDataSize::Invalid;
  return static_cast<LSCDataSize>(
      cast<ConstantInt>(I->getOperand(DataSizeIdx))->getZExtValue());
}

GenXIntrinsic::LSCDataOrder GenXIntrinsic::getLSCDataOrder(
    const Instruction *I) {
  assert(isLSC(I));
  const int DataOrderIdx = LSCArgIdx::getLSCDataOrder(getLSCCategory(I));
  if (DataOrderIdx == LSCArgIdx::Invalid)
    return LSCDataOrder::Invalid;
  return static_cast<LSCDataOrder>(
      cast<ConstantInt>(I->getOperand(DataOrderIdx))->getZExtValue());
}

unsigned GenXIntrinsic::getLSCWidth(const Instruction *I) {
  assert(isLSC(I));
  const int WidthIdx = LSCArgIdx::getLSCWidth(getLSCCategory(I));
  if (WidthIdx == LSCArgIdx::Invalid)
    return 1;
  if (auto VT = dyn_cast<VectorType>(I->getOperand(WidthIdx)->getType()))
    return VCINTR::VectorType::getNumElements(VT);
  return 1;
}

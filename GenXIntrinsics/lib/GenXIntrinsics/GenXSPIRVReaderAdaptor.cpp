/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2025 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This pass converts metadata from SPIRV format to whichever used in backend.

#include "AdaptorsCommon.h"
#include "GenXSingleElementVectorUtil.h"

#include "llvm/GenXIntrinsics/GenXIntrinsics.h"
#include "llvm/GenXIntrinsics/GenXMetadata.h"
#include "llvm/GenXIntrinsics/GenXSPIRVReaderAdaptor.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#include "llvmVCWrapper/IR/Attributes.h"
#include "llvmVCWrapper/IR/Function.h"
#include "llvmVCWrapper/IR/Instructions.h"
#include "llvmVCWrapper/IR/Type.h"

using namespace llvm;
using namespace genx;

namespace {

class GenXSPIRVReaderAdaptorImpl final {
public:
  explicit GenXSPIRVReaderAdaptorImpl() {}
  bool run(Module &M);

private:
  bool runOnFunction(Function &F);

  bool processVCFunctionAttributes(Function &F);
  bool processVCKernelAttributes(Function &F);

  void dropAttributeAtIndex(Function &F, unsigned Index, StringRef Kind) {
    auto NewAttributes = VCINTR::AttributeList::removeAttributeAtIndex(
            F.getContext(), F.getAttributes(), Index, Kind);
    F.setAttributes(NewAttributes);
  }
  void dropFnAttribute(Function &F, StringRef Kind) {
    dropAttributeAtIndex(F, AttributeList::FunctionIndex, Kind);
  }
};

} // namespace

static std::pair<SPIRVType, StringRef> parseImageDim(StringRef TyName) {
  // Greedy match: 1d_buffer first.
  if (TyName.consume_front(OCLTypes::Dim1dBuffer))
    return {SPIRVType::Image1dBuffer, TyName};

  if (TyName.consume_front(OCLTypes::Dim1dArray))
    return {SPIRVType::Image1dArray, TyName};

  if (TyName.consume_front(OCLTypes::Dim1d))
    return {SPIRVType::Image1d, TyName};

  if (TyName.consume_front(OCLTypes::Dim2dArray))
    return {SPIRVType::Image2dArray, TyName};

  if (TyName.consume_front(OCLTypes::Dim2d))
    return {SPIRVType::Image2d, TyName};

  if (TyName.consume_front(OCLTypes::Dim3d))
    return {SPIRVType::Image3d, TyName};

  llvm_unreachable("Unexpected image dimensionality");
}

static std::pair<AccessType, StringRef> parseAccessQualifier(StringRef TyName) {
  if (TyName.consume_front(CommonTypes::ReadOnly))
    return {AccessType::ReadOnly, TyName};

  if (TyName.consume_front(CommonTypes::WriteOnly))
    return {AccessType::WriteOnly, TyName};

  if (TyName.consume_front(CommonTypes::ReadWrite))
    return {AccessType::ReadWrite, TyName};

  llvm_unreachable("Unexpected image access modifier");
}

static SPIRVArgDesc parseImageType(StringRef TyName) {
  const bool Consumed = TyName.consume_front(OCLTypes::Image);
  assert(Consumed && "Unexpected opencl type");
  (void)Consumed;

  SPIRVType ImageType;
  std::tie(ImageType, TyName) = parseImageDim(TyName);
  AccessType AccType;
  std::tie(AccType, TyName) = parseAccessQualifier(TyName);
#if VC_INTR_LLVM_VERSION_MAJOR >= 16
  assert(TyName.starts_with(CommonTypes::TypeSuffix) && "Bad image type");
#else
  assert(TyName.startswith(CommonTypes::TypeSuffix) && "Bad image type");
#endif
  return {ImageType, AccType};
}

static std::pair<SPIRVType, StringRef> parseIntelMainType(StringRef TyName) {
  if (TyName.consume_front(IntelTypes::Buffer))
    return {SPIRVType::Buffer, TyName};

  if (TyName.consume_front(IntelTypes::MediaBlockImage))
    return {SPIRVType::Image2dMediaBlock, TyName};

  llvm_unreachable("Unexpected intel extension type");
}

template <typename T> T consumeIntegerLiteral(StringRef TyName) {
  int Literal;

  auto ProperlyConsumed = !TyName.consumeInteger(0, Literal);
  assert(ProperlyConsumed && "Expected string to rpresent integer literal");
  (void)ProperlyConsumed;

  return static_cast<T>(Literal);
}

static SPIRVType evaluateImageTypeFromSPVIR(SPIRVIRTypes::Dim Dim,
                                            bool Arrayed) {
  SPIRVType ResultType;
  if (!Arrayed) {
    switch (Dim) {
    case SPIRVIRTypes::Dim1D:
      ResultType = SPIRVType::Image1d;
      break;
    case SPIRVIRTypes::Dim2D:
      ResultType = SPIRVType::Image2d;
      break;
    case SPIRVIRTypes::Dim3D:
      ResultType = SPIRVType::Image3d;
      break;
    case SPIRVIRTypes::DimBuffer:
      ResultType = SPIRVType::Image1dBuffer;
      break;
    }
  } else {
    switch (Dim) {
    case SPIRVIRTypes::Dim1D:
      ResultType = SPIRVType::Image1dArray;
      break;
    case SPIRVIRTypes::Dim2D:
      ResultType = SPIRVType::Image2dArray;
      break;
    default:
      llvm_unreachable("Bad Image Type");
    }
  }

  return ResultType;
}

static StringRef skipUnderscores(StringRef StrRef, int Count) {
  for (int i = 0; i < Count; ++i) {
    StrRef = StrRef.drop_while([](char C) { return C != '_'; });
    StrRef = StrRef.drop_front(1);
  }

  return StrRef;
}

static SPIRVArgDesc parseSPIRVIRImageType(StringRef TyName) {
  const bool Consumed = TyName.consume_front(SPIRVIRTypes::Image);
  assert(Consumed && "Unexpected SPIRV friendly IR type");
  (void)Consumed;

  // SPIRV friendly Ir image type looks like this:
  // spirv.Image._{Sampled T}_{Dim}_{Depth}_{Arrayed}_{MS}_{Fmt}_{Acc}

  // skip dot
  TyName = TyName.drop_front(1);

  // skip Samled Type.
  TyName = skipUnderscores(TyName, 2);

  auto Dim = consumeIntegerLiteral<SPIRVIRTypes::Dim>(TyName);

  // Skip Depth.
  TyName = skipUnderscores(TyName, 2);

  auto Arrayed = consumeIntegerLiteral<bool>(TyName);

  // Skip Multisampling and Format.
  TyName = skipUnderscores(TyName, 4);

  AccessType AccessTy = AccessType::ReadOnly;

  if (!TyName.empty())
    AccessTy = consumeIntegerLiteral<AccessType>(TyName);

  auto ResultType = evaluateImageTypeFromSPVIR(Dim, Arrayed);

  return {ResultType, AccessTy};
}

static VCINTR::Optional<SPIRVArgDesc> parseIntelType(StringRef TyName) {
  if (!TyName.consume_front(IntelTypes::TypePrefix))
    return {};

  SPIRVType MainType;
  std::tie(MainType, TyName) = parseIntelMainType(TyName);
  AccessType AccType;
  std::tie(AccType, TyName) = parseAccessQualifier(TyName);
#if VC_INTR_LLVM_VERSION_MAJOR >= 16
  assert(TyName.starts_with(CommonTypes::TypeSuffix) && "Bad intel type");
#else
  assert(TyName.startswith(CommonTypes::TypeSuffix) && "Bad intel type");
#endif
  return SPIRVArgDesc{MainType, AccType};
}

static VCINTR::Optional<SPIRVArgDesc> parseOCLType(StringRef TyName) {
  if (!TyName.consume_front(OCLTypes::TypePrefix))
    return {};

  // Sampler type.
  if (TyName.consume_front(OCLTypes::Sampler)) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 16
    assert(TyName.starts_with(CommonTypes::TypeSuffix) && "Bad sampler type");
#else
    assert(TyName.startswith(CommonTypes::TypeSuffix) && "Bad sampler type");
#endif
    return {SPIRVType::Sampler};
  }

  // Images are the rest.
  return parseImageType(TyName);
}

static VCINTR::Optional<SPIRVArgDesc> parseSPIRVIRType(StringRef TyName) {
  if (!TyName.consume_front(SPIRVIRTypes::TypePrefix))
    return {};

  if (TyName.consume_front(SPIRVIRTypes::Sampler))
    return {SPIRVType::Sampler};

  return parseSPIRVIRImageType(TyName);
}
// Parse opaque type name.
// Ty -> "opencl." OCLTy | "spirv." SPVIRTy | "intel" IntelTy
// OCLTy -> "sampler_t" | ImageTy
// IntelTy -> MainIntelTy Acc "_t"
// MainIntelTy -> "buffer" | "image2d_media_block"
// ImageTy -> "image" Dim Acc "_t"
// Dim -> "1d" | "1d_buffer" | "2d" | "3d"
// Acc -> "_ro" | "_wo" | "_rw"
// SPVIRTy -> "Sampler" | SPVImageTy
// SPVImageTy -> "Image." _..._{Dim}_..._{Arrayed}_..._{Acc}
// Dim, Arrayed, Acc - literal operands matching OpTypeImage operands in SPIRV
// Assume that "opencl." "spirv." and "intel.buffer" types are well-formed.
static VCINTR::Optional<SPIRVArgDesc> parseOpaqueType(StringRef TyName) {
  if (auto MaybeIntelTy = parseIntelType(TyName))
    return VCINTR::getValue(MaybeIntelTy);

  if (auto MaybeOCL = parseOCLType(TyName))
    return VCINTR::getValue(MaybeOCL);

  return parseSPIRVIRType(TyName);
}

#if VC_INTR_LLVM_VERSION_MAJOR >= 16
static SPIRVArgDesc analyzeTargetExtTypeArg(const Argument &Arg,
                                            TargetExtType *TET) {
  auto TyName = TET->getName();
  if (TyName.consume_front(SPIRVIRTypes::TypePrefix)) {
    if (TyName.consume_front(SPIRVIRTypes::Sampler))
      return {SPIRVType::Sampler};
    if (TyName.consume_front(SPIRVIRTypes::Buffer)) {
      assert(TET->getNumIntParameters() == 1);
      auto Acc = static_cast<AccessType>(TET->getIntParameter(0));
      return SPIRVArgDesc(SPIRVType::Buffer, Acc);
    }
    if (TyName.consume_front(SPIRVIRTypes::Image)) {
      auto Dim = static_cast<SPIRVIRTypes::Dim>(
          TET->getIntParameter(SPIRVIRTypes::Dimension));
      auto Arr = static_cast<bool>(TET->getIntParameter(SPIRVIRTypes::Arrayed));
      auto Acc =
          static_cast<AccessType>(TET->getIntParameter(SPIRVIRTypes::Access));
      auto SpvTy = evaluateImageTypeFromSPVIR(Dim, Arr);
      if (SpvTy == SPIRVType::Image2d &&
          Arg.getParent()->getAttributes().hasParamAttr(
              Arg.getArgNo(), VCFunctionMD::VCMediaBlockIO))
        SpvTy = SPIRVType::Image2dMediaBlock;
      return SPIRVArgDesc(SpvTy, Acc);
    }
    llvm_unreachable("Unexpected spirv target extension type");
  }
  llvm_unreachable("Unexpected target extension type");
}
#endif //VC_INTR_LLVM_VERSION_MAJOR >= 16

static SPIRVArgDesc analyzeKernelArg(const Argument &Arg) {
  const Function *F = Arg.getParent();
  // If there is vc attribute, then no conversion is needed.
  if (F->getAttributes().hasParamAttr(Arg.getArgNo(),
                                      VCFunctionMD::VCArgumentKind))
    return {SPIRVType::None};

  Type *Ty = Arg.getType();
#if VC_INTR_LLVM_VERSION_MAJOR >= 16
  if (auto *TET = dyn_cast<TargetExtType>(Ty))
    return analyzeTargetExtTypeArg(Arg, TET);
#endif //VC_INTR_LLVM_VERSION_MAJOR >= 16
  // Not a pointer means that it is general argument without annotation.
  if (!isa<PointerType>(Ty))
    return {SPIRVType::Other};

  auto *PointerTy = cast<PointerType>(Ty);
  // Annotated things are converted to global and constant pointers.
  const unsigned AddressSpace = PointerTy->getAddressSpace();
  if (AddressSpace != SPIRVParams::SPIRVGlobalAS &&
      AddressSpace != SPIRVParams::SPIRVConstantAS)
    return {SPIRVType::Other};

  if (VCINTR::Type::isOpaquePointerTy(Ty))
    return {SPIRVType::Pointer};

  Type *PointeeTy = VCINTR::Type::getNonOpaquePtrEltTy(PointerTy);
  // Not a pointer to struct, cannot be sampler or image.
  if (!isa<StructType>(PointeeTy))
    return {SPIRVType::Pointer};

  auto *StrTy = cast<StructType>(PointeeTy);
  // Pointer to literal structure, cannot be sampler or image.
  // (is this case possible in SPIRV translator?)
  if (!StrTy->hasName())
    return {SPIRVType::Pointer};

  if (auto MaybeDesc = parseOpaqueType(StrTy->getName())) {
    SPIRVArgDesc Desc = VCINTR::getValue(MaybeDesc);
    assert(getOpaqueTypeAddressSpace(Desc.Ty) == AddressSpace &&
           "Mismatching address space for type");
    return Desc;
  }

  // If nothing was matched then it is simple pointer.
  return {SPIRVType::Pointer};
}

static std::vector<SPIRVArgDesc> analyzeKernelArguments(Function &F) {
  std::vector<SPIRVArgDesc> Descs;
  std::transform(F.arg_begin(), F.arg_end(), std::back_inserter(Descs),
                 [](const Argument &Arg) { return analyzeKernelArg(Arg); });
  return Descs;
}

static bool isArgConvIntrinsic(const Value *V) {
  return GenXIntrinsic::getGenXIntrinsicID(V) ==
         GenXIntrinsic::genx_address_convert;
}

// Get original value that should be used in restored kernel.
// SPIRV arguments converted to old style with address convert intrinsic
// so if intrinsic is present, then its type should be used instead of
// current argument. Otherwise argument was not changed.
static Value *getOriginalValue(Argument &Arg) {
  if (Arg.hasOneUse()) {
    User *U = Arg.user_back();
    if (isArgConvIntrinsic(U) || isa<BitCastInst>(U) ||
        isa<AddrSpaceCastInst>(U) || isa<IntToPtrInst>(U))
      return U;
  }

  assert(llvm::none_of(Arg.users(), isArgConvIntrinsic) &&
         "Arg convert can occur as the only user of argument");
  return &Arg;
}

static ArgKind mapSPIRVTypeToArgKind(SPIRVType Ty) {
  switch (Ty) {
  case SPIRVType::Buffer:
  case SPIRVType::Image1d:
  case SPIRVType::Image1dArray:
  case SPIRVType::Image1dBuffer:
  case SPIRVType::Image2d:
  case SPIRVType::Image2dArray:
  case SPIRVType::Image2dMediaBlock:
  case SPIRVType::Image3d:
    return ArgKind::Surface;
  case SPIRVType::Sampler:
    return ArgKind::Sampler;
  case SPIRVType::Pointer:
  case SPIRVType::Other:
    return ArgKind::General;
  case SPIRVType::None:
    break;
  }
  llvm_unreachable("Unexpected spirv type");
}

static std::string mapSPIRVDescToArgDesc(SPIRVArgDesc SPIRVDesc) {
  std::string Desc;
  switch (SPIRVDesc.Ty) {
  case SPIRVType::Buffer:
    Desc += ArgDesc::Buffer;
    break;
  case SPIRVType::Image1d:
    Desc += ArgDesc::Image1d;
    break;
  case SPIRVType::Image1dArray:
    Desc += ArgDesc::Image1dArray;
    break;
  case SPIRVType::Image1dBuffer:
    Desc += ArgDesc::Image1dBuffer;
    break;
  case SPIRVType::Image2d:
    Desc += ArgDesc::Image2d;
    break;
  case SPIRVType::Image2dArray:
    Desc += ArgDesc::Image2dArray;
    break;
  case SPIRVType::Image2dMediaBlock:
    Desc += ArgDesc::Image2dMediaBlock;
    break;
  case SPIRVType::Image3d:
    Desc += ArgDesc::Image3d;
    break;
  case SPIRVType::Sampler:
    return ArgDesc::Sampler;
  case SPIRVType::Pointer:
    return ArgDesc::SVM;
  case SPIRVType::Other:
    return {};
  default:
    llvm_unreachable("Unexpected spirv type");
  }

  Desc += ' ';

  // Surface arg kinds also have access modifier.
  switch (SPIRVDesc.Acc) {
  case AccessType::ReadOnly:
    Desc += ArgDesc::ReadOnly;
    break;
  case AccessType::WriteOnly:
    Desc += ArgDesc::WriteOnly;
    break;
  case AccessType::ReadWrite:
    Desc += ArgDesc::ReadWrite;
    break;
  }

  return Desc;
}

static PointerType *getKernelArgPointerType(PointerType *ConvertTy,
                                            PointerType *ArgTy) {
  auto AddressSpace = ConvertTy->getPointerAddressSpace();
  if (VCINTR::Type::isOpaquePointerTy(ArgTy))
    return VCINTR::PointerType::getWithSamePointeeType(ArgTy, AddressSpace);
  auto *ConvertPointeeTy = VCINTR::Type::getNonOpaquePtrEltTy(ConvertTy);
  auto *ArgPointeeTy = VCINTR::Type::getNonOpaquePtrEltTy(ArgTy);

  if (ConvertPointeeTy->isAggregateType())
    return ConvertTy;

  return ArgPointeeTy->getPointerTo(AddressSpace);
}

// Create new empty function with restored types based on old function and
// arguments descriptors.
static Function *
transformKernelSignature(Function &F, const std::vector<SPIRVArgDesc> &Descs) {
  // Collect new kernel argument types.
  std::vector<Type *> NewTypes;
  std::transform(F.arg_begin(), F.arg_end(), std::back_inserter(NewTypes),
                 [Descs](Argument &Arg) -> llvm::Type * {
                   auto *Orig = getOriginalValue(Arg);
                   auto *OrigTy = Orig->getType();
                   auto *ArgTy = Arg.getType();
                   if (isArgConvIntrinsic(Orig)) {
                     if (ArgTy->isPointerTy() && OrigTy->isIntegerTy(64))
                       return ArgTy;
#if VC_INTR_LLVM_VERSION_MAJOR > 15
                     if (ArgTy->isTargetExtTy()) {
                       auto &Ctx = Arg.getContext();
#if VC_INTR_LLVM_VERSION_MAJOR == 16
                       assert(!Ctx.supportsTypedPointers() &&
                              "Target extension types should be used only with "
                              "opaque pointers");
#endif
                       unsigned AddrSpace =
                           getOpaqueTypeAddressSpace(Descs[Arg.getArgNo()].Ty);
                       return PointerType::get(Ctx, AddrSpace);
                     }
#endif
                   }
                   if (OrigTy->isPointerTy() && ArgTy->isPointerTy())
                     return getKernelArgPointerType(cast<PointerType>(OrigTy),
                                                    cast<PointerType>(ArgTy));
                   return OrigTy;
                 });

  auto *NewFTy = FunctionType::get(F.getReturnType(), NewTypes, false);
  auto *NewF = Function::Create(NewFTy, F.getLinkage(), F.getAddressSpace());

  // Copy function info.
  LLVMContext &Ctx = F.getContext();
  NewF->copyAttributesFrom(&F);
  NewF->takeName(&F);
  NewF->copyMetadata(&F, 0);
  NewF->setComdat(F.getComdat());

  // Set appropriate argument attributes related to kind and desc.
  std::string ArgDesc;
  for (int i = 0, e = Descs.size(); i != e; ++i) {
    SPIRVArgDesc SPVDesc = Descs[i];
    // No need to set things, old style argument attributes were copied before.
    if (SPVDesc.Ty == SPIRVType::None)
      continue;

    // Add needed attributes to newly created function argument.
    ArgKind AK = mapSPIRVTypeToArgKind(SPVDesc.Ty);
    ArgDesc = mapSPIRVDescToArgDesc(SPVDesc);
    Attribute Attr = Attribute::get(Ctx, VCFunctionMD::VCArgumentKind,
                                    std::to_string(static_cast<unsigned>(AK)));
    NewF->addParamAttr(i, Attr);
    Attr = Attribute::get(Ctx, VCFunctionMD::VCArgumentDesc, ArgDesc);
    NewF->addParamAttr(i, Attr);
  }

  legalizeParamAttributes(NewF);

  return NewF;
}

// Rewrite function if it has SPIRV types as parameters.
// Function
//  define spir_kernel @foo(%opencl.image2d_rw_t addrspace(1)* %im) {
//    %conv = ptrtoint %opencl.image2d_rw_t addrspace(1)* %im to i32
//   ...
//  }
// will be changed to
//  define spir_kernel @foo(i32 "VCArgumentKind"="2" "VCArgumentDesc"="image2d_t
//  read_write" %im) {
//    ...
//  }
// If parameter has at least "VCArgumentKind" attribute then it is not
// converted.
static void rewriteKernelArguments(Function &F) {
  std::vector<SPIRVArgDesc> ArgDescs = analyzeKernelArguments(F);
  if (std::all_of(
          ArgDescs.begin(), ArgDescs.end(),
          [](const SPIRVArgDesc Desc) { return Desc.Ty == SPIRVType::None; }))
    // All arguments are in old style.
    return;

  // At the moment there are only two cases when kernel function with converted
  // parameters can have users:
  // 1. Kernel is called from another function via fast composite
  //    For such kernels we just don't rewrite arguments on SPIRV write, so
  //    there should not be presented on read
  // 2. Kernel is referenced in @llvm.global.annotations
  //    We have to replace the original function with the new one
  if (!F.use_empty()) {
    Value *Ptr = &F;
    assert(Ptr->hasOneUse());
    if (isa<BitCastOperator>(Ptr->user_back())) {
      Ptr = Ptr->user_back();
      assert(Ptr->hasOneUse());
    }
    auto *Struct = Ptr->user_back();
    assert(Struct->hasOneUse());
    auto *Array = Struct->user_back();
    assert(Array->hasOneUse());
    auto *GV = dyn_cast<GlobalVariable>(Array->user_back());
    assert(GV && GV->getName() == "llvm.global.annotations");
  }

  Function *NewF = transformKernelSignature(F, ArgDescs);
  F.getParent()->getFunctionList().insert(F.getIterator(), NewF);
#if VC_INTR_LLVM_VERSION_MAJOR > 15
  NewF->splice(NewF->begin(), &F);
#else
  NewF->getBasicBlockList().splice(NewF->begin(), F.getBasicBlockList());
#endif
  // Rewrite uses and delete conversion intrinsics.
  for (int i = 0, e = ArgDescs.size(); i != e; ++i) {
    Argument &OldArg = *std::next(F.arg_begin(), i);
    Argument &NewArg = *std::next(NewF->arg_begin(), i);
    Value *Orig = getOriginalValue(OldArg);

    NewArg.takeName(&OldArg);

    auto *OrigTy = Orig->getType();
    auto *NewTy = NewArg.getType();

    Value *NewVal = &NewArg;

    if (isa<Instruction>(Orig) && OrigTy != NewTy) {
      IRBuilder<> Builder(cast<Instruction>(Orig));
      if (OrigTy->isIntegerTy())
        NewVal = Builder.CreatePtrToInt(NewVal, OrigTy);
      else
        NewVal = Builder.CreatePointerBitCastOrAddrSpaceCast(NewVal, OrigTy);
    }

    Orig->replaceAllUsesWith(NewVal);

    if (Orig != &OldArg) {
      cast<Instruction>(Orig)->eraseFromParent();
    }
  }

  F.mutateType(NewF->getType());
  F.replaceAllUsesWith(NewF);
  F.eraseFromParent();
}

// Rewrite kernels from SPIRV representation to old style VC
// integers with attributes as incoming parameters.
static void rewriteKernelsTypes(Module &M) {
  SmallVector<Function *, 4> Kernels;
  std::transform(M.begin(), M.end(), std::back_inserter(Kernels),
                 [](Function &F) { return &F; });
  for (auto *F : Kernels) {
    // Skip things that are not VC kernels.
    if (F->getCallingConv() != CallingConv::SPIR_KERNEL)
      continue;
    if (!VCINTR::AttributeList::hasFnAttr(F->getAttributes(),
                                          VCFunctionMD::VCFunction))
      continue;
    rewriteKernelArguments(*F);
  }
}

bool GenXSPIRVReaderAdaptorImpl::run(Module &M) {
  auto *KernelMDs = M.getNamedMetadata(FunctionMD::GenXKernels);
  if (KernelMDs)
    return false;

  for (auto &&GV : M.globals()) {
    if (!GV.hasAttribute(VCModuleMD::VCGlobalVariable))
      continue;
    if (GV.hasAttribute(VCModuleMD::VCVolatile))
      GV.addAttribute(FunctionMD::GenXVolatile);
    if (GV.hasAttribute(VCModuleMD::VCByteOffset)) {
      auto Offset =
          GV.getAttribute(VCModuleMD::VCByteOffset).getValueAsString();
      GV.addAttribute(FunctionMD::GenXByteOffset, Offset);
    }
  }

  rewriteKernelsTypes(M);
  SEVUtil(M).restoreSEVs();

  for (auto &&F : M)
    runOnFunction(F);

  return true;
}

bool GenXSPIRVReaderAdaptorImpl::processVCFunctionAttributes(Function &F) {
  auto Attrs = F.getAttributes();
  if (!VCINTR::AttributeList::hasFnAttr(Attrs, VCFunctionMD::VCFunction))
    return false;

  dropFnAttribute(F, VCFunctionMD::VCFunction);

  if (VCINTR::AttributeList::hasFnAttr(Attrs, VCFunctionMD::VCStackCall)) {
    F.addFnAttr(FunctionMD::CMStackCall);
    dropFnAttribute(F, VCFunctionMD::VCStackCall);
  }

  if (VCINTR::AttributeList::hasFnAttr(Attrs, VCFunctionMD::VCCallable)) {
    F.addFnAttr(FunctionMD::CMCallable);
    dropFnAttribute(F, VCFunctionMD::VCCallable);
  }

  if (VCINTR::AttributeList::hasFnAttr(Attrs, VCFunctionMD::VCFCEntry)) {
    F.addFnAttr(FunctionMD::CMEntry);
    dropFnAttribute(F, VCFunctionMD::VCFCEntry);
  }

  if (VCINTR::AttributeList::hasFnAttr(Attrs, VCFunctionMD::VCSIMTCall)) {
    auto SIMTMode = StringRef();
    SIMTMode =
        VCINTR::AttributeList::getAttributeAtIndex(
            Attrs, AttributeList::FunctionIndex, VCFunctionMD::VCSIMTCall)
            .getValueAsString();
    F.addFnAttr(FunctionMD::CMGenxSIMT, SIMTMode);
    dropFnAttribute(F, VCFunctionMD::VCSIMTCall);
  }

  auto &&Context = F.getContext();
  if (VCINTR::AttributeList::hasFnAttr(Attrs, VCFunctionMD::VCFloatControl)) {
    auto FloatControl = unsigned(0);
    VCINTR::AttributeList::getAttributeAtIndex(
        Attrs, AttributeList::FunctionIndex, VCFunctionMD::VCFloatControl)
        .getValueAsString()
        .getAsInteger(0, FloatControl);

    auto Attr = Attribute::get(Context, FunctionMD::CMFloatControl,
                               std::to_string(FloatControl));
    VCINTR::Function::addAttributeAtIndex(F, AttributeList::FunctionIndex,
                                          Attr);
    dropFnAttribute(F, VCFunctionMD::VCFloatControl);
  }

  if (auto *ReqdSubgroupSize =
          F.getMetadata(SPIRVParams::SPIRVSIMDSubgroupSize)) {
    auto SIMDSize =
        mdconst::extract<ConstantInt>(ReqdSubgroupSize->getOperand(0))
            ->getZExtValue();
    Attribute Attr = Attribute::get(Context, FunctionMD::OCLRuntime,
                                    std::to_string(SIMDSize));
    VCINTR::Function::addAttributeAtIndex(F, AttributeList::FunctionIndex,
                                          Attr);
  }
  return true;
}

bool GenXSPIRVReaderAdaptorImpl::processVCKernelAttributes(Function &F) {
  if (!(F.getCallingConv() == CallingConv::SPIR_KERNEL))
    return false;

  F.addFnAttr(FunctionMD::CMGenXMain);
  F.setDLLStorageClass(llvm::GlobalVariable::DLLExportStorageClass);

  auto Attrs = F.getAttributes();

  auto *FunctionRef = ValueAsMetadata::get(&F);
  auto KernelName = F.getName();
  auto ArgKinds = llvm::SmallVector<llvm::Metadata *, 8>();
  auto SLMSize = unsigned(0);
  auto ArgOffset = unsigned(0);
  auto ArgIOKinds = llvm::SmallVector<llvm::Metadata *, 8>();
  auto ArgDescs = llvm::SmallVector<llvm::Metadata *, 8>();
  auto NBarrierCnt = unsigned(0);

  auto &&Context = F.getContext();
  llvm::Type *I32Ty = llvm::Type::getInt32Ty(Context);

  for (Function::arg_iterator I = F.arg_begin(), E = F.arg_end(); I != E; ++I) {
    auto ArgNo = I->getArgNo();
    auto ArgKind = unsigned(0);
    auto ArgIOKind = unsigned(0);
    auto ArgDesc = std::string();
    auto AttrIndex = ArgNo + 1;

    if (VCINTR::AttributeList::hasAttributeAtIndex(
            Attrs, AttrIndex, VCFunctionMD::VCArgumentKind)) {
      VCINTR::AttributeList::getAttributeAtIndex(Attrs, AttrIndex,
                                                 VCFunctionMD::VCArgumentKind)
          .getValueAsString()
          .getAsInteger(0, ArgKind);
      dropAttributeAtIndex(F, AttrIndex, VCFunctionMD::VCArgumentKind);
    }
    if (VCINTR::AttributeList::hasAttributeAtIndex(
            Attrs, AttrIndex, VCFunctionMD::VCArgumentIOKind)) {
      VCINTR::AttributeList::getAttributeAtIndex(Attrs, AttrIndex,
                                                 VCFunctionMD::VCArgumentIOKind)
          .getValueAsString()
          .getAsInteger(0, ArgIOKind);
      dropAttributeAtIndex(F, AttrIndex, VCFunctionMD::VCArgumentIOKind);
    }
    if (VCINTR::AttributeList::hasAttributeAtIndex(
            Attrs, AttrIndex, VCFunctionMD::VCArgumentDesc)) {
      ArgDesc = VCINTR::AttributeList::getAttributeAtIndex(
                    Attrs, AttrIndex, VCFunctionMD::VCArgumentDesc)
                    .getValueAsString()
                    .str();
      dropAttributeAtIndex(F, AttrIndex, VCFunctionMD::VCArgumentDesc);
    }
    ArgKinds.push_back(
        llvm::ValueAsMetadata::get(llvm::ConstantInt::get(I32Ty, ArgKind)));
    ArgIOKinds.push_back(
        llvm::ValueAsMetadata::get(llvm::ConstantInt::get(I32Ty, ArgIOKind)));
    ArgDescs.push_back(llvm::MDString::get(Context, ArgDesc));
  }

  if (VCINTR::AttributeList::hasFnAttr(Attrs, VCFunctionMD::VCSLMSize)) {
    VCINTR::AttributeList::getAttributeAtIndex(
        Attrs, AttributeList::FunctionIndex, VCFunctionMD::VCSLMSize)
        .getValueAsString()
        .getAsInteger(0, SLMSize);
    dropFnAttribute(F, VCFunctionMD::VCSLMSize);
  }

  if (VCINTR::AttributeList::hasFnAttr(Attrs,
                                       VCFunctionMD::VCNamedBarrierCount)) {
    VCINTR::AttributeList::getAttributeAtIndex(
        Attrs, AttributeList::FunctionIndex, VCFunctionMD::VCNamedBarrierCount)
        .getValueAsString()
        .getAsInteger(0, NBarrierCnt);
    dropFnAttribute(F, VCFunctionMD::VCNamedBarrierCount);
  }

  auto KernelMD = std::vector<llvm::Metadata *>();
  KernelMD.push_back(FunctionRef);
  KernelMD.push_back(llvm::MDString::get(Context, KernelName));
  KernelMD.push_back(llvm::MDNode::get(Context, ArgKinds));
  KernelMD.push_back(ConstantAsMetadata::get(ConstantInt::get(I32Ty, SLMSize)));
  KernelMD.push_back(
      ConstantAsMetadata::get(ConstantInt::get(I32Ty, ArgOffset)));
  KernelMD.push_back(llvm::MDNode::get(Context, ArgIOKinds));
  KernelMD.push_back(llvm::MDNode::get(Context, ArgDescs));
  KernelMD.push_back(
      ConstantAsMetadata::get(ConstantInt::get(I32Ty, NBarrierCnt)));
  NamedMDNode *KernelMDs =
      F.getParent()->getOrInsertNamedMetadata(FunctionMD::GenXKernels);
  llvm::MDNode *Node = MDNode::get(F.getContext(), KernelMD);
  KernelMDs->addOperand(Node);

  return true;
}

bool GenXSPIRVReaderAdaptorImpl::runOnFunction(Function &F) {
  if (!processVCFunctionAttributes(F))
    return true;

  processVCKernelAttributes(F);
  return true;
}

//-----------------------------------------------------------------------------
// New PM support
//-----------------------------------------------------------------------------
PreservedAnalyses llvm::GenXSPIRVReaderAdaptor::run(Module &M,
                                                    ModuleAnalysisManager &) {
  GenXSPIRVReaderAdaptorImpl Impl;

  if (!Impl.run(M))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

//-----------------------------------------------------------------------------
// Legacy PM support
//-----------------------------------------------------------------------------
namespace {
class GenXSPIRVReaderAdaptorLegacy final : public ModulePass {
public:
  static char ID;

public:
  explicit GenXSPIRVReaderAdaptorLegacy() : ModulePass(ID) {}

  llvm::StringRef getPassName() const override {
    return GenXSPIRVReaderAdaptor::getArgString();
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnModule(Module &M) override;
}; // class GenXSPIRVReaderAdaptorLegacy

} // namespace

char GenXSPIRVReaderAdaptorLegacy::ID = 0;

INITIALIZE_PASS(GenXSPIRVReaderAdaptorLegacy,
                GenXSPIRVReaderAdaptor::getArgString(),
                GenXSPIRVReaderAdaptor::getArgString(), false, false)

ModulePass *llvm::createGenXSPIRVReaderAdaptorPass() {
  return new GenXSPIRVReaderAdaptorLegacy();
}

void GenXSPIRVReaderAdaptorLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
}

bool GenXSPIRVReaderAdaptorLegacy::runOnModule(Module &M) {
  GenXSPIRVReaderAdaptorImpl impl;
  return impl.run(M);
}

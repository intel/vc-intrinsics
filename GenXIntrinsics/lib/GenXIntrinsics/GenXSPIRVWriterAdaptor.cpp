/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This pass converts metadata to SPIRV format from whichever used in frontend.

#include "AdaptorsCommon.h"
#include "GenXSingleElementVectorUtil.h"

#include "llvm/GenXIntrinsics/GenXSPIRVWriterAdaptor.h"
#include "llvm/GenXIntrinsics/GenXIntrinsics.h"
#include "llvm/GenXIntrinsics/GenXMetadata.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Process.h"

#include "llvmVCWrapper/IR/Attributes.h"
#include "llvmVCWrapper/IR/DerivedTypes.h"
#include "llvmVCWrapper/IR/Function.h"
#include "llvmVCWrapper/IR/GlobalValue.h"

using namespace llvm;
using namespace genx;

namespace {

class GenXSPIRVWriterAdaptorImpl final {
private:
  bool RewriteTypes = true;
  bool RewriteSingleElementVectors = true;

public:
  explicit GenXSPIRVWriterAdaptorImpl(bool RewriteTypesIn,
                                      bool RewriteSingleElementVectorsIn)
      : RewriteTypes(RewriteTypesIn),
        RewriteSingleElementVectors(RewriteSingleElementVectorsIn) {
    overrideOptionsWithEnv();
  }

  bool run(Module &M);

private:
  // This function overrides options with environment variables
  // It is used for debugging.
  void overrideOptionsWithEnv() {
    auto RewriteSEVOpt = llvm::sys::Process::GetEnv("GENX_REWRITE_SEV");
    if (RewriteSEVOpt)
      RewriteSingleElementVectors = RewriteSEVOpt.getValue() == "1";
  }

  bool runOnFunction(Function &F);
};

} // namespace

// Get some pointer to global address space.
static Type *getGlobalPtrType(LLVMContext &Ctx) {
  return PointerType::get(Type::getInt8Ty(Ctx), SPIRVParams::SPIRVGlobalAS);
}

// Get some opaque structure pointer to global address space. This is
// how OCL/SPIRV types are implemented in clang/SPIRV Translator.
static Type *getOpaquePtrType(Module *M, StringRef Name,
                              unsigned AddressSpace) {
  StructType *STy = VCINTR::getTypeByName(M, Name);
  if (!STy)
    STy = StructType::create(M->getContext(), Name);
  return PointerType::get(STy, AddressSpace);
}

static Type *getSamplerType(Module *M) {
  std::string Name = OCLTypes::TypePrefix;
  Name += OCLTypes::Sampler;
  Name += CommonTypes::TypeSuffix;

  return getOpaquePtrType(M, Name,
                          getOpaqueTypeAddressSpace(SPIRVType::Sampler));
}

// Add access qualifiers and type suffix to type name.
static void addCommonTypesPostfix(std::string &Name, AccessType Acc) {
  switch (Acc) {
  case AccessType::ReadOnly:
    Name += CommonTypes::ReadOnly;
    break;
  case AccessType::WriteOnly:
    Name += CommonTypes::WriteOnly;
    break;
  case AccessType::ReadWrite:
    Name += CommonTypes::ReadWrite;
    break;
  }

  Name += CommonTypes::TypeSuffix;
}

// Get or create image type from spirv type descriptor. Name encoding
// is the same as in clang and it is required by SPIRV translator.
static Type *getImageType(SPIRVArgDesc Desc, Module *M) {
  std::string Name = OCLTypes::TypePrefix;
  Name += OCLTypes::Image;
  switch (Desc.Ty) {
  case SPIRVType::Image1d:
    Name += OCLTypes::Dim1d;
    break;
  case SPIRVType::Image1dArray:
    Name += OCLTypes::Dim1dArray;
    break;
  case SPIRVType::Image1dBuffer:
    Name += OCLTypes::Dim1dBuffer;
    break;
  case SPIRVType::Image2d:
    Name += OCLTypes::Dim2d;
    break;
  case SPIRVType::Image2dArray:
    Name += OCLTypes::Dim2dArray;
    break;
  case SPIRVType::Image3d:
    Name += OCLTypes::Dim3d;
    break;
  default:
    llvm_unreachable("Unexpected spirv type for image");
  }

  addCommonTypesPostfix(Name, Desc.Acc);

  return getOpaquePtrType(M, Name, getOpaqueTypeAddressSpace(Desc.Ty));
}

// Get or create buffer type with given access qualifier.
static Type *getBufferType(AccessType Acc, Module *M) {
  std::string Name = IntelTypes::TypePrefix;
  Name += IntelTypes::Buffer;

  addCommonTypesPostfix(Name, Acc);

  return getOpaquePtrType(M, Name,
                          getOpaqueTypeAddressSpace(SPIRVType::Buffer));
}

// Sampler and surface arguments require opaque types that will be
// translated in native SPIRV types.
static Type *getOpaqueType(SPIRVArgDesc Desc, Module *M) {
  switch (Desc.Ty) {
  case SPIRVType::Sampler:
    return getSamplerType(M);
  case SPIRVType::Buffer:
    return getBufferType(Desc.Acc, M);
  default:
    return getImageType(Desc, M);
  }
}

// Convert spirv type descriptor to LLVM type that later will be
// handled by SPIRV translator. Mostly relying on implementation of
// clang/SPIRV translator to handle image/sampler types.
static Type *getArgTypeFromDesc(SPIRVArgDesc Desc, Argument &Arg) {
  std::string TypeName;
  switch (Desc.Ty) {
  case SPIRVType::Pointer:
    return getGlobalPtrType(Arg.getContext());
  case SPIRVType::Other:
  case SPIRVType::None:
    return Arg.getType();
  default:
    return getOpaqueType(Desc, Arg.getParent()->getParent());
  }
}

static Function *
transformKernelSignature(Function &F, const std::vector<SPIRVArgDesc> &Descs) {
  SmallVector<Type *, 8> NewParams;
  std::transform(Descs.begin(), Descs.end(), F.arg_begin(),
                 std::back_inserter(NewParams),
                 [](SPIRVArgDesc Desc, Argument &Arg) {
                   return getArgTypeFromDesc(Desc, Arg);
                 });

  assert(!F.isVarArg() && "Kernel cannot be vararg");
  auto *NewFTy = FunctionType::get(F.getReturnType(), NewParams, false);
  auto *NewF = VCINTR::Function::Create(
      NewFTy, F.getLinkage(), VCINTR::GlobalValue::getAddressSpace(F));
  NewF->copyAttributesFrom(&F);
  NewF->takeName(&F);
  NewF->copyMetadata(&F, 0);
  NewF->setComdat(F.getComdat());

  // Remove no more needed attributes.
  for (int i = 0, e = Descs.size(); i != e; ++i) {
    if (Descs[i].Ty == SPIRVType::None)
      continue;

    NewF->removeParamAttr(i, VCFunctionMD::VCArgumentKind);
    NewF->removeParamAttr(i, VCFunctionMD::VCArgumentDesc);
  }

  legalizeParamAttributes(NewF);

  return NewF;
}

// Replace old arguments with new ones generating conversion
// intrinsics for types that were changed.
static Instruction *rewriteArgumentUses(Argument &OldArg, Argument &NewArg) {
  NewArg.takeName(&OldArg);

  Type *OldTy = OldArg.getType();
  Type *NewTy = NewArg.getType();
  if (OldTy == NewTy) {
    OldArg.replaceAllUsesWith(&NewArg);
    return nullptr;
  }

  Module *M = OldArg.getParent()->getParent();
  Function *ConvFn = GenXIntrinsic::getGenXDeclaration(
      M, GenXIntrinsic::genx_address_convert, {OldTy, NewTy});
  auto *Conv = CallInst::Create(ConvFn, {&NewArg});
  OldArg.replaceAllUsesWith(Conv);
  return Conv;
}

// Parse argument desc.
// String can contain arbitrary words, some of which have special meaning.
// Special words are listed in ArgDesc namespace and correspond to SPIRVType
// and AccessType.
// If no special words were encountered, default to other general types.
static SPIRVArgDesc parseArgDesc(StringRef Desc) {
  SmallVector<StringRef, 2> Tokens;
  Desc.split(Tokens, /*Separator=*/' ', /*MaxSplit=*/-1, /*KeepEmpty=*/false);

  // Scan tokens until end or required info is found.
  Optional<AccessType> AccTy;
  Optional<SPIRVType> Ty;
  for (StringRef Tok : Tokens) {
    if (!Ty) {
      Ty = StringSwitch<Optional<SPIRVType>>(Tok)
               .Case(ArgDesc::Buffer, SPIRVType::Buffer)
               .Case(ArgDesc::Image1d, SPIRVType::Image1d)
               .Case(ArgDesc::Image1dArray, SPIRVType::Image1dArray)
               .Case(ArgDesc::Image1dBuffer, SPIRVType::Image1dBuffer)
               .Case(ArgDesc::Image2d, SPIRVType::Image2d)
               .Case(ArgDesc::Image2dArray, SPIRVType::Image2dArray)
               .Case(ArgDesc::Image3d, SPIRVType::Image3d)
               .Case(ArgDesc::SVM, SPIRVType::Pointer)
               .Case(ArgDesc::Sampler, SPIRVType::Sampler)
               .Default(None);
    }

    if (!AccTy) {
      AccTy = StringSwitch<Optional<AccessType>>(Tok)
                  .Case(ArgDesc::ReadOnly, AccessType::ReadOnly)
                  .Case(ArgDesc::WriteOnly, AccessType::WriteOnly)
                  .Case(ArgDesc::ReadWrite, AccessType::ReadWrite)
                  .Default(None);
    }

    if (Ty && AccTy)
      break;
  }

  // Default to other types.
  if (!Ty)
    return {SPIRVType::Other};

  // Default to read write access qualifier.
  if (!AccTy)
    AccTy = AccessType::ReadWrite;

  return {Ty.getValue(), AccTy.getValue()};
}

// General arguments can be either pointers or any other types.
static SPIRVArgDesc analyzeGeneralArg(StringRef Desc) {
  SPIRVArgDesc SPVDesc = parseArgDesc(Desc);
  switch (SPVDesc.Ty) {
  case SPIRVType::Other:
  case SPIRVType::Pointer:
    return SPVDesc;
  // Default to other types since there are cases where people write
  // strange things.
  default:
    return {SPIRVType::Other};
  }
}

static SPIRVArgDesc analyzeSurfaceArg(StringRef Desc) {
  SPIRVArgDesc SPVDesc = parseArgDesc(Desc);
  switch (SPVDesc.Ty) {
  case SPIRVType::Buffer:
  case SPIRVType::Image1d:
  case SPIRVType::Image1dArray:
  case SPIRVType::Image1dBuffer:
  case SPIRVType::Image2d:
  case SPIRVType::Image2dArray:
  case SPIRVType::Image3d:
    return SPVDesc;
  // CMRT does not require to annotate arguments.
  // Default to read_write buffer_t currently.
  case SPIRVType::Other:
    return {SPIRVType::Buffer};
  default:
    llvm_unreachable("Unexpected descs on surface argument");
  }
}

// Redundant analysis for sampler. Sampler arg kind can
// have "sampler_t" annotation.
static SPIRVArgDesc analyzeSamplerArg(StringRef Desc) {
  SPIRVArgDesc SPVDesc = parseArgDesc(Desc);
  switch (SPVDesc.Ty) {
  // sampler_t annotation.
  case SPIRVType::Sampler:
  // CMRT does not require to annotate arguments.
  case SPIRVType::Other:
    return {SPIRVType::Sampler};
  default:
    llvm_unreachable("Unexpected descs on sampler argument");
  }
}

// Convert arg kind and arg desc to spirv type decriptor. Requires
// parsing of arg desc.
static SPIRVArgDesc analyzeArgumentAttributes(ArgKind Kind, StringRef Desc) {
  switch (Kind) {
  case ArgKind::General:
    return analyzeGeneralArg(Desc);
  case ArgKind::Sampler:
    return analyzeSamplerArg(Desc);
  case ArgKind::Surface:
    return analyzeSurfaceArg(Desc);
  }
  return {SPIRVType::None};
}

// Extract ArgKind from VCArgumentKind attribute.
// In presence of implicit arguments (that is temporary),
// value can be out of listed in ArgKind enum.
// Such values are not processed later.
// Return None if there is no such attribute.
static Optional<ArgKind> extractArgumentKind(const Argument &Arg) {
  const Function *F = Arg.getParent();
  const AttributeList Attrs = F->getAttributes();
  if (!Attrs.hasParamAttr(Arg.getArgNo(), VCFunctionMD::VCArgumentKind))
    return None;

  const Attribute Attr =
      Attrs.getParamAttr(Arg.getArgNo(), VCFunctionMD::VCArgumentKind);
  unsigned AttrVal = {};
  const bool Conv = Attr.getValueAsString().getAsInteger(0, AttrVal);
  assert(!Conv && "Expected integer value as arg kind");
  // TODO: add some sanity check that the value can be casted to ArgKind
  return static_cast<ArgKind>(AttrVal);
}

// Extract string desc from VCArgumentDesc attribute.
static StringRef extractArgumentDesc(const Argument &Arg) {
  const Function *F = Arg.getParent();
  const AttributeList Attrs = F->getAttributes();
  return Attrs.getParamAttr(Arg.getArgNo(), VCFunctionMD::VCArgumentDesc)
      .getValueAsString();
}

// Get SPIRV type and access qualifier of kernel argument
// using its corresponding attributes.
// Default to None if no information available.
static SPIRVArgDesc analyzeKernelArg(const Argument &Arg) {
  if (auto Kind = extractArgumentKind(Arg)) {
    const StringRef Desc = extractArgumentDesc(Arg);
    return analyzeArgumentAttributes(Kind.getValue(), Desc);
  }

  return {SPIRVType::None};
}

static std::vector<SPIRVArgDesc> analyzeKernelArguments(const Function &F) {
  std::vector<SPIRVArgDesc> Descs;
  std::transform(F.arg_begin(), F.arg_end(), std::back_inserter(Descs),
                 [](const Argument &Arg) { return analyzeKernelArg(Arg); });
  return Descs;
}

// Rewrite function if it has SPIRV types as parameters.
// Function
//  define spir_kernel @foo(i32 "VCArgumentKind"="2" "VCArgumentDesc"="image2d_t
//  read_write" %im) {
//    ...
//  }
// will be changed to
//  define spir_kernel @foo(%opencl.image2d_rw_t addrspace(1)* %im) {
//    %conv = call @llvm.genx.address.convert(%im)
//   ...
//  }
// Parameters that are not part of public interface (implicit arguments)
// are not converted. Currently there are generated by old cmc. They are not
// needed for IGC VC backend.
static void rewriteKernelArguments(Function &F) {
  // Do not touch callable kernels at this moment. Other kernels
  // should have no uses.
  if (!F.use_empty())
    return;

  std::vector<SPIRVArgDesc> ArgDescs = analyzeKernelArguments(F);

  Function *NewF = transformKernelSignature(F, ArgDescs);
  F.getParent()->getFunctionList().insert(F.getIterator(), NewF);
  NewF->getBasicBlockList().splice(NewF->begin(), F.getBasicBlockList());

  Instruction *InsPt = &NewF->getEntryBlock().front();
  for (auto ArgPair : llvm::zip(F.args(), NewF->args())) {
    Instruction *Convert =
        rewriteArgumentUses(std::get<0>(ArgPair), std::get<1>(ArgPair));
    if (Convert)
      Convert->insertBefore(InsPt);
  }

  F.eraseFromParent();
}

// Rewrite kernels from VC representation to SPIRV
// with different types as incoming parameters.
static void rewriteKernelsTypes(Module &M) {
  SmallVector<Function *, 4> Kernels;
  std::transform(M.begin(), M.end(), std::back_inserter(Kernels),
                 [](Function &F) { return &F; });
  for (auto *F : Kernels)
    if (F->getCallingConv() == CallingConv::SPIR_KERNEL)
      rewriteKernelArguments(*F);
}

bool GenXSPIRVWriterAdaptorImpl::run(Module &M) {
  auto TargetTriple = StringRef(M.getTargetTriple());
  if (TargetTriple.startswith("genx")) {
    if (TargetTriple.startswith("genx32"))
      M.setTargetTriple("spir");
    else
      M.setTargetTriple("spir64");
  }

  for (auto &&GV : M.getGlobalList()) {
    GV.addAttribute(VCModuleMD::VCGlobalVariable);
    if (GV.hasAttribute(FunctionMD::GenXVolatile))
      GV.addAttribute(VCModuleMD::VCVolatile);
    if (GV.hasAttribute(FunctionMD::GenXByteOffset)) {
      auto Offset =
          GV.getAttribute(FunctionMD::GenXByteOffset).getValueAsString();
      GV.addAttribute(VCModuleMD::VCByteOffset, Offset);
    }
  }


  for (auto &&F : M)
      runOnFunction(F);

  // Old metadata is not needed anymore at this point.
  if (auto *MD = M.getNamedMetadata(FunctionMD::GenXKernels))
    M.eraseNamedMetadata(MD);

  if (RewriteTypes)
    rewriteKernelsTypes(M);

  if (RewriteSingleElementVectors)
    rewriteSingleElementVectors(M);

  return true;
}

bool GenXSPIRVWriterAdaptorImpl::runOnFunction(Function &F) {
  if (F.isIntrinsic() && !GenXIntrinsic::isGenXIntrinsic(&F))
    return true;
  F.addFnAttr(VCFunctionMD::VCFunction);

  auto Attrs = F.getAttributes();
  if (VCINTR::AttributeList::hasFnAttr(Attrs, FunctionMD::CMStackCall)) {
    F.addFnAttr(VCFunctionMD::VCStackCall);
  }

  if (VCINTR::AttributeList::hasFnAttr(Attrs, FunctionMD::CMCallable)) {
    F.addFnAttr(VCFunctionMD::VCCallable);
  }

  if (VCINTR::AttributeList::hasFnAttr(Attrs, FunctionMD::CMEntry)) {
    F.addFnAttr(VCFunctionMD::VCFCEntry);
  }

  if (VCINTR::AttributeList::hasFnAttr(Attrs, FunctionMD::CMGenxSIMT)) {
    auto SIMTMode = StringRef();
    SIMTMode = VCINTR::AttributeList::getAttributeAtIndex(
                   Attrs, AttributeList::FunctionIndex, FunctionMD::CMGenxSIMT)
                   .getValueAsString();
    F.addFnAttr(VCFunctionMD::VCSIMTCall, SIMTMode);
  }

  auto &&Context = F.getContext();
  if (VCINTR::AttributeList::hasFnAttr(Attrs, FunctionMD::CMFloatControl)) {
    auto FloatControl = unsigned(0);
    VCINTR::AttributeList::getAttributeAtIndex(
        Attrs, AttributeList::FunctionIndex, FunctionMD::CMFloatControl)
        .getValueAsString()
        .getAsInteger(0, FloatControl);

    auto Attr = Attribute::get(Context, VCFunctionMD::VCFloatControl,
                               std::to_string(FloatControl));
    VCINTR::Function::addAttributeAtIndex(F, AttributeList::FunctionIndex,
                                          Attr);
  }

  auto *KernelMDs = F.getParent()->getNamedMetadata(FunctionMD::GenXKernels);
  if (!KernelMDs)
    return true;

  if (VCINTR::AttributeList::hasFnAttr(Attrs, FunctionMD::OCLRuntime)) {
    auto SIMDSize = unsigned(0);
    VCINTR::AttributeList::getAttributeAtIndex(
        Attrs, AttributeList::FunctionIndex, FunctionMD::OCLRuntime)
        .getValueAsString()
        .getAsInteger(0, SIMDSize);
    auto SizeMD = ConstantAsMetadata::get(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), SIMDSize));
    F.setMetadata(SPIRVParams::SPIRVSIMDSubgroupSize,
                  MDNode::get(Context, SizeMD));
  }

  auto *KernelMD = GetOldStyleKernelMD(F);
  if (!KernelMD)
    return true;

  F.setCallingConv(CallingConv::SPIR_KERNEL);

  auto MDName =
      cast<MDString>(KernelMD->getOperand(KernelMDOp::Name).get())->getString();
  if (MDName != F.getName())
    F.setName(MDName);

  if (KernelMD->getNumOperands() > KernelMDOp::ArgKinds) {
    if (auto *KindsNode =
            dyn_cast<MDNode>(KernelMD->getOperand(KernelMDOp::ArgKinds))) {
      for (unsigned ArgNo = 0, e = KindsNode->getNumOperands(); ArgNo != e;
           ++ArgNo) {
        if (auto *VM = dyn_cast<ValueAsMetadata>(KindsNode->getOperand(ArgNo)))
          if (auto *V = dyn_cast<ConstantInt>(VM->getValue())) {
            auto ArgKind = V->getZExtValue();
            auto Attr = Attribute::get(Context, VCFunctionMD::VCArgumentKind,
                                       std::to_string(ArgKind));
            VCINTR::Function::addAttributeAtIndex(F, ArgNo + 1, Attr);
          }
      }
    }
  }

  if (KernelMD->getNumOperands() > KernelMDOp::SLMSize) {
    if (auto *VM = dyn_cast<ValueAsMetadata>(
            KernelMD->getOperand(KernelMDOp::SLMSize)))
      if (auto *V = dyn_cast<ConstantInt>(VM->getValue())) {
        auto SLMSize = V->getZExtValue();
        auto Attr = Attribute::get(Context, VCFunctionMD::VCSLMSize,
                                   std::to_string(SLMSize));
        VCINTR::Function::addAttributeAtIndex(F, AttributeList::FunctionIndex,
                                              Attr);
      }
  }

  if (KernelMD->getNumOperands() > KernelMDOp::ArgIOKinds) {
    if (auto *KindsNode =
            dyn_cast<MDNode>(KernelMD->getOperand(KernelMDOp::ArgIOKinds))) {
      for (unsigned ArgNo = 0, e = KindsNode->getNumOperands(); ArgNo != e;
           ++ArgNo) {
        if (auto *VM = dyn_cast<ValueAsMetadata>(KindsNode->getOperand(ArgNo)))
          if (auto *V = dyn_cast<ConstantInt>(VM->getValue())) {
            auto ArgKind = V->getZExtValue();
            auto Attr = Attribute::get(Context, VCFunctionMD::VCArgumentIOKind,
                                       std::to_string(ArgKind));
            VCINTR::Function::addAttributeAtIndex(F, ArgNo + 1, Attr);
          }
      }
    }
  }

  if (KernelMD->getNumOperands() > KernelMDOp::ArgTypeDescs) {
    if (auto Node =
            dyn_cast<MDNode>(KernelMD->getOperand(KernelMDOp::ArgTypeDescs))) {
      for (unsigned ArgNo = 0, e = Node->getNumOperands(); ArgNo != e;
           ++ArgNo) {
        if (auto *MS = dyn_cast<MDString>(Node->getOperand(ArgNo))) {
          auto &&Desc = MS->getString();
          auto Attr =
              Attribute::get(Context, VCFunctionMD::VCArgumentDesc, Desc);
          VCINTR::Function::addAttributeAtIndex(F, ArgNo + 1, Attr);
        }
      }
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
// New PM support
//-----------------------------------------------------------------------------
PreservedAnalyses llvm::GenXSPIRVWriterAdaptor::run(Module &M,
                                                    ModuleAnalysisManager &) {
  GenXSPIRVWriterAdaptorImpl Impl(RewriteTypes, RewriteSingleElementVectors);

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
class GenXSPIRVWriterAdaptorLegacy final : public ModulePass {
public:
  static char ID;

  bool RewriteTypes = true;
  bool RewriteSingleElementVectors = true;

public:
  explicit GenXSPIRVWriterAdaptorLegacy() : ModulePass(ID) {}
  explicit GenXSPIRVWriterAdaptorLegacy(bool RewriteTypesIn,
                                        bool RewriteSingleElementVectorsIn)
      : ModulePass(ID), RewriteTypes(RewriteTypesIn),
        RewriteSingleElementVectors(RewriteSingleElementVectorsIn) {}

  llvm::StringRef getPassName() const override {
    return GenXSPIRVWriterAdaptor::getArgString();
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnModule(Module &M) override;
};

} // namespace

char GenXSPIRVWriterAdaptorLegacy::ID = 0;

INITIALIZE_PASS(GenXSPIRVWriterAdaptorLegacy,
                GenXSPIRVWriterAdaptor::getArgString(),
                GenXSPIRVWriterAdaptor::getArgString(), false, false)

ModulePass *
llvm::createGenXSPIRVWriterAdaptorPass(bool RewriteTypes,
                                       bool RewriteSingleElementVectors) {
  return new GenXSPIRVWriterAdaptorLegacy(RewriteTypes,
                                          RewriteSingleElementVectors);
}

void GenXSPIRVWriterAdaptorLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
}

bool GenXSPIRVWriterAdaptorLegacy::runOnModule(Module &M) {
  GenXSPIRVWriterAdaptorImpl Impl(RewriteTypes, RewriteSingleElementVectors);
  return Impl.run(M);
}

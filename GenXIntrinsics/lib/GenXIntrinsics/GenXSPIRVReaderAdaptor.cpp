/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

///
/// GenXSPIRVReaderAdaptor
/// ---------------------------
/// This pass converts metadata from SPIRV format to whichever used in backend

#include "AdaptorsCommon.h"

#include "llvm/GenXIntrinsics/GenXIntrinsics.h"
#include "llvm/GenXIntrinsics/GenXMetadata.h"
#include "llvm/GenXIntrinsics/GenXSPIRVReaderAdaptor.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#include "llvmVCWrapper/IR/Function.h"
#include "llvmVCWrapper/IR/GlobalValue.h"

using namespace llvm;
using namespace genx;

namespace {

class GenXSPIRVReaderAdaptor final : public ModulePass {
public:
  static char ID;
  explicit GenXSPIRVReaderAdaptor() : ModulePass(ID) {}
  llvm::StringRef getPassName() const override {
    return "GenX SPIRVReader Adaptor";
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnModule(Module &M) override;

private:
  bool runOnFunction(Function &F);
};

} // namespace

char GenXSPIRVReaderAdaptor::ID = 0;

INITIALIZE_PASS_BEGIN(GenXSPIRVReaderAdaptor, "GenXSPIRVReaderAdaptor",
                      "GenXSPIRVReaderAdaptor", false, false)
INITIALIZE_PASS_END(GenXSPIRVReaderAdaptor, "GenXSPIRVReaderAdaptor",
                    "GenXSPIRVReaderAdaptor", false, false)

ModulePass *llvm::createGenXSPIRVReaderAdaptorPass() {
  return new GenXSPIRVReaderAdaptor();
}

void GenXSPIRVReaderAdaptor::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
}

// Check that Str starts with Part.
// If true, drop Part from Str and return result,
// else return None.
static Optional<StringRef> consumeStringPart(StringRef Str, StringRef Part) {
  if (!Str.startswith(Part))
    return None;

  return Str.drop_front(Part.size());
}

static std::pair<SPIRVType, StringRef> parseImageDim(StringRef TyName) {
  // Greedy match: 1d_buffer first.
  if (auto Rest = consumeStringPart(TyName, OCLTypes::Dim1dBuffer))
    return {SPIRVType::Image1dBuffer, Rest.getValue()};

  if (auto Rest = consumeStringPart(TyName, OCLTypes::Dim1d))
    return {SPIRVType::Image1d, Rest.getValue()};

  if (auto Rest = consumeStringPart(TyName, OCLTypes::Dim2d))
    return {SPIRVType::Image2d, Rest.getValue()};

  if (auto Rest = consumeStringPart(TyName, OCLTypes::Dim3d))
    return {SPIRVType::Image3d, Rest.getValue()};

  llvm_unreachable("Unexpected image dimensionality");
}

static std::pair<AccessType, StringRef> parseAccessQualifier(StringRef TyName) {
  if (auto Rest = consumeStringPart(TyName, CommonTypes::ReadOnly))
    return {AccessType::ReadOnly, Rest.getValue()};

  if (auto Rest = consumeStringPart(TyName, CommonTypes::WriteOnly))
    return {AccessType::WriteOnly, Rest.getValue()};

  if (auto Rest = consumeStringPart(TyName, CommonTypes::ReadWrite))
    return {AccessType::ReadWrite, Rest.getValue()};

  llvm_unreachable("Unexpected image access modifier");
}

static SPIRVArgDesc parseImageType(StringRef TyName) {
  Optional<StringRef> MaybeName = consumeStringPart(TyName, OCLTypes::Image);
  assert(MaybeName && "Unexpected opencl type");
  TyName = MaybeName.getValue();

  SPIRVType ImageType;
  std::tie(ImageType, TyName) = parseImageDim(TyName);
  AccessType AccType;
  std::tie(AccType, TyName) = parseAccessQualifier(TyName);
  assert(TyName == CommonTypes::TypeSuffix && "Bad image type");
  return {ImageType, AccType};
}

static Optional<SPIRVArgDesc> parseBufferType(StringRef TyName) {
  Optional<StringRef> MaybeName =
      consumeStringPart(TyName, IntelTypes::TypePrefix);
  if (!MaybeName)
    return None;

  MaybeName = consumeStringPart(MaybeName.getValue(), IntelTypes::Buffer);
  if (!MaybeName)
    return None;

  // Now assume that buffer type is correct.
  AccessType AccType;
  StringRef Suffix;
  std::tie(AccType, Suffix) = parseAccessQualifier(MaybeName.getValue());
  assert(Suffix == CommonTypes::TypeSuffix && "Bad buffer type");
  return SPIRVArgDesc{SPIRVType::Buffer, AccType};
}

static Optional<SPIRVArgDesc> parseOCLType(StringRef TyName) {
  Optional<StringRef> MaybeName =
      consumeStringPart(TyName, OCLTypes::TypePrefix);
  if (!MaybeName)
    return None;

  TyName = MaybeName.getValue();
  // Sampler type.
  if (auto Rest = consumeStringPart(TyName, OCLTypes::Sampler)) {
    assert(Rest.getValue() == CommonTypes::TypeSuffix && "Bad sampler type");
    return {SPIRVType::Sampler};
  }

  // Images are the rest.
  return parseImageType(TyName);
}

// Parse opaque type name.
// Ty -> "opencl." OCLTy | "intel.buffer" Acc "_t"
// OCLTy -> "sampler_t" | ImageTy
// ImageTy -> "image" Dim Acc "_t"
// Dim -> "1d" | "1d_buffer" | "2d" | "3d"
// Acc -> "_ro" | "_wo" | "_rw"
// Assume that "opencl." and "intel.buffer" types are well-formed.
static Optional<SPIRVArgDesc> parseOpaqueType(StringRef TyName) {
  if (auto MaybeBuffer = parseBufferType(TyName))
    return MaybeBuffer.getValue();

  return parseOCLType(TyName);
}

static SPIRVArgDesc analyzeKernelArg(const Argument &Arg) {
  const Function *F = Arg.getParent();
  // If there is vc attribute, then no conversion is needed.
  if (F->getAttributes().hasParamAttr(Arg.getArgNo(),
                                      VCFunctionMD::VCArgumentKind))
    return {SPIRVType::None};

  Type *Ty = Arg.getType();
  // Not a pointer means that it is general argument without annotation.
  if (!isa<PointerType>(Ty))
    return {SPIRVType::Other};

  Type *PointeeTy = cast<PointerType>(Ty)->getElementType();
  // Not a pointer to struct, cannot be sampler or image.
  if (!isa<StructType>(PointeeTy))
    return {SPIRVType::Pointer};

  auto *StrTy = cast<StructType>(PointeeTy);
  // Pointer to literal structure, cannot be sampler or image.
  // (is this case possible in SPIRV translator?)
  if (!StrTy->hasName())
    return {SPIRVType::Pointer};

  if (auto MaybeDesc = parseOpaqueType(StrTy->getName()))
    return MaybeDesc.getValue();

  // If nothing was matched then it is simple pointer.
  return {SPIRVType::Pointer};
}

static std::vector<SPIRVArgDesc> analyzeKernelArguments(Function &F) {
  std::vector<SPIRVArgDesc> Descs;
  std::transform(F.arg_begin(), F.arg_end(), std::back_inserter(Descs),
                 [](const Argument &Arg) { return analyzeKernelArg(Arg); });
  return Descs;
}

// SPIRV arguments converted to old style with address convert intrinsic.
static Instruction *getArgConvIntrinsic(Argument &Arg) {
  assert(Arg.hasOneUse() == 1 &&
         "Rewritten signature argument should have one use!");

  User *Intr = Arg.user_back();
  assert(GenXIntrinsic::getGenXIntrinsicID(Intr) ==
             GenXIntrinsic::genx_address_convert &&
         "Expected address convert intrinsic for rewritten type");
  return cast<Instruction>(Intr);
}

static ArgKind mapSPIRVTypeToArgKind(SPIRVType Ty) {
  switch (Ty) {
  case SPIRVType::Buffer:
  case SPIRVType::Image1d:
  case SPIRVType::Image1dBuffer:
  case SPIRVType::Image2d:
  case SPIRVType::Image3d:
    return ArgKind::Surface;
  case SPIRVType::Sampler:
    return ArgKind::Sampler;
  case SPIRVType::Pointer:
  case SPIRVType::Other:
    return ArgKind::General;
  case SPIRVType::None:
  default:
    llvm_unreachable("Unexpected spirv type");
  }
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
  case SPIRVType::Image1dBuffer:
    Desc += ArgDesc::Image1dBuffer;
    break;
  case SPIRVType::Image2d:
    Desc += ArgDesc::Image2d;
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

// Only types with desc require argument rewriting.
static bool typeRequiresRewriting(SPIRVType Ty) {
  return Ty != SPIRVType::None && Ty != SPIRVType::Other;
}

// Create new empty function with restored types based on old function and
// arguments descriptors.
static Function *
transformKernelSignature(Function &F, const std::vector<SPIRVArgDesc> &Descs) {
  // Collect new kernel argument types.
  std::vector<Type *> NewTypes;
  for (Argument &Arg : F.args()) {
    const SPIRVArgDesc Desc = Descs[Arg.getArgNo()];
    if (typeRequiresRewriting(Desc.Ty))
      // Arguments that require rewriting have only one user:
      // "address convert" intrinsic with old type.
      NewTypes.push_back(getArgConvIntrinsic(Arg)->getType());
    else
      NewTypes.push_back(Arg.getType());
  }

  auto *NewFTy = FunctionType::get(F.getReturnType(), NewTypes, false);
  auto *NewF = VCINTR::Function::Create(
      NewFTy, F.getLinkage(), VCINTR::GlobalValue::getAddressSpace(F));

  // Copy function info.
  LLVMContext &Ctx = F.getContext();
  NewF->copyAttributesFrom(&F);
  NewF->takeName(&F);
  NewF->copyMetadata(&F, 0);

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

  return NewF;
}

// Rewrite function if it has SPIRV types as parameters.
// Function
//  define spir_kernel @foo(%opencl.image2d_rw_t addrspace(1)* %im) {
//    %conv = call @llvm.genx.address.convert(%im)
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

  // No uses. Fast composite is not converted in writer part for now.
  assert(F.use_empty() && "FC is not supported yet");

  Function *NewF = transformKernelSignature(F, ArgDescs);
  F.getParent()->getFunctionList().insert(F.getIterator(), NewF);
  NewF->getBasicBlockList().splice(NewF->begin(), F.getBasicBlockList());

  // Rewrite uses and delete conversion intrinsics.
  for (int i = 0, e = ArgDescs.size(); i != e; ++i) {
    Argument &OldArg = *std::next(F.arg_begin(), i);
    Argument &NewArg = *std::next(NewF->arg_begin(), i);
    NewArg.takeName(&OldArg);
    if (typeRequiresRewriting(ArgDescs[i].Ty)) {
      Instruction *Conv = getArgConvIntrinsic(OldArg);
      Conv->replaceAllUsesWith(&NewArg);
      Conv->eraseFromParent();
    } else {
      OldArg.replaceAllUsesWith(&NewArg);
    }
  }

  F.eraseFromParent();
}

// Rewrite kernels from SPIRV representation to old style VC
// integers with attributes as incoming parameters.
static void rewriteKernelsTypes(Module &M) {
  SmallVector<Function *, 4> Kernels;
  std::transform(M.begin(), M.end(), std::back_inserter(Kernels),
                 [](Function &F) { return &F; });
  for (auto *F : Kernels)
    if (F->getCallingConv() == CallingConv::SPIR_KERNEL)
      rewriteKernelArguments(*F);
}

bool GenXSPIRVReaderAdaptor::runOnModule(Module &M) {
  for (auto &&GV : M.getGlobalList()) {
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

  for (auto &&F : M)
    runOnFunction(F);

  return true;
}

bool GenXSPIRVReaderAdaptor::runOnFunction(Function &F) {
  auto Attrs = F.getAttributes();
  if (!Attrs.hasFnAttribute(VCFunctionMD::VCFunction))
    return true;

  if (Attrs.hasFnAttribute(VCFunctionMD::VCStackCall)) {
    F.addFnAttr(FunctionMD::CMStackCall);
    F.addFnAttr(Attribute::NoInline);
  }

  if (Attrs.hasFnAttribute(VCFunctionMD::VCSIMTCall)) {
    auto SIMTMode = StringRef();
    SIMTMode = Attrs
                   .getAttribute(AttributeList::FunctionIndex,
                                 VCFunctionMD::VCSIMTCall)
                   .getValueAsString();
    F.addFnAttr(FunctionMD::CMGenxSIMT, SIMTMode);
  }

  auto &&Context = F.getContext();
  if (Attrs.hasFnAttribute(VCFunctionMD::VCFloatControl)) {
    auto FloatControl = unsigned(0);
    Attrs
        .getAttribute(AttributeList::FunctionIndex,
                      VCFunctionMD::VCFloatControl)
        .getValueAsString()
        .getAsInteger(0, FloatControl);

    auto Attr = Attribute::get(Context, FunctionMD::CMFloatControl,
                               std::to_string(FloatControl));
    F.addAttribute(AttributeList::FunctionIndex, Attr);
  }

  if (auto *ReqdSubgroupSize =
          F.getMetadata(SPIRVParams::SPIRVSIMDSubgroupSize)) {
    auto SIMDSize =
        mdconst::dyn_extract<ConstantInt>(ReqdSubgroupSize->getOperand(0))
            ->getZExtValue();
    Attribute Attr = Attribute::get(Context, FunctionMD::OCLRuntime,
                                    std::to_string(SIMDSize));
    F.addAttribute(AttributeList::FunctionIndex, Attr);
  }

  if (!(F.getCallingConv() == CallingConv::SPIR_KERNEL))
    return true;
  F.addFnAttr(FunctionMD::CMGenXMain);
  F.setDLLStorageClass(llvm::GlobalVariable::DLLExportStorageClass);

  auto *FunctionRef = ValueAsMetadata::get(&F);
  auto KernelName = F.getName();
  auto ArgKinds = llvm::SmallVector<llvm::Metadata *, 8>();
  auto SLMSize = unsigned(0);
  auto ArgOffset = unsigned(0);
  auto ArgIOKinds = llvm::SmallVector<llvm::Metadata *, 8>();
  auto ArgDescs = llvm::SmallVector<llvm::Metadata *, 8>();

  llvm::Type *I32Ty = llvm::Type::getInt32Ty(Context);

  if (Attrs.hasFnAttribute(VCFunctionMD::VCSLMSize)) {
    Attrs.getAttribute(AttributeList::FunctionIndex, VCFunctionMD::VCSLMSize)
        .getValueAsString()
        .getAsInteger(0, SLMSize);
  }

  for (Function::arg_iterator I = F.arg_begin(), E = F.arg_end(); I != E; ++I) {
    auto ArgNo = I->getArgNo();
    auto ArgKind = unsigned(0);
    auto ArgIOKind = unsigned(0);
    auto ArgDesc = std::string();
    if (Attrs.hasAttribute(ArgNo + 1, VCFunctionMD::VCArgumentKind)) {
      Attrs.getAttribute(ArgNo + 1, VCFunctionMD::VCArgumentKind)
          .getValueAsString()
          .getAsInteger(0, ArgKind);
    }
    if (Attrs.hasAttribute(ArgNo + 1, VCFunctionMD::VCArgumentIOKind)) {
      Attrs.getAttribute(ArgNo + 1, VCFunctionMD::VCArgumentIOKind)
          .getValueAsString()
          .getAsInteger(0, ArgIOKind);
    }
    if (Attrs.hasAttribute(ArgNo + 1, VCFunctionMD::VCArgumentDesc)) {
      ArgDesc = Attrs.getAttribute(ArgNo + 1, VCFunctionMD::VCArgumentDesc)
                    .getValueAsString()
                    .str();
    }
    ArgKinds.push_back(
        llvm::ValueAsMetadata::get(llvm::ConstantInt::get(I32Ty, ArgKind)));
    ArgIOKinds.push_back(
        llvm::ValueAsMetadata::get(llvm::ConstantInt::get(I32Ty, ArgIOKind)));
    ArgDescs.push_back(llvm::MDString::get(Context, ArgDesc));
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
  KernelMD.push_back(ConstantAsMetadata::get(ConstantInt::get(I32Ty, 0)));

  NamedMDNode *KernelMDs =
      F.getParent()->getOrInsertNamedMetadata(FunctionMD::GenXKernels);
  llvm::MDNode *Node = MDNode::get(F.getContext(), KernelMD);
  KernelMDs->addOperand(Node);
  return true;
}

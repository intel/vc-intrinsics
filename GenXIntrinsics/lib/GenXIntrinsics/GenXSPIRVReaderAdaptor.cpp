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

#include "llvm/GenXIntrinsics/GenXSPIRVReaderAdaptor.h"
#include "llvm/GenXIntrinsics/GenXMetadata.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

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

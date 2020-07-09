/*===================== begin_copyright_notice ==================================

Copyright (c) 2017 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


======================= end_copyright_notice ==================================*/
///
/// GenXSPIRVWriterAdaptor
/// ---------------------------
/// This pass converts metadata to SPIRV format from whichever used in frontend

#include "llvm/GenXIntrinsics/GenXSPIRVWriterAdaptor.h"
#include "llvm/GenXIntrinsics/GenXIntrinsics.h"
#include "llvm/GenXIntrinsics/GenXMetadata.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

using namespace llvm;
using namespace genx;

char GenXSPIRVWriterAdaptor::ID = 0;

INITIALIZE_PASS_BEGIN(GenXSPIRVWriterAdaptor, "GenXSPIRVWriterAdaptor",
                      "GenXSPIRVWriterAdaptor", false, false)
INITIALIZE_PASS_END(GenXSPIRVWriterAdaptor, "GenXSPIRVWriterAdaptor",
                    "GenXSPIRVWriterAdaptor", false, false)

ModulePass *llvm::createGenXSPIRVWriterAdaptorPass() {
  return new GenXSPIRVWriterAdaptor();
}

void GenXSPIRVWriterAdaptor::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
}

bool GenXSPIRVWriterAdaptor::runOnModule(Module &M) {

  auto TargetTriple = StringRef(M.getTargetTriple());
  if (!M.getNamedMetadata(SPIRVParams::SPIRVMemoryModel)) {
    // TODO: remove this block when finally transferred to open source
    auto &&Context = M.getContext();
    auto AddressingModel = TargetTriple.startswith("genx64")
                               ? SPIRVParams::SPIRVAddressingModel64
                               : SPIRVParams::SPIRVAddressingModel32;
    auto *MemoryModelMD =
        M.getOrInsertNamedMetadata(SPIRVParams::SPIRVMemoryModel);
    auto ValueVec = std::vector<llvm::Metadata *>();
    ValueVec.push_back(ConstantAsMetadata::get(
        ConstantInt::get(Type::getInt32Ty(Context), AddressingModel)));
    ValueVec.push_back(ConstantAsMetadata::get(ConstantInt::get(
        Type::getInt32Ty(Context), SPIRVParams::SPIRVMemoryModelSimple)));
    MemoryModelMD->addOperand(MDNode::get(Context, ValueVec));
  }
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

  return true;
}

bool GenXSPIRVWriterAdaptor::runOnFunction(Function &F) {
  if (F.isIntrinsic() && !GenXIntrinsic::isGenXIntrinsic(&F))
    return true;
  F.addFnAttr(VCFunctionMD::VCFunction);

  auto Attrs = F.getAttributes();
  if (Attrs.hasFnAttribute(FunctionMD::CMStackCall)) {
    F.addFnAttr(VCFunctionMD::VCStackCall);
  }

  if (Attrs.hasFnAttribute(FunctionMD::CMGenxSIMT)) {
    auto SIMTMode = StringRef();
    SIMTMode =
        Attrs.getAttribute(AttributeList::FunctionIndex, FunctionMD::CMGenxSIMT)
            .getValueAsString();
    F.addFnAttr(VCFunctionMD::VCSIMTCall, SIMTMode);
  }

  auto &&Context = F.getContext();
  if (Attrs.hasFnAttribute(FunctionMD::CMFloatControl)) {
    auto FloatControl = unsigned(0);
    Attrs.getAttribute(AttributeList::FunctionIndex, FunctionMD::CMFloatControl)
        .getValueAsString()
        .getAsInteger(0, FloatControl);

    auto Attr = Attribute::get(Context, VCFunctionMD::VCFloatControl,
                               std::to_string(FloatControl));
    F.addAttribute(AttributeList::FunctionIndex, Attr);
  }

  auto *KernelMDs = F.getParent()->getNamedMetadata(FunctionMD::GenXKernels);
  if (!KernelMDs)
    return true;

  if (Attrs.hasFnAttribute(FunctionMD::OCLRuntime)) {
    auto SIMDSize = unsigned(0);
    Attrs.getAttribute(AttributeList::FunctionIndex, FunctionMD::OCLRuntime)
        .getValueAsString()
        .getAsInteger(0, SIMDSize);
    auto SizeMD = ConstantAsMetadata::get(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), SIMDSize));
    F.setMetadata(SPIRVParams::SPIRVSIMDSubgroupSize,
                  MDNode::get(Context, SizeMD));
  }

  auto *KernelMD = static_cast<MDNode *>(nullptr);
  for (unsigned I = 0, E = KernelMDs->getNumOperands(); I < E; ++I) {
    auto *Kernel = mdconst::dyn_extract<Function>(
        KernelMDs->getOperand(I)->getOperand(KernelMDOp::FunctionRef));
    if (Kernel == &F) {
      KernelMD = KernelMDs->getOperand(I);
      break;
    }
  }
  if (!KernelMD)
    return true;

  F.setCallingConv(CallingConv::SPIR_KERNEL);

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
            F.addAttribute(ArgNo + 1, Attr);
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
        F.addAttribute(AttributeList::FunctionIndex, Attr);
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
            F.addAttribute(ArgNo + 1, Attr);
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
          F.addAttribute(ArgNo + 1, Attr);
        }
      }
    }
  }

  return true;
}

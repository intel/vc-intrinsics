/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "llvm/GenXIntrinsics/GenXSimdCFLowering.h"
#include "llvm/GenXIntrinsics/GenXSPIRVReaderAdaptor.h"
#include "llvm/GenXIntrinsics/GenXSPIRVWriterAdaptor.h"

#include "llvm/PassRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// New PM support
//-----------------------------------------------------------------------------
// Add callback to create plugin pass to pass builder.
// PassArgs - arguments for pass construction, passed by value to avoid
// dangling references in callbacks.
template <typename PassT, typename... ArgsT>
static void registerModulePass(PassBuilder &PB, ArgsT... PassArgs) {
  auto Reg = [PassArgs...](StringRef Name, ModulePassManager &MPM,
                           ArrayRef<PassBuilder::PipelineElement>) {
    if (Name != PassT::getArgString())
      return false;
    MPM.addPass(PassT{PassArgs...});
    return true;
  };
  PB.registerPipelineParsingCallback(Reg);
}

static void registerPasses(PassBuilder &PB) {
  registerModulePass<CMSimdCFLowering>(PB);
  registerModulePass<GenXSPIRVWriterAdaptor>(
      PB, /*RewriteTypes=*/true, /*RewriteSingleElementVectors=*/true);
  registerModulePass<GenXSPIRVReaderAdaptor>(PB);
}

static PassPluginLibraryInfo getIntrinsicsPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "VC intrinsics plugin", "v1",
          registerPasses};
}

// Entry point for plugin in new PM infrastructure.
extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return getIntrinsicsPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM support
//-----------------------------------------------------------------------------
// Register intrinsics passes on dynamic loading of plugin library.
static int initializePasses() {
  PassRegistry &PR = *PassRegistry::getPassRegistry();

  initializeCMSimdCFLoweringLegacyPass(PR);
  initializeGenXSPIRVReaderAdaptorLegacyPass(PR);
  initializeGenXSPIRVWriterAdaptorLegacyPass(PR);

  return 0;
}

static const int Init = initializePasses();

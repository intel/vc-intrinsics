/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


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
  registerModulePass<GenXSPIRVWriterAdaptor>(
      PB, /*RewriteTypes=*/true, /*RewriteSingleElementVectors=*/true);
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

  initializeCMSimdCFLoweringPass(PR);
  initializeGenXSPIRVReaderAdaptorPass(PR);
  initializeGenXSPIRVWriterAdaptorLegacyPass(PR);

  return 0;
}

static const int Init = initializePasses();

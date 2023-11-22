/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

///
/// GenXSPIRVReaderAdaptor
/// ---------------------------
/// This pass converts metadata from SPIRV format to whichever used in backend

#include "llvm/IR/PassManager.h"

namespace llvm {
class ModulePass;
class PassRegistry;

//-----------------------------------------------------------------------------
// New PM support
//-----------------------------------------------------------------------------
// Reader adaptor for new PM.
class GenXSPIRVReaderAdaptor final
    : public PassInfoMixin<GenXSPIRVReaderAdaptor> {

public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  static StringRef getArgString() { return "GenXSPIRVReaderAdaptor"; }
};

//-----------------------------------------------------------------------------
// Legacy PM support
//-----------------------------------------------------------------------------
void initializeGenXSPIRVReaderAdaptorLegacyPass(PassRegistry &);

ModulePass *createGenXSPIRVReaderAdaptorPass();
} // namespace llvm

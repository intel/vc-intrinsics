/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

///
/// GenXSPIRVWriterAdaptor
/// ---------------------------
/// This pass converts metadata to SPIRV format from whichever used in frontend

#include "llvm/IR/PassManager.h"

namespace llvm {
class ModulePass;
class PassRegistry;

//-----------------------------------------------------------------------------
// New PM support
//-----------------------------------------------------------------------------
// Writer adaptor for new PM.
class GenXSPIRVWriterAdaptor final
    : public PassInfoMixin<GenXSPIRVWriterAdaptor> {
  bool RewriteTypes = true;
  bool RewriteSingleElementVectors = true;

public:
  GenXSPIRVWriterAdaptor(bool RewriteTypesIn,
                         bool RewriteSingleElementVectorsIn)
      : RewriteTypes(RewriteTypesIn),
        RewriteSingleElementVectors(RewriteSingleElementVectorsIn) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static StringRef getArgString() { return "GenXSPIRVWriterAdaptor"; }
};

//-----------------------------------------------------------------------------
// Legacy PM support
//-----------------------------------------------------------------------------
void initializeGenXSPIRVWriterAdaptorLegacyPass(PassRegistry &);

// Create spirv writer adaptor pass.
// RewriteTypes -- whether plain types with decorations should be
// rewritten with native SPIRV types. Defaults to false for
// compatibility reasons until backend will be able to handle new
// types.
ModulePass *
createGenXSPIRVWriterAdaptorPass(bool RewriteTypes = false,
                                 bool RewriteSingleElementVectors = false);
} // namespace llvm

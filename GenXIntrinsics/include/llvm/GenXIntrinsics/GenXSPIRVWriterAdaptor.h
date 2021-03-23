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

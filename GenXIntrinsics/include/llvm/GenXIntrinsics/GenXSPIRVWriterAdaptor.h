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

#include "llvm/ADT/StringRef.h"
#include "llvm/Pass.h"

namespace llvm {
namespace genx {
class GenXSPIRVWriterAdaptor final : public ModulePass {

public:
  static char ID;
  explicit GenXSPIRVWriterAdaptor() : ModulePass(ID) {}
  llvm::StringRef getPassName() const override {
    return "GenX SPIRVWriter Adaptor";
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnModule(Module &M) override;

private:
  bool runOnFunction(Function &F);
};

} // namespace genx
} // namespace llvm

namespace llvm {
void initializeGenXSPIRVWriterAdaptorPass(PassRegistry &);
ModulePass *createGenXSPIRVWriterAdaptorPass();
} // namespace llvm

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



//===-- GenXRestoreIntrAttr.cpp - GenX Restore Intrinsics' attributes pass --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// GenXRestoreIntrAttr
/// -------------------
///
/// This is a module pass that restores attributes for intrinsics:
///
/// * Since SPIR-V doesn't save intrinsics' attributes, some important
///   information can be lost. This pass restores it.
///
/// * Only GenX intrinsics are handled.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "GENX_RESTOREINTRATTR"

#include "llvm/GenXIntrinsics/GenXIntrOpts.h"
#include "llvm/GenXIntrinsics/GenXIntrinsics.h"
#include "llvm/Support/Debug.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace {

// GenXRestoreIntrAttr : restore intrinsics' attributes
class GenXRestoreIntrAttr : public ModulePass {
public:
  GenXRestoreIntrAttr();

  StringRef getPassName() const override {
    return "GenX Restore Intrinsics' Attributes";
  }

  bool runOnModule(Module &M) override;

private:
  bool restoreAttributes(Function *F);

public:
  static char ID;
};
} // namespace

namespace llvm {
void initializeGenXRestoreIntrAttrPass(PassRegistry &);
}
INITIALIZE_PASS_BEGIN(GenXRestoreIntrAttr, "GenXRestoreIntrAttr",
                      "GenXRestoreIntrAttr", false, false)
INITIALIZE_PASS_END(GenXRestoreIntrAttr, "GenXRestoreIntrAttr",
                    "GenXRestoreIntrAttr", false, false)

char GenXRestoreIntrAttr::ID = 0;

Pass *llvm::createGenXRestoreIntrAttrPass() {
  return new GenXRestoreIntrAttr;
}

GenXRestoreIntrAttr::GenXRestoreIntrAttr() : ModulePass(ID) {
  initializeGenXRestoreIntrAttrPass(*PassRegistry::getPassRegistry());
}

bool GenXRestoreIntrAttr::restoreAttributes(Function *F) {
  LLVM_DEBUG(dbgs() << "Restoring attributes for: " << F->getName() << "\n");
  F->setAttributes(GenXIntrinsic::getAttributes(F->getContext(), GenXIntrinsic::getGenXIntrinsicID(F)));
  return true;
}

bool GenXRestoreIntrAttr::runOnModule(Module &M) {
  bool Modified = false;

  for (auto &F : M.getFunctionList()) {
    if (GenXIntrinsic::isGenXIntrinsic(&F))
      Modified |= restoreAttributes(&F);
  }

  return Modified;
}

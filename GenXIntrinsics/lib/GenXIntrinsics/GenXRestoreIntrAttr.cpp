/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

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

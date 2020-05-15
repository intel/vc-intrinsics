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


//===----------------------------------------------------------------------===//
//
// This file defines classes that make it really easy to deal with intrinsic
// functions with the isa/dyncast family of functions.  In particular, this
// allows you to do things like:
//
//     if (MemCpyInst *MCI = dyn_cast<MemCpyInst>(Inst))
//        ... MCI->getDest() ... MCI->getSource() ...
//
// All intrinsic function calls are instances of the call instruction, so these
// are all subclasses of the CallInst class.  Note that none of these classes
// has state or virtual methods, which is an important part of this gross/neat
// hack working.
//
//===----------------------------------------------------------------------===//

#ifndef GENX_INTRINSIC_INST_H
#define GENX_INTRINSIC_INST_H

#include "llvm/GenXIntrinsics/GenXIntrinsics.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

namespace llvm {
/// IntrinsicInst - A useful wrapper class for inspecting calls to intrinsic
/// functions.  This allows the standard isa/dyncast/cast functionality to
/// work with calls to intrinsic functions.
class GenXIntrinsicInst : public CallInst {
public:
  GenXIntrinsicInst() = delete;
  GenXIntrinsicInst(const GenXIntrinsicInst &) = delete;
  void operator=(const GenXIntrinsicInst &) = delete;

  /// getIntrinsicID - Return the intrinsic ID of this intrinsic.
  ///
  GenXIntrinsic::ID getIntrinsicID() const {
    return GenXIntrinsic::getGenXIntrinsicID(getCalledFunction());
  }

  // Methods for support type inquiry through isa, cast, and dyn_cast:
  static inline bool classof(const CallInst *I) {
    if (const Function *CF = I->getCalledFunction()) {
      return CF->getName().startswith(GenXIntrinsic::getGenXIntrinsicPrefix());
    }
    return false;
  }
  static inline bool classof(const Value *V) {
    return isa<CallInst>(V) && classof(cast<CallInst>(V));
  }
};

// TODO: add more classes to make our intrinsics easier to use

} // namespace llvm

#endif

/*========================== begin_copyright_notice ============================

Copyright (C) 2021-2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

SPDX-License-Identifier: MIT
============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_INTRINSICS_H
#define VCINTR_IR_INTRINSICS_H

#include <llvm/IR/Intrinsics.h>

namespace VCINTR {

namespace Intrinsic {
inline std::string getName(llvm::Intrinsic::ID Id,
                           llvm::ArrayRef<llvm::Type *> Tys) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 13
  return llvm::Intrinsic::getNameNoUnnamedTypes(Id, Tys);
#else
  return llvm::Intrinsic::getName(Id, Tys);
#endif
}

} // namespace Intrinsic
} // namespace VCINTR

#endif // VCINTR_IR_INTRINSICS_H

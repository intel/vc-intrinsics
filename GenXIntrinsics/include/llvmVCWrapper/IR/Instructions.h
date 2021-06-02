/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

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

#ifndef VCINTR_IR_INSTRUCTIONS_H
#define VCINTR_IR_INSTRUCTIONS_H

#include <algorithm>
#include <llvm/IR/Instructions.h>

namespace VCINTR {
namespace ShuffleVectorInst {
auto static constexpr UndefMaskElem = -1;

// LLVM <= 10 does not have ShuffleVectorInst ctor which accepts ArrayRef<int>
// This method returns mask with appropriate type for ShuffleVectorInst ctor
#if VC_INTR_LLVM_VERSION_MAJOR <= 10
static llvm::Constant *getShuffleMask(llvm::ArrayRef<int> Mask,
                                      llvm::LLVMContext &Context) {
  using namespace llvm;
  auto Indices = SmallVector<llvm::Constant *, 8>{};
  auto *Int32Ty = IntegerType::getInt32Ty(Context);
  std::transform(Mask.begin(), Mask.end(), std::back_inserter(Indices),
                 [&](int El) -> llvm::Constant * {
                   if (El == UndefMaskElem)
                     return UndefValue::get(Int32Ty);
                   else
                     return ConstantInt::get(Int32Ty, El);
                 });
  return ConstantVector::get(Indices);
}
#else
static llvm::ArrayRef<int> getShuffleMask(llvm::ArrayRef<int> Mask,
                                          llvm::LLVMContext &Context) {
  return Mask;
}
#endif

} // namespace VCINTR

} // namespace VCINTR

#endif // VCINTR_IR_INSTRUCTIONS_H

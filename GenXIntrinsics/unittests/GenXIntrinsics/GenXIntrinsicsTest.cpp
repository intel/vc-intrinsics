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


#include "llvm/ADT/StringRef.h"
#include "llvm/GenXIntrinsics/GenXIntrinsics.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "gtest/gtest.h"

using namespace llvm;

namespace {

TEST(GenXIntrinsics, OverloadedTypes) {
  EXPECT_EQ(GenXIntrinsic::isOverloadedArg(Intrinsic::fma, 0), false);
  EXPECT_EQ(GenXIntrinsic::isOverloadedArg(Intrinsic::fma, 1), false);
  EXPECT_EQ(GenXIntrinsic::isOverloadedArg(GenXIntrinsic::genx_3d_sample, 7),
            true);
  EXPECT_EQ(GenXIntrinsic::isOverloadedArg(GenXIntrinsic::genx_raw_send, 1),
            true);
  EXPECT_EQ(GenXIntrinsic::isOverloadedArg(GenXIntrinsic::genx_simdcf_any, 0),
            true);
}
} // namespace

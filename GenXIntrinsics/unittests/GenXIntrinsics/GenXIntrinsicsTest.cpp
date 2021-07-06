/*========================== begin_copyright_notice ============================

Copyright (C) 2019-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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

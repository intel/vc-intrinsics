#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2019-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

set(LLVM_LINK_COMPONENTS
  Core
  Support
  CodeGen
  )

add_genx_intrinsics_unittest(GenXIntrinsicsTests
  GenXIntrinsicsTest.cpp
  )

target_link_libraries(GenXIntrinsicsTests
  PRIVATE
  LLVMGenXIntrinsics
  LLVMTestingSupport
  )

#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2019-2021 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT
#============================ end_copyright_notice =============================

set(LLVM_COMPONENTS
  CodeGen
  Support
  Core
  Analysis
  )

if(BUILD_EXTERNAL)
  add_library(LLVMGenXIntrinsics 
              GenXIntrinsics.cpp
              GenXRestoreIntrAttr.cpp
              GenXSimdCFLowering.cpp
              GenXSingleElementVectorUtil.cpp
              GenXSPIRVReaderAdaptor.cpp
              GenXSPIRVWriterAdaptor.cpp
              GenXVersion.cpp
             )
  llvm_update_compile_flags(LLVMGenXIntrinsics)
  add_dependencies(LLVMGenXIntrinsics GenXIntrinsicsGen)

  vc_get_llvm_targets(LLVM_LIBS ${LLVM_COMPONENTS})
  target_link_libraries(LLVMGenXIntrinsics ${LLVM_LIBS})

  target_include_directories(LLVMGenXIntrinsics PUBLIC
    $<BUILD_INTERFACE:${GENX_INTRINSICS_MAIN_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
  )
else()
  # when we are building in LLVM infra, we need to conform
  set(LLVM_LINK_COMPONENTS
    ${LLVM_COMPONENTS}
    )

  add_llvm_library(LLVMGenXIntrinsics
    GenXIntrinsics.cpp
    GenXRestoreIntrAttr.cpp
    GenXSimdCFLowering.cpp
    GenXSingleElementVectorUtil.cpp
    GenXSPIRVReaderAdaptor.cpp
    GenXSPIRVWriterAdaptor.cpp
    GenXVersion.cpp
    ADDITIONAL_HEADER_DIRS
    ${GENX_INTRINSICS_MAIN_INCLUDE_DIR}/llvm/GenXIntrinsics

    DEPENDS
      GenXIntrinsicsGen
      intrinsics_gen
  )
endif()

#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2019-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

set(LLVM_COMPONENTS
  CodeGen
  Support
  Core
  Analysis
  )

set(SRC_LIST
  GenXIntrinsics.cpp
  GenXRestoreIntrAttr.cpp
  GenXSimdCFLowering.cpp
  GenXSingleElementVectorUtil.cpp
  GenXSPIRVReaderAdaptor.cpp
  GenXSPIRVWriterAdaptor.cpp
  GenXVersion.cpp
  AdaptorsCommon.cpp
  GenXMetadata.cpp
)

if(BUILD_EXTERNAL)
  add_library(LLVMGenXIntrinsics ${SRC_LIST})
  llvm_update_compile_flags(LLVMGenXIntrinsics)
  add_dependencies(LLVMGenXIntrinsics GenXIntrinsicsGen)

  vc_get_llvm_targets(LLVM_LIBS ${LLVM_COMPONENTS})
  target_link_libraries(LLVMGenXIntrinsics ${LLVM_LIBS})
else()
  # when we are building in LLVM infra, we need to conform
  set(LLVM_LINK_COMPONENTS
    ${LLVM_COMPONENTS}
    )

  if(LLVM_LINK_LLVM_DYLIB)
    add_llvm_library(LLVMGenXIntrinsics STATIC DISABLE_LLVM_LINK_LLVM_DYLIB
      ${SRC_LIST}
	
      ADDITIONAL_HEADER_DIRS
        ${GENX_INTRINSICS_MAIN_INCLUDE_DIR}/llvm/GenXIntrinsics
      DEPENDS
        GenXIntrinsicsGen
        intrinsics_gen
	    LLVMCodeGen
        LLVMSupport
        LLVMCore
        LLVMAnalysis
        LLVMSPIRVLib
    )
  else()
    add_llvm_library(LLVMGenXIntrinsics
      ${SRC_LIST}
	
      ADDITIONAL_HEADER_DIRS
        ${GENX_INTRINSICS_MAIN_INCLUDE_DIR}/llvm/GenXIntrinsics
      DEPENDS
        GenXIntrinsicsGen
        intrinsics_gen
    )
  endif()
endif()

target_include_directories(LLVMGenXIntrinsics PUBLIC
  $<BUILD_INTERFACE:${GENX_INTRINSICS_MAIN_INCLUDE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../include>
  $<INSTALL_INTERFACE:include>
  )

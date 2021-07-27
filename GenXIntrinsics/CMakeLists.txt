#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2019-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

cmake_minimum_required(VERSION 3.13.4)
include(GNUInstallDirs)

set(GENX_INTRINSICS_MAIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(GENX_INTRINSICS_MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(IGC_INFRA)
  set(GENX_INTRINSICS_MAIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
endif()

include(cmake/utils.cmake)

# Global config.
add_compile_definitions(VC_INTR_LLVM_VERSION_MAJOR=${LLVM_VERSION_MAJOR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

set(GENX_INTRINSICS_DESCRIPTION "GenXIntrinsicDescription.gen")

add_subdirectory(include/llvm)
add_subdirectory(lib)

# LLVM doesn't install gtest which is required for our tests
# so we cannot simply enable current unittests with prebuilt LLVM
if(NOT BUILD_EXTERNAL)
  add_subdirectory(unittests)
endif()

# Experimental lit tests for intrinsic passes. Require plugin support,
# so only available with LLVM dylib (for stability).
if(VC_INTR_ENABLE_LIT_TESTS)
  if(LLVM_LINK_LLVM_DYLIB)
    message(STATUS "VC intrinsics lit tests are enabled")
    add_subdirectory(test)
  else()
    message(STATUS "VC intrinsics lit tests require dynamic LLVM, skipping")
  endif()
endif()

# this option is to switch on install when we are building not inside IGC
if(INSTALL_REQUIRED)
  install(DIRECTORY include/llvm
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT genx-intrinsics-headers
    FILES_MATCHING
    PATTERN "*.h"
  )

  # cmake creates too many subdirectories in build directory
  # and then "install(DIRECTORY" installs them even if they are empty
  # so generated file has to be installed separetely
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/llvm/GenXIntrinsics/${GENX_INTRINSICS_DESCRIPTION}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/llvm/GenXIntrinsics
    COMPONENT genx-intrinsics-headers
  )

  # for now not clear do we need to check deps on install,
  # so config will only contain single file
  # we need to limit it to external build
  if(BUILD_EXTERNAL)
    install(TARGETS LLVMGenXIntrinsics EXPORT LLVMGenXIntrinsicsTargets
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT LLVMGenXIntrinsicsTargets
      FILE LLVMGenXIntrinsicsConfig.cmake
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LLVMGenXIntrinsics
    )
  endif() # BUILD_EXTERNAL
endif() # INSTALL_REQUIRED

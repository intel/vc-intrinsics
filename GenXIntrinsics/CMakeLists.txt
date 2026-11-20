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

include(CheckCXXSourceCompiles)
unset(VC_INTR_LLVM_HAS_ANY_SAME_VEC_WIDTH)
unset(VC_INTR_LLVM_HAS_ANY_SAME_VEC_WIDTH CACHE)
set(_VC_INTR_SAVED_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES}")
set(_VC_INTR_REQUIRED_INCLUDES ${LLVM_INCLUDE_DIRS} ${LLVM_INCLUDE_DIR}
  ${LLVM_MAIN_SRC_DIR}/include ${LLVM_BINARY_DIR}/include)
list(FILTER _VC_INTR_REQUIRED_INCLUDES EXCLUDE REGEX "^$")
list(REMOVE_DUPLICATES _VC_INTR_REQUIRED_INCLUDES)
set(_VC_INTR_SAVED_TRY_COMPILE_TARGET_TYPE "${CMAKE_TRY_COMPILE_TARGET_TYPE}")
set(CMAKE_REQUIRED_INCLUDES ${_VC_INTR_REQUIRED_INCLUDES})
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
check_cxx_source_compiles("\
#include <llvm/IR/Intrinsics.h>\n\
int main() {\n\
  using namespace llvm::Intrinsic;\n\
  auto D = IITDescriptor::get(IITDescriptor::AnySameVecWidth, 0,\n\
                              IITDescriptor::EC_Pointer << 4);\n\
  return D.Kind == IITDescriptor::AnySameVecWidth ? 0 : 1;\n\
}\n\
" VC_INTR_LLVM_HAS_ANY_SAME_VEC_WIDTH)
set(CMAKE_REQUIRED_INCLUDES "${_VC_INTR_SAVED_REQUIRED_INCLUDES}")
set(CMAKE_TRY_COMPILE_TARGET_TYPE "${_VC_INTR_SAVED_TRY_COMPILE_TARGET_TYPE}")
unset(_VC_INTR_SAVED_REQUIRED_INCLUDES)
unset(_VC_INTR_SAVED_TRY_COMPILE_TARGET_TYPE)
unset(_VC_INTR_REQUIRED_INCLUDES)

if(VC_INTR_LLVM_HAS_ANY_SAME_VEC_WIDTH)
  add_compile_definitions(VC_INTR_LLVM_HAS_ANY_SAME_VEC_WIDTH)
endif()

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

install(TARGETS LLVMGenXIntrinsics EXPORT LLVMGenXIntrinsicsTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install Config-file only for external build
if(BUILD_EXTERNAL OR LLVM_GENX_INTRINSICS_IN_TREE_INSTALL)

  # Legacy export. To remove when all clients switch to new name.
  install(EXPORT LLVMGenXIntrinsicsTargets
    FILE LLVMGenXIntrinsicsConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LLVMGenXIntrinsics
  )

  set(PACKAGE_NAME VCIntrinsics${LLVM_VERSION_MAJOR})
  install(EXPORT LLVMGenXIntrinsicsTargets
    FILE ${PACKAGE_NAME}Config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PACKAGE_NAME}
  )
endif() # BUILD_EXTERNAL OR LLVM_GENX_INTRINSICS_IN_TREE_INSTALL

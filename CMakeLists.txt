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

cmake_minimum_required(VERSION 3.13.4)

set(LLVM_GENX_INTRINSICS_VERSION 1.0)

set(LLVM_GENX_INTRINSICS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# check if we build inside llvm or not
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  get_filename_component(LLVM_CMAKE_PATH ${LLVM_DIR} ABSOLUTE BASE_DIR ${CMAKE_BINARY_DIR})
  set(BUILD_EXTERNAL YES)
  if (NOT DEFINED INSTALL_REQUIRED)
    set(INSTALL_REQUIRED YES)
  endif()
  project(LLVM_GenXIntrinsics
    VERSION
      ${LLVM_GENX_INTRINSICS_VERSION}
    LANGUAGES
      C CXX
  )

  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED OFF)

  find_package(LLVM REQUIRED HINTS "${LLVM_CMAKE_PATH}")
  set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${LLVM_CMAKE_DIR}
  )
  include(AddLLVM)
  include(HandleLLVMOptions)

  include_directories("${LLVM_INCLUDE_DIR}")
  link_directories("${LLVM_LIBRARY_DIR}")

  message(STATUS "Found LLVM: ${LLVM_VERSION}")
else(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(BUILD_EXTERNAL NO)

  # LLVM_CMAKE_DIR is not set for non-standalone builds. Use LLVM_CMAKE_PATH
  # instead. (see clang/CMakeLists.txt)
  if (NOT LLVM_CMAKE_DIR)
    set(LLVM_CMAKE_DIR ${LLVM_CMAKE_PATH})
  endif()
endif(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

include(FindPythonInterp)
if( NOT PYTHONINTERP_FOUND )
  message(FATAL_ERROR
"Unable to find Python interpreter, required for builds and testing.

Please install Python or specify the PYTHON_EXECUTABLE CMake variable.")
endif()

if( "${PYTHON_VERSION_STRING}" VERSION_LESS 2.7 )
  message(FATAL_ERROR "Python 2.7 or newer is required")
endif()


add_subdirectory(GenXIntrinsics)

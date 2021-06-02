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

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${GENX_INTRINSICS_DESCRIPTION}
    COMMAND ${PYTHON_EXECUTABLE} -B
            ${CMAKE_CURRENT_SOURCE_DIR}/Intrinsics.py
            ${CMAKE_CURRENT_SOURCE_DIR}/Intrinsic_definitions.py
            ${CMAKE_CURRENT_BINARY_DIR}/${GENX_INTRINSICS_DESCRIPTION}
    DEPENDS
         ${CMAKE_CURRENT_SOURCE_DIR}/Intrinsics.py
         ${CMAKE_CURRENT_SOURCE_DIR}/Intrinsic_definitions.py
    COMMENT "Building ${GENX_INTRINSICS_DESCRIPTION}..."
)

find_first_existing_vc_file(intrinsics_vc "${GENX_INTRINSICS_MAIN_DIR}")

set(version_inc ${CMAKE_CURRENT_BINARY_DIR}/GenXVersion.inc)

if(LLVM_GENX_INTRINSICS_ROOT_DIR)
  set(LLVM_GENX_INTRINSICS_REPO_DIR ${LLVM_GENX_INTRINSICS_ROOT_DIR})
else()
  set(LLVM_GENX_INTRINSICS_REPO_DIR ${GENX_INTRINSICS_MAIN_DIR}/..)
endif()

if(${LLVM_VERSION_MAJOR} LESS 9)
  find_file(FOUND_VCS GetSVN.cmake PATHS ${LLVM_CMAKE_DIR} REQUIRED)
  add_custom_command(
    OUTPUT "${version_inc}"
    COMMAND
    ${CMAKE_COMMAND} "-DSOURCE_DIRS=${LLVM_GENX_INTRINSICS_REPO_DIR}"
                     "-DNAMES=VCI"
                     "-DHEADER_FILE=${version_inc}"
                     -P "${FOUND_VCS}")
else()
  find_file(FOUND_VCS VersionFromVCS.cmake PATHS ${LLVM_CMAKE_DIR} REQUIRED)
  add_custom_command(
    OUTPUT "${version_inc}"
    COMMAND
    ${CMAKE_COMMAND} "-DSOURCE_DIR=${LLVM_GENX_INTRINSICS_REPO_DIR}"
                     "-DNAME=VCI"
                     "-DHEADER_FILE=${version_inc}"
                     "-DVCS_SCRIPT=${FOUND_VCS}"
                     -P "${CMAKE_CURRENT_SOURCE_DIR}/ConfigureVersionFile.cmake")
endif()

set_source_files_properties("${version_inc}"
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE)

add_custom_target(GenXIntrinsicDescriptionGen
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${GENX_INTRINSICS_DESCRIPTION}
    ${version_inc}
)
add_custom_target(GenXIntrinsicsGen)
add_dependencies(GenXIntrinsicsGen GenXIntrinsicDescriptionGen)

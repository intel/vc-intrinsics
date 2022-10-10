#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2019-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${GENX_INTRINSICS_DESCRIPTION}
    COMMAND ${Python_EXECUTABLE} -B
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

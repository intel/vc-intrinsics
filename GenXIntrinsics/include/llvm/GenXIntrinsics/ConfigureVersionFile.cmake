#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2020-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

include(${VCS_SCRIPT})

function(generate_version_file output_file)
  get_source_info(${SOURCE_DIR} rev repo)
  file(APPEND "${output_file}.txt" "#define ${NAME}_REVISION \"${rev}\"\n")
  file(APPEND "${output_file}.txt" "#define ${NAME}_REPOSITORY \"${repo}\"\n")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${output_file}.txt" "${output_file}")
  file(REMOVE "${output_file}.txt")

endfunction()

generate_version_file(${HEADER_FILE})


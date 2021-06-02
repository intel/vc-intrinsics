#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2020-2021 Intel Corporation
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

if(BUILD_EXTERNAL)
  if(NOT EXISTS ${LLVM_EXTERNAL_LIT})
    message(FATAL_ERROR "External build requires LLVM_EXTERNAL_LIT to be defined to lit executable")
  endif()
endif()

# Add plugin with all intrinsics libraries for loading with opt.
add_subdirectory(Plugin)

set(VC_INTRINSICS_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Generate temporary site config with LLVM variables filled.
configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/temp.cfg.py
  MAIN_CONFIG
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
  )

# Need to regenerate again since plugin name is required and proper
# way to get it is to use generator expressions that are not allowed
# in configure_file.
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/vcint.$<CONFIG>.lit.site.cfg.py"
  INPUT "${CMAKE_CURRENT_BINARY_DIR}/temp.cfg.py"
  )

set(USED_TOOLS
  # These are required by lit default substitutions.
  FileCheck
  count
  not
  # Main tool for plugin testing.
  opt
  )

if(NOT BUILD_EXTERNAL)
  set(TEST_DEPS
    ${USED_TOOLS}
    )
else()
  # Check for tools availability.
  foreach(tool ${USED_TOOLS})
    set(TOOL_PATH "${LLVM_TOOLS_BINARY_DIR}/${tool}")
    if(NOT EXISTS ${TOOL_PATH})
      message(FATAL_ERROR "Tool ${tool} is not found (required by lit tests)")
    endif()
  endforeach()
endif()

# Add testsuite with custom config name that depends on generator.
add_lit_testsuite(check-vc-intrinsics "Running the vc-intrinsics regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  ARGS
    "--config-prefix=vcint.$<CONFIG>.lit"
    "-sv"
  DEPENDS
    ${TEST_DEPS}
    VCIntrinsicsPlugin
)

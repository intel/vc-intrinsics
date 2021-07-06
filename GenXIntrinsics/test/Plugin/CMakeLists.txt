#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2020-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

set(PLUGIN_SOURCES
  Plugin.cpp
  )

add_library(VCIntrinsicsPlugin
  MODULE
  ${PLUGIN_SOURCES}
  )

set(LINK_LIBS
  LLVMGenXIntrinsics
  )

target_link_libraries(VCIntrinsicsPlugin
  ${LINK_LIBS}
  )

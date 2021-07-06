#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2019-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

add_custom_target(GenXIntrinsicsUnitTests)
set_target_properties(GenXIntrinsicsUnitTests PROPERTIES FOLDER "GenXIntrinsicsTests")

function(add_genx_intrinsics_unittest test_dirname)
  add_unittest(GenXIntrinsicsUnitTests ${test_dirname} ${ARGN})
endfunction()

add_subdirectory(GenXIntrinsics)

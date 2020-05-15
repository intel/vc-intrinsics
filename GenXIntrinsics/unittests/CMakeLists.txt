add_custom_target(GenXIntrinsicsUnitTests)
set_target_properties(GenXIntrinsicsUnitTests PROPERTIES FOLDER "GenXIntrinsicsTests")

function(add_genx_intrinsics_unittest test_dirname)
  add_unittest(GenXIntrinsicsUnitTests ${test_dirname} ${ARGN})
endfunction()

add_subdirectory(GenXIntrinsics)

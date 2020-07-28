set(LLVM_LINK_COMPONENTS
  Core
  Support
  CodeGen
  )

add_genx_intrinsics_unittest(GenXIntrinsicsTests
  GenXIntrinsicsTest.cpp
  )

target_link_libraries(GenXIntrinsicsTests
  PRIVATE
  LLVMGenXIntrinsics
  LLVMTestingSupport
  )

set(LLVM_LINK_COMPONENTS
  Core
  Support
  CodeGen
  GenXIntrinsics
  )

add_genx_intrinsics_unittest(GenXIntrinsicsTests
  GenXIntrinsicsTest.cpp
  )

target_link_libraries(GenXIntrinsicsTests PRIVATE LLVMTestingSupport)

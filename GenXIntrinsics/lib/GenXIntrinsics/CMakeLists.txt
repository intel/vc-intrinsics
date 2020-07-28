set(LLVM_COMPONENTS
  CodeGen
  Support
  Core
  )

if(BUILD_EXTERNAL)
  add_library(LLVMGenXIntrinsics 
              GenXIntrinsics.cpp
              GenXRestoreIntrAttr.cpp
              GenXSimdCFLowering.cpp
              GenXSPIRVReaderAdaptor.cpp
              GenXSPIRVWriterAdaptor.cpp
             )
  llvm_update_compile_flags(LLVMGenXIntrinsics)
  add_dependencies(LLVMGenXIntrinsics GenXIntrinsicsGen)

  vc_get_llvm_targets(LLVM_LIBS ${LLVM_COMPONENTS})
  target_link_libraries(LLVMGenXIntrinsics ${LLVM_LIBS})

  target_include_directories(LLVMGenXIntrinsics PUBLIC
    $<BUILD_INTERFACE:${GENX_INTRINSICS_MAIN_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
  )
else()
  # when we are building in LLVM infra, we need to conform
  set(LLVM_LINK_COMPONENTS
    ${LLVM_COMPONENTS}
    )

  add_llvm_library(LLVMGenXIntrinsics
    GenXIntrinsics.cpp
    GenXRestoreIntrAttr.cpp
    GenXSimdCFLowering.cpp
    GenXSPIRVReaderAdaptor.cpp
    GenXSPIRVWriterAdaptor.cpp
    ADDITIONAL_HEADER_DIRS
    ${GENX_INTRINSICS_MAIN_INCLUDE_DIR}/llvm/GenXIntrinsics

    DEPENDS
      GenXIntrinsicsGen
      intrinsics_gen
  )
endif()

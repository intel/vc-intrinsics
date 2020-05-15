cmake_minimum_required(VERSION 3.3)

set(GENX_INTRINSICS_MAIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(IGC_INFRA)
  set(GENX_INTRINSICS_MAIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
endif()

set(GENX_INTRINSICS_DESCRIPTION "GenXIntrinsicDescription.gen")

add_subdirectory(include/llvm)
add_subdirectory(lib)

# LLVM doesn't install gtest which is required for our tests
# so we cannot simply enable current unittests with prebuilt LLVM
if(NOT BUILD_EXTERNAL)
  add_subdirectory(unittests)
endif()

# this option is to switch on install when we are building not inside IGC
if(INSTALL_REQUIRED)
  install(DIRECTORY include/llvm
    DESTINATION include
    COMPONENT genx-intrinsics-headers
    FILES_MATCHING
    PATTERN "*.h"
  )

  # cmake creates too many subdirectories in build directory
  # and then "install(DIRECTORY" installs them even if they are empty
  # so generated file has to be installed separetely
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/llvm/GenXIntrinsics/${GENX_INTRINSICS_DESCRIPTION}
    DESTINATION include/llvm/GenXIntrinsics
    COMPONENT genx-intrinsics-headers
  )

  # for now not clear do we need to check deps on install,
  # so config will only contain single file
  # we need to limit it to external build
  if(BUILD_EXTERNAL)
    install(TARGETS LLVMGenXIntrinsics EXPORT LLVMGenXIntrinsicsTargets
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      RUNTIME DESTINATION bin
      INCLUDES DESTINATION include
    )

    install(EXPORT LLVMGenXIntrinsicsTargets
      FILE LLVMGenXIntrinsicsConfig.cmake
      DESTINATION lib/cmake/LLVMGenXIntrinsics
    )
  endif() # BUILD_EXTERNAL
endif() # INSTALL_REQUIRED

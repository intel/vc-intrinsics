cmake_minimum_required(VERSION 3.13.4)

set(LLVM_GENX_INTRINSICS_VERSION 1.0)

# check if we build inside llvm or not
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  get_filename_component(LLVM_CMAKE_PATH ${LLVM_DIR} ABSOLUTE BASE_DIR ${CMAKE_BINARY_DIR})
  set(BUILD_EXTERNAL YES)
  if (NOT DEFINED INSTALL_REQUIRED)
    set(INSTALL_REQUIRED YES)
  endif()
  project(LLVM_GenXIntrinsics
    VERSION
      ${LLVM_GENX_INTRINSICS_VERSION}
    LANGUAGES
      C CXX
  )

  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED OFF)

  find_package(LLVM REQUIRED HINTS "${LLVM_CMAKE_PATH}")
  set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${LLVM_CMAKE_DIR}
  )
  include(AddLLVM)
  include(HandleLLVMOptions)

  include_directories("${LLVM_INCLUDE_DIR}")
  link_directories("${LLVM_LIBRARY_DIR}")

  message(STATUS "Found LLVM: ${LLVM_VERSION}")
else(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(BUILD_EXTERNAL NO)

  # LLVM_CMAKE_DIR is not set for non-standalone builds. Use LLVM_CMAKE_PATH
  # instead. (see clang/CMakeLists.txt)
  if (NOT LLVM_CMAKE_DIR)
    set(LLVM_CMAKE_DIR ${LLVM_CMAKE_PATH})
  endif()

  # we require include_directories because other components may include intrinsics headers.
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/GenXIntrinsics/include)
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/GenXIntrinsics/include)
endif(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

include(FindPythonInterp)
if( NOT PYTHONINTERP_FOUND )
  message(FATAL_ERROR
"Unable to find Python interpreter, required for builds and testing.

Please install Python or specify the PYTHON_EXECUTABLE CMake variable.")
endif()

if( "${PYTHON_VERSION_STRING}" VERSION_LESS 2.7 )
  message(FATAL_ERROR "Python 2.7 or newer is required")
endif()


add_subdirectory(GenXIntrinsics)

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

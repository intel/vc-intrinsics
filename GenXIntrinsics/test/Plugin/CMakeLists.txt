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

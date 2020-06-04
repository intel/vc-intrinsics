add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${GENX_INTRINSICS_DESCRIPTION}
    COMMAND ${PYTHON_EXECUTABLE} -B
            ${CMAKE_CURRENT_SOURCE_DIR}/Intrinsics.py
            ${CMAKE_CURRENT_SOURCE_DIR}/Intrinsic_definitions.py
            ${CMAKE_CURRENT_BINARY_DIR}/${GENX_INTRINSICS_DESCRIPTION}
    DEPENDS
         ${CMAKE_CURRENT_SOURCE_DIR}/Intrinsics.py
         ${CMAKE_CURRENT_SOURCE_DIR}/Intrinsic_definitions.py
    COMMENT "Building ${GENX_INTRINSICS_DESCRIPTION}..."
)

add_custom_target(GenXIntrinsicDescriptionGen
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${GENX_INTRINSICS_DESCRIPTION}
)
add_custom_target(GenXIntrinsicsGen)
add_dependencies(GenXIntrinsicsGen GenXIntrinsicDescriptionGen)

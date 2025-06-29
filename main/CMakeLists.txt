file(GLOB SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

idf_component_register(SRCS ${SRC_FILES}
                    INCLUDE_DIRS ""
                    WHOLE_ARCHIVE)

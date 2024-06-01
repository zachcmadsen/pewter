add_library(libpewter STATIC)

set_target_properties(libpewter PROPERTIES C_STANDARD 99)
set_target_properties(libpewter PROPERTIES C_STANDARD_REQUIRED ON)
set_target_properties(libpewter PROPERTIES C_EXTENSIONS OFF)

set_target_properties(libpewter PROPERTIES PREFIX "")

target_sources(libpewter PRIVATE save.c)

target_include_directories(libpewter PUBLIC ${CMAKE_SOURCE_DIR}/src/libpewter/include)

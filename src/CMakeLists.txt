include(FetchContent)

FetchContent_Declare(
    FLTK
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/../fltk
    # GIT_REPOSITORY https://github.com/fltk/fltk
    # GIT_TAG master
    # GIT_SHALLOW TRUE
)

message(STATUS "Fetching FLTK...")
FetchContent_MakeAvailable(FLTK)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG master
)

message(STATUS "Fetching fmt...")
FetchContent_MakeAvailable(fmt)

# "WIN32 MACOSX_BUNDLE" apparently tells CMake that pewter is a GUI app.
add_executable(pewter WIN32 MACOSX_BUNDLE main.cpp)

set_target_properties(pewter PROPERTIES CXX_STANDARD 20)
set_target_properties(pewter PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(pewter PROPERTIES CXX_EXTENSIONS OFF)

if (PEWTER_CXX_FLAGS)
    separate_arguments(PEWTER_CXX_FLAGS_LIST NATIVE_COMMAND ${PEWTER_CXX_FLAGS})
    target_compile_options(pewter PRIVATE ${PEWTER_CXX_FLAGS_LIST})
endif()

target_sources(pewter PRIVATE main.cpp app.cpp save.cpp util.cpp)

target_include_directories(pewter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(pewter PRIVATE fltk::fltk fmt::fmt)

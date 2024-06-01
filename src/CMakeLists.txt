include(FetchContent)

# TODO: Look into FLTK's CMake options to try to build as little as possible.
FetchContent_Declare(
    FLTK
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/../fltk
    # GIT_REPOSITORY https://github.com/fltk/fltk
    # GIT_TAG master
    # GIT_SHALLOW TRUE
)

message(STATUS "Fetching FLTK...")
FetchContent_MakeAvailable(FLTK)

# "WIN32 MACOSX_BUNDLE" apparently tells CMake that pewter is a GUI app.
add_executable(pewter WIN32 MACOSX_BUNDLE main.cpp)

set_target_properties(pewter PROPERTIES CXX_STANDARD 14)
set_target_properties(pewter PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(pewter PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(pewter PRIVATE fltk::fltk)

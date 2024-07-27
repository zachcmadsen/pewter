cmake_minimum_required(VERSION 3.27)

project(pewter LANGUAGES CXX)

set(FLTK_BUILD_FLTK_OPTIONS OFF)
set(FLTK_BUILD_FLUID OFF)
set(FLTK_BUILD_FORMS OFF)
set(FLTK_BUILD_GL OFF)
# Tell FLTK to use to the statically-linked MSVC runtime.
set(FLTK_MSVC_RUNTIME_DLL OFF)

include(FetchContent)
FetchContent_Declare(
    FLTK
    GIT_REPOSITORY https://github.com/fltk/fltk
    GIT_TAG b740c48ee8505f93662065384d84b449d5e33438
    GIT_SHALLOW TRUE
)
message(STATUS "Fetching FLTK...")
FetchContent_MakeAvailable(FLTK)

add_executable(pewter WIN32 MACOSX_BUNDLE)
target_sources(
	pewter PRIVATE
	src/app.cpp
	src/main.cpp
	src/save.cpp 
	src/util.cpp)
target_include_directories(pewter PRIVATE src)
target_link_libraries(pewter PRIVATE fltk::fltk)
target_compile_features(pewter PRIVATE cxx_std_23)

if(MSVC)
	# Use the statically-linked MSVC runtime.
	set_property(TARGET pewter PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
cmake_minimum_required(VERSION 3.27)

project(pewter LANGUAGES CXX)

# TODO: Disable FLTK bundled image libraries (except for PNG) to build even
# less. Hopefully it'll still compile...
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
    SYSTEM
)
message(STATUS "Fetching FLTK...")

if(WIN32)
	# Disable deprecation warnings only for FLTK on Windows.
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
	FetchContent_MakeAvailable(FLTK)
	remove_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
else()
	FetchContent_MakeAvailable(FLTK)
endif()

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

if(PEWTER_CXX_FLAGS)
	separate_arguments(PEWTER_CXX_FLAGS_LIST NATIVE_COMMAND ${PEWTER_CXX_FLAGS})
    target_compile_options(pewter PRIVATE ${PEWTER_CXX_FLAGS_LIST})
endif()
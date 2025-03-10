# Find dependencies

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)

# Use a proper bgfx CMake fork instead of original repository
include(FetchContent)

# Preprocessor definitions
add_compile_definitions(UNICODE)
add_compile_definitions(MA_AUDIO_SAMPLES)
add_compile_definitions(MA_AUDIO_ENGINE)
add_compile_definitions(LEVEL_EDITOR)
add_compile_definitions(FLYCHEAT_NOCLIP)
add_compile_definitions(TOMB4_EXPORTS)

# BGFX with proper CMake support
message(STATUS "Attempting to fetch bgfx...")
FetchContent_Declare(
  bgfx
  GIT_REPOSITORY https://github.com/bkaradzic/bgfx.cmake
  GIT_TAG master
)

message(STATUS "Attempting to fetch bx...")
FetchContent_Declare(
    bx
    GIT_REPOSITORY https://github.com/bkaradzic/bx
    GIT_TAG master
)

message(STATUS "Attempting to fetch bimg...")
FetchContent_Declare(
    bimg
    GIT_REPOSITORY https://github.com/bkaradzic/bimg
    GIT_TAG master
)


# Build bgfx tools including shaderc
set(BGFX_CONFIG_RENDERER_OPENGL ON CACHE BOOL "Enable OpenGL renderer")
set(BGFX_BUILD_TOOLS ON CACHE BOOL "Build BGFX tools" FORCE)
set(BGFX_BUILD_SHADERC ON CACHE BOOL "Build shaderc" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "Disable examples" FORCE)

FetchContent_MakeAvailable(bgfx)

# After this, bgfx targets will be available but without namespace
# We need to create alias targets for proper namespacing
if(TARGET bgfx AND NOT TARGET bgfx::bgfx)
    add_library(bgfx::bgfx ALIAS bgfx)
    add_library(bx::bx ALIAS bx)
    add_library(bimg::bimg ALIAS bimg)
endif()

# SimpleINI
message(STATUS "Attempting to fetch simpleini...")
FetchContent_Declare(
  simpleini
  GIT_REPOSITORY https://github.com/brofield/simpleini.git
  GIT_TAG master  # or a specific commit/tag
)
FetchContent_MakeAvailable(simpleini)

# MiniAudio
message(STATUS "Attempting to fetch miniaudio...")
FetchContent_Declare(
  miniaudio
  GIT_REPOSITORY https://github.com/mackron/miniaudio
  GIT_TAG master  # or a specific commit/tag
)
FetchContent_MakeAvailable(miniaudio)

# Check if the miniaudio target is not already defined
if(NOT TARGET miniaudio)
  # If not, create the library and include stb_vorbis
  file(GLOB_RECURSE MINIAUDIO_SOURCES
      "${miniaudio_SOURCE_DIR}/extras/stb_vorbis.c"
  )
  add_library(miniaudio STATIC ${MINIAUDIO_SOURCES})
else()
  # If it exists, add stb_vorbis sources to the existing target if necessary
  # Note: Check if stb_vorbis is already included by the fetched miniaudio
  # If not, append it (assuming target allows modification)
  target_sources(miniaudio PRIVATE "${miniaudio_SOURCE_DIR}/extras/stb_vorbis.c")
endif()

# TinyJSON
message(STATUS "Attempting to fetch tinyjson...")
FetchContent_Declare(
  tinyjson
  GIT_REPOSITORY https://github.com/rafagafe/tiny-json
  GIT_TAG master  # or a specific commit/tag
)
FetchContent_MakeAvailable(tinyjson)

add_library(tinyjson STATIC ${tinyjson_SOURCE_DIR}/tiny-json.c ${tinyjson_SOURCE_DIR}/tiny-json.h)

# Gather source files
file(GLOB_RECURSE SRC_FILES
    "*.cpp"
    "*.h"
    "*.hpp"
)

# Shader compilation setup
add_subdirectory(shaders)

# Create executable
add_executable(tomb4plus
	${SRC_FILES}
	tomb4.rc)
	
if(TARGET compile_shaders)
    add_dependencies(tomb4plus compile_shaders)
endif()

# Link with aliased targets
target_link_libraries(tomb4plus PRIVATE
    bgfx
    bx
    bimg
    SDL2::SDL2main
    SDL2::SDL2
    OpenGL::GL
    ZLIB::ZLIB
    miniaudio
    tinyjson
)

# Include directories
target_include_directories(tomb4plus PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${BGFX_INCLUDE_DIRS}
    ${simpleini_SOURCE_DIR}
    ${miniaudio_SOURCE_DIR}
    ${tinyjson_SOURCE_DIR}
)

 if (MSVC)
	target_compile_definitions(tomb4plus PUBLIC UNICODE _UNICODE)
 endif()
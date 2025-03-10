# shaders/CMakeLists.txt

if(NOT TARGET shaderc)
    message(STATUS "Shader compilation disabled - shaderc target not found")
    return()
endif()

message(STATUS "Configuring cross-platform shader compilation...")

# Detect target platform
if(WIN32)
    set(SHADER_PLATFORM "windows")
    set(ENABLED_BACKENDS "DX11;SPIRV;GLSL")
    message(STATUS "Building shaders for Windows (DX11, SPIR-V, GLSL)")
elseif(APPLE)
    set(SHADER_PLATFORM "osx")
    set(ENABLED_BACKENDS "METAL;SPIRV")
    message(STATUS "Building shaders for macOS (Metal, SPIR-V)")
else()
    set(SHADER_PLATFORM "linux")
    set(ENABLED_BACKENDS "SPIRV;GLSL")
    message(STATUS "Building shaders for Linux (SPIR-V, GLSL)")
endif()

file(GLOB_RECURSE SHADER_FILES
    "vs_*.sc"
    "fs_*.sc"
)

if(NOT SHADER_FILES)
    message(WARNING "No shader files found in ${CMAKE_CURRENT_SOURCE_DIR}")
    return()
endif()

# Create platform-specific output directories
set(OUTPUT_DIRS)
foreach(backend IN LISTS ENABLED_BACKENDS)
    string(TOLOWER ${backend} backend_dir)
    list(APPEND OUTPUT_DIRS "${CMAKE_BINARY_DIR}/TOMB4/shaders/${backend_dir}")
endforeach()

file(MAKE_DIRECTORY ${OUTPUT_DIRS})

set(SHADER_OUTPUTS)
foreach(shader_file ${SHADER_FILES})
    get_filename_component(shader_name ${shader_file} NAME_WE)
    get_filename_component(shader_dir ${shader_file} DIRECTORY)

    # Determine shader type from filename
    if(shader_name MATCHES "^vs_")
        set(shader_type vertex)
    elseif(shader_name MATCHES "^fs_")
        set(shader_type fragment)
    else()
        continue()
    endif()

    # Common shaderc arguments
    set(BASE_ARGS
        -f "${shader_file}"
        --type ${shader_type}
        --verbose
        -i "${CMAKE_CURRENT_SOURCE_DIR}"
        --platform ${SHADER_PLATFORM}
    )

    # Create compilation commands for each enabled backend
    foreach(backend IN LISTS ENABLED_BACKENDS)
        string(TOLOWER ${backend} backend_dir)
        set(output_file "${CMAKE_BINARY_DIR}/TOMB4/shaders/${backend_dir}/${shader_name}.bin")
        
        if(backend STREQUAL "SPIRV")
            set(profile "spirv")
        elseif(backend STREQUAL "GLSL")
            set(profile "150")
        elseif(backend STREQUAL "DX11")
            set(profile "s_4_0")
        elseif(backend STREQUAL "METAL")
            set(profile "metal")
        endif()

        add_custom_command(
            OUTPUT ${output_file}
            COMMAND $<TARGET_FILE:shaderc>
                ${BASE_ARGS}
                -o "${output_file}"
                -p ${profile}
            DEPENDS shaderc ${shader_file}
            COMMENT "Compiling ${shader_name} for ${backend}"
            VERBATIM
        )
        list(APPEND SHADER_OUTPUTS ${output_file})
    endforeach()
endforeach()

add_custom_target(compile_shaders ALL
    DEPENDS ${SHADER_OUTPUTS}
    COMMENT "Building platform-specific shaders for ${SHADER_PLATFORM}"
)

# Add install target for shaders
install(DIRECTORY "${CMAKE_BINARY_DIR}/TOMB4/shaders/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/shaders"
    COMPONENT shaders
)
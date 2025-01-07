# glm
add_subdirectory(glm)
set( "GLM_DEFINITIONS"
    GLM_FORCE_MESSAGES
    GLM_FORCE_CXX20
    GLM_ENABLE_EXPERIMENTAL
    GLM_FORCE_SWIZZLE
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)
target_compile_definitions(glm PUBLIC ${GLM_DEFINITIONS})
target_compile_options( glm 
	PRIVATE 
	$<$<CXX_COMPILER_ID:MSVC>:/bigobj> 
	$<$<CXX_COMPILER_ID:GNU>:-Wa,-mbig-obj>
) 


# glfw 
add_subdirectory(glfw) 


# stb 
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")


# assimp
## To see more Setting details: https://github.com/assimp/assimp/blob/master/Build.md
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "If the test suite for Assimp is built in addition to the library." FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Install Assimp library. Disable this if you want to use Assimp as a submodule." FORCE)
set(ASSIMP_BUILD_ASSIMP_VIEW OFF CACHE BOOL "Build Assimp view tool (requires DirectX)." FORCE)

set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "Build Assimp with all exporters enabled." FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "Build Assimp with all importers enabled." FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER TRUE)

add_subdirectory(assimp)


# spdlog
add_subdirectory(spdlog)
## 设置 SPDLOG_ACTIVE_LEVEL 根据构建配置自动调整
target_compile_definitions(spdlog PUBLIC 
    $<$<CONFIG:DEBUG>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
    $<$<CONFIG:RELEASE>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
)


# glad
set(GLAD_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glad")
add_subdirectory("${GLAD_SOURCES_DIR}/cmake" glad_cmake)
GLAD_ADD_LIBRARY(glad_gl_compatibility_33 STATIC REPRODUCIBLE API gl:compatibility=3.3)

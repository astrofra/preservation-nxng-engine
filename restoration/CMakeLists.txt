include(FetchContent)

add_library(nxng_compat STATIC
    compat/src/cqString.cpp
    compat/src/qFileIO.cpp
    compat/src/MCIO.cpp
    compat/src/BitConvertIO.cpp
    compat/src/xPack.cpp
    compat/src/IMG.cpp
    compat/src/generic.cpp
    compat/src/JPEG.cpp
)

target_include_directories(nxng_compat
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/compat/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Headless CLI path (A2) with an OpenGL backend (A3).
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

add_executable(nxng-cli
    nxng_cli/src/main.cpp
    nxng_cli/src/scene_loader.cpp
    nxng_cli/src/opengl_renderer.cpp
)

target_include_directories(nxng-cli
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/nxng_cli/include
)

target_link_libraries(nxng-cli
    PRIVATE
        nxng_compat
        glfw
        OpenGL::GL
)

if(APPLE)
    target_compile_definitions(nxng-cli PRIVATE GL_SILENCE_DEPRECATION=1)
endif()

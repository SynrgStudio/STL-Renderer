cmake_minimum_required(VERSION 3.24)
project(STLRenderer VERSION 1.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Runtime estático: el .exe no necesita el redistribuible de Visual C++.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
include(CTest)
include(FetchContent)
option(STL_RENDERER_BUILD_APP "Build the Windows desktop application" ON)

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 8d1fd52e5ab5590e2c81768ace50c72bae28f2ed) # 1.0.3
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)
add_library(stl_core src/stl_loader.cpp src/cli.cpp)
target_include_directories(stl_core PUBLIC src)
target_link_libraries(stl_core PUBLIC glm::glm)
if(MSVC)
    target_compile_options(stl_core PRIVATE /utf-8 /W4)
endif()
if(BUILD_TESTING)
    add_executable(core_tests tests/core_tests.cpp)
    target_link_libraries(core_tests PRIVATE stl_core)
    add_test(NAME core_tests COMMAND core_tests)
endif()

if(STL_RENDERER_BUILD_APP)
    if(NOT WIN32)
        message(FATAL_ERROR "The desktop UI requires Windows. Use -DSTL_RENDERER_BUILD_APP=OFF for core tests.")
    endif()
    find_package(OpenGL REQUIRED)
    FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG f1cc2ae15e53a861a874c3034aae6798fde194ab) # v1.92.9b
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4)
    FetchContent_Declare(stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG d14d46f6b44456eb88beac58cba833a4135d4c89)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(imgui glfw stb)
    add_library(imgui_backend STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    target_include_directories(imgui_backend PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
    target_link_libraries(imgui_backend PUBLIC glfw OpenGL::GL)
    enable_language(RC)
    add_executable(STLRenderer WIN32
        src/main.cpp src/app.cpp src/gui.cpp src/renderer.cpp
        src/stl_renderer.cpp src/glad.c src/resources.rc)
    target_include_directories(STLRenderer PRIVATE external/include ${stb_SOURCE_DIR})
    target_compile_definitions(STLRenderer PRIVATE
        GLFW_INCLUDE_NONE NOMINMAX WIN32_LEAN_AND_MEAN
        STL_RENDERER_VERSION="${PROJECT_VERSION}" STBIW_WINDOWS_UTF8
        "STL_RENDERER_VERSION_RC=${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0")
    target_link_libraries(STLRenderer PRIVATE stl_core imgui_backend gdi32 shell32 ole32 comdlg32)
    if(MSVC)
        target_compile_options(STLRenderer PRIVATE /utf-8)
    endif()
    install(TARGETS STLRenderer RUNTIME DESTINATION bin)
    if(BUILD_TESTING)
        add_test(NAME cli_help COMMAND STLRenderer --help)
        add_test(NAME cli_version COMMAND STLRenderer --version)
        add_test(NAME cli_invalid COMMAND STLRenderer --unknown)
        set_tests_properties(cli_invalid PROPERTIES WILL_FAIL TRUE)
    endif()
endif()

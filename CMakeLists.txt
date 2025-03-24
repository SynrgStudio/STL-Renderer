cmake_minimum_required(VERSION 3.10)
project(STLRenderer VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opciones de compilación
option(USE_VULKAN "Usar Vulkan en lugar de OpenGL" OFF)

# Incluir dependencias
find_package(OpenGL REQUIRED)

# Directorio para bibliotecas externas
include_directories(${CMAKE_SOURCE_DIR}/external)
include_directories(${CMAKE_SOURCE_DIR}/external/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Descargar Dear ImGui
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.5
)
FetchContent_MakeAvailable(imgui)

# Descargar GLFW para manejo de ventanas
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# Descargar GLM para matemáticas
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Descargar stb_image para manejo de imágenes
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# Archivos fuente
set(SOURCES
    src/main.cpp
    src/renderer.cpp
    src/stl_loader.cpp
    src/app.cpp
    src/gui.cpp
    src/glad.c
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Archivos de cabecera
set(HEADERS
    src/renderer.h
    src/stl_loader.h
    src/app.h
    src/gui.h
    src/glad.h
)

# Ejecutable principal
add_executable(STLRenderer ${SOURCES} ${HEADERS})

# Enlazar bibliotecas
target_link_libraries(STLRenderer PRIVATE
    OpenGL::GL
    glfw
)

# Incluir directorios
target_include_directories(STLRenderer PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${stb_SOURCE_DIR}
    ${glm_SOURCE_DIR}
)

# Configuración específica de plataforma
if(WIN32)
    target_link_libraries(STLRenderer PRIVATE gdi32)
    
    # Configuración para crear un ejecutable de Windows
    if(MSVC)
        # Para Visual Studio, usar el subsistema de Windows sin modificar el punto de entrada
        set_target_properties(STLRenderer PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    else()
        # Para MinGW
        set_target_properties(STLRenderer PROPERTIES
            LINK_FLAGS "-mwindows"
        )
    endif()
    
    # Para manejo de recursos (icono, etc.)
    enable_language(RC)
    set(RC_FILE "src/resources.rc")
    target_sources(STLRenderer PRIVATE ${RC_FILE})
endif()

# Configuración de instalación
install(TARGETS STLRenderer DESTINATION bin) 
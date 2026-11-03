#include <windows.h>
#include <wingdi.h>
#include <combaseapi.h>
#include <ShlObj.h>
#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")

// Después de Windows, incluir el resto
#include "gui.h"
#include "app.h"
#include "renderer.h"
#include "stl_loader.h"
#include "cli.h"

// Luego OpenGL y otras bibliotecas
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

// Luego ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Y finalmente las otras dependencias
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <filesystem>
#include <functional>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <shellapi.h>

namespace fs = std::filesystem;

// Definiciones para la versión de ImGui que estamos usando
#ifndef ImGuiWindowFlags_NoDocking
#define ImGuiWindowFlags_NoDocking 0
#endif

#ifndef ImGuiConfigFlags_DockingEnable
#define ImGuiConfigFlags_DockingEnable 0
#endif

// Prototipos de funciones GLFW
#if defined(_WIN32)
typedef HWND(*GLFWgetWin32WindowProc)(GLFWwindow*);
#endif

namespace {
std::string toUtf8(const wchar_t* text) {
    const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1) return {};
    std::string result(size_t(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, result.data(), size, nullptr, nullptr);
    result.pop_back();
    return result;
}

// Devuelve la ruta elegida en UTF-8, o una cadena vacía si se canceló.
std::string fileDialog(GLFWwindow* window, bool save) {
    wchar_t filename[32768] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = glfwGetWin32Window(window);
    ofn.lpstrFile = filename;
    ofn.nMaxFile = static_cast<DWORD>(std::size(filename));
    if (save) {
        ofn.lpstrFilter = L"PNG Images\0*.png\0";
        ofn.lpstrDefExt = L"png";
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
        return GetSaveFileNameW(&ofn) ? toUtf8(filename) : std::string();
    }
    ofn.lpstrFilter = L"STL Files\0*.stl\0All Files\0*.*\0";
    ofn.lpstrDefExt = L"stl";
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    return GetOpenFileNameW(&ofn) ? toUtf8(filename) : std::string();
}

std::string folderDialog(GLFWwindow* window) {
    wchar_t path[MAX_PATH] = {};
    BROWSEINFOW bi = {};
    bi.hwndOwner = glfwGetWin32Window(window);
    bi.lpszTitle = L"Seleccionar carpeta con archivos STL";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
    if (!pidl) return {};
    const bool ok = SHGetPathFromIDListW(pidl, path);
    CoTaskMemFree(pidl);
    return ok ? toUtf8(path) : std::string();
}
} // namespace

// Datos para compartir entre GLFW y las funciones de callback
struct GUICallbackData {
    App* app;
    Gui* gui;
};

static GUICallbackData g_CallbackData;

Gui::Gui(App& app)
    : m_app(app)
    , m_window(nullptr)
    , m_showDemoWindow(false)
    , m_showColorPicker(true)
    , m_showCameraControls(true)
    , m_showModelInfo(false)
    , m_showBatchProcessing(true)
    , m_currentFile("")
    , m_saveFile("")
    , m_batchDirectory("")
    , m_processingBatch(false)
    , m_cameraYaw(0.0f)
    , m_cameraPitch(0.0f)
    , m_cameraDistance(2.0f)
    , m_isMouseDragging(false)
    , m_lastMouseX(0.0f)
    , m_lastMouseY(0.0f)
    , m_isMouseInPreviewWindow(false)
{
    // Inicializar valores por defecto de colores
    m_bgColor[0] = 0.0f; 
    m_bgColor[1] = 0.0f; 
    m_bgColor[2] = 0.0f;
    
    m_modelColor[0] = 0.7f;
    m_modelColor[1] = 0.7f;
    m_modelColor[2] = 0.7f;
    
}

Gui::~Gui() {
    // Limpiar ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool Gui::initialize() {
    // Obtener ventana de GLFW de la aplicación
    m_window = glfwGetCurrentContext();
    if (!m_window) {
        std::cerr << "Error: No hay contexto GLFW activo" << std::endl;
        return false;
    }
    
    
    // Configurar datos para callbacks
    g_CallbackData.app = &m_app;
    g_CallbackData.gui = this;
    
    // Configurar callback de arrastrar y soltar
    glfwSetDropCallback(m_window, dropCallback);
    
    // Inicializar ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // Configurar estilo
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 2.0f;
    style.FramePadding = ImVec2(4, 2);
    style.ItemSpacing = ImVec2(8, 4);
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = ImVec2(8, 8);
    
    // Inicializar backend
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    
    // Cargar configuración actual
    updateFromConfig();
    
    return true;
}

void Gui::processEvents() {
    glfwPollEvents();
}

void Gui::render() {
    // Iniciar un nuevo frame de ImGui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Limpiar toda la pantalla primero con el color de fondo
    glClearColor(m_app.getConfig().backgroundColor.r, 
                 m_app.getConfig().backgroundColor.g, 
                 m_app.getConfig().backgroundColor.b, 
                 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Diseño simplificado sin DockSpace
    ImGuiWindowFlags windowFlags = 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoBringToFrontOnFocus | 
        ImGuiWindowFlags_NoNavFocus;
    
    // Establecer el tamaño y posición de la ventana principal
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    // Interfaz principal
    ImGui::Begin("Main Window", nullptr, windowFlags);
    
    // Crear una simple división manual mediante columnas para reemplazar el DockSpace
    ImGui::Columns(2, "MainColumns", false);  // false = sin barra de redimensionamiento
    
    // Panel izquierdo (controles)
    {
        // Configurar ancho de la columna
        ImGui::SetColumnWidth(0, viewport->Size.x * 0.3f);
        
        // Paneles de control
        renderCameraControls();
        renderColorControls();
        renderBatchControls();
    }
    
    // Panel derecho (vista previa)
    ImGui::NextColumn();
    {
        // Guardar la posición antes de renderizar la vista previa
        ImVec2 previewPanelPos = ImGui::GetCursorScreenPos();
        ImVec2 previewPanelSize = ImGui::GetContentRegionAvail();
        
        // Renderizar y guardar la posición de la ventana de vista previa
        m_previewWindowPos = ImVec2(0, 0);  // Por defecto en caso de error
        m_previewWindowSize = ImVec2(0, 0);
        
        // Almacenar la posición actual antes de renderizar para poder capturarla
        ImGui::PushID("PreviewSection");
        
        renderPreviewWindow();
        
        // La posición y tamaño deben ser capturados en renderPreviewWindow
        
        ImGui::PopID();
        renderStatusBar();
    }
    
    ImGui::Columns(1);
    
    ImGui::End(); // Ventana principal
    
    // Renderizar la interfaz de ImGui
    ImGui::Render();
    
    // IMPORTANTE: Primero renderizamos la interfaz de ImGui
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // DESPUÉS de renderizar la interfaz, renderizamos el modelo 3D encima
    if (m_previewWindowSize.x > 0 && m_previewWindowSize.y > 0) {
        // Usar las coordenadas guardadas durante renderPreviewWindow
        float previewX = m_previewWindowPos.x;
        float previewY = m_previewWindowPos.y;
        float previewWidth = m_previewWindowSize.x;
        float previewHeight = m_previewWindowSize.y;
        
        // Guardar el viewport original
        int originalViewport[4];
        glGetIntegerv(GL_VIEWPORT, originalViewport);
        
        // Activar scissor test para limitar el renderizado al área de vista previa
        glEnable(GL_SCISSOR_TEST);
        glScissor(static_cast<int>(previewX), 
                static_cast<int>(ImGui::GetIO().DisplaySize.y - previewY - previewHeight), 
                static_cast<int>(previewWidth), 
                static_cast<int>(previewHeight));
        
        // SOLUCIÓN A LA DISTORSIÓN: Configurar el viewport para mantener las proporciones correctas
        // Usaremos todo el espacio disponible pero ajustaremos la matriz de proyección
        glViewport(static_cast<int>(previewX), 
                  static_cast<int>(ImGui::GetIO().DisplaySize.y - previewY - previewHeight), 
                  static_cast<int>(previewWidth), 
                  static_cast<int>(previewHeight));
        
        // Renderizar el modelo 3D encima de la interfaz
        try {
            // Limpiar solo el buffer de profundidad sin afectar el color
            glClear(GL_DEPTH_BUFFER_BIT);
            
            // Obtener referencia al renderer
            Renderer& renderer = m_app.getRenderer();
            
            // Establecer la cámara con los mismos valores exactos que se usarán en el render final
            renderer.setCameraOrbit(m_cameraYaw, m_cameraPitch, m_cameraDistance);
            
            // IMPORTANTE: Establecer una matriz de proyección con el aspect ratio correcto
            // para evitar la distorsión
            float aspectRatio = previewWidth / previewHeight;
            renderer.setProjectionMatrix(45.0f, aspectRatio, 0.1f, 100.0f);
            
            // Renderizar el modelo
            renderer.renderModel();
        }
        catch (...) {
            std::cout << "Error al renderizar modelo desde GUI\n";
        }
        
        // Restaurar el viewport original
        glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]);
        
        // Desactivar scissor test
        glDisable(GL_SCISSOR_TEST);
    }
}

bool Gui::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Gui::renderMainMenu() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Archivo")) {
            if (ImGui::MenuItem("Abrir STL...")) {
                openFileDialog();
            }

            if (ImGui::MenuItem("Guardar imagen...", nullptr, false, !m_currentFile.empty())) {
                const std::string file = fileDialog(m_window, true);
                if (!file.empty()) {
                    m_saveFile = file;
                    m_app.renderSingleFile(m_currentFile, m_saveFile);
                }
            }

            ImGui::Separator();
            
            if (ImGui::MenuItem("Salir", "Alt+F4")) {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Ayuda")) {
            if (ImGui::MenuItem("Acerca de STL Renderer")) {
                // Mostramos una ventana modal con información sobre la aplicación
                ImGui::OpenPopup("Acerca de STL Renderer");
            }
            
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
    
    // Ventana modal para "Acerca de"
    if (ImGui::BeginPopupModal("Acerca de STL Renderer", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("STL Renderer %s", STL_RENDERER_VERSION);
        ImGui::Separator();
        ImGui::Text("Una aplicación para convertir archivos STL a imágenes PNG.");
        ImGui::Text("Características principales:");
        ImGui::BulletText("Conversión rápida de archivos STL a PNG");
        ImGui::BulletText("Soporte para fondos transparentes");
        ImGui::BulletText("Personalización de colores del modelo y fondo");
        ImGui::BulletText("Control preciso de la cámara");
        ImGui::BulletText("Procesamiento por lotes");
        ImGui::BulletText("Arrastrar y soltar archivos");
        ImGui::Separator();
        ImGui::Text("Desarrollado con ❤️ por Synrg Studio");
        ImGui::Text("Copyright (C) 2023");
        
        if (ImGui::Button("Cerrar", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Gui::renderCameraControls() {
    if (ImGui::CollapsingHeader("Controles de cámara", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Opciones de visualización que estaban en el menú "Ver"
        ImGui::Checkbox("Mostrar información del modelo", &m_showModelInfo);
        #ifdef _DEBUG
        ImGui::Checkbox("ImGui Demo", &m_showDemoWindow);
        #endif
        
        ImGui::Separator();
        
        // Controles de yaw, pitch y distancia más precisos
        bool changed = false;
        
        // Mostrar sliders con exactamente el mismo rango que los controles de ratón
        // para asegurar consistencia
        changed |= ImGui::SliderFloat("Rotación X", &m_cameraYaw, -3.14f, 3.14f, "%.2f");
        
        // Limitar exactamente igual que el control de ratón
        float limitedPitch = m_cameraPitch;
        if (ImGui::SliderFloat("Rotación Y", &limitedPitch, -1.5f, 1.5f, "%.2f")) {
            m_cameraPitch = limitedPitch;
            changed = true;
        }
        
        changed |= ImGui::SliderFloat("Distancia", &m_cameraDistance, 1.0f, 20.0f, "%.1f");
        
        if (ImGui::Button("Restaurar Vista Predeterminada")) {
            // Restaurar valores optimizados
            m_cameraYaw = 0.8f;
            m_cameraPitch = 0.5f;
            m_cameraDistance = 2.0f;
            changed = true;
        }
        
        // Aplicar cambios si hubo alguno
        if (changed) {
            try {
                // Actualizar configuración
                m_app.getConfig().cameraYaw = m_cameraYaw;
                m_app.getConfig().cameraPitch = m_cameraPitch;
                m_app.getConfig().cameraDistance = m_cameraDistance;
                
                // Aplicar al renderer
                m_app.getRenderer().setCameraOrbit(m_cameraYaw, m_cameraPitch, m_cameraDistance);
            } catch (...) {
                std::cout << "Error al aplicar cambios de cámara desde slider\n";
            }
        }
    }
}

void Gui::renderColorControls() {
    if (ImGui::CollapsingHeader("Colores", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3));
        
        // Color del modelo
        float modelColor[3] = {
            m_app.getConfig().modelColor.r,
            m_app.getConfig().modelColor.g,
            m_app.getConfig().modelColor.b
        };
        
        // Display color swatch before the TreeNode
        ImGui::PushID("ModelColorTree");
        ImGui::AlignTextToFramePadding();
        
        // Create the color swatch
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + 20, p.y + ImGui::GetFrameHeight()), 
                                                 ImGui::ColorConvertFloat4ToU32(ImVec4(modelColor[0], modelColor[1], modelColor[2], 1.0f)));
        ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + 20, p.y + ImGui::GetFrameHeight()), 
                                           ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f)));
        ImGui::Dummy(ImVec2(20, ImGui::GetFrameHeight()));
        ImGui::SameLine(0.0f, 5.0f);
        
        // Color picker for object inside TreeNode
        if (ImGui::TreeNodeEx("Color del objeto", ImGuiTreeNodeFlags_None)) {
            ImGui::ColorPicker3("##ModelColorPicker", modelColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_PickerHueWheel);
            
            // Actualizar color en la configuración cuando cambia
            Color newColor(modelColor[0], modelColor[1], modelColor[2]);
            m_app.setModelColor(newColor);
            
            ImGui::TreePop();
        }
        ImGui::PopID();
        
        ImGui::Spacing();
        
        // Color de fondo
        float bgColor[3] = {
            m_app.getConfig().backgroundColor.r,
            m_app.getConfig().backgroundColor.g,
            m_app.getConfig().backgroundColor.b
        };
        
        // Display color swatch before the TreeNode
        ImGui::PushID("BgColorTree");
        ImGui::AlignTextToFramePadding();
        
        // Create the color swatch
        p = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + 20, p.y + ImGui::GetFrameHeight()), 
                                                 ImGui::ColorConvertFloat4ToU32(ImVec4(bgColor[0], bgColor[1], bgColor[2], 1.0f)));
        ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + 20, p.y + ImGui::GetFrameHeight()), 
                                           ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f)));
        ImGui::Dummy(ImVec2(20, ImGui::GetFrameHeight()));
        ImGui::SameLine(0.0f, 5.0f);
        
        // Color picker for background inside TreeNode
        if (ImGui::TreeNodeEx("Color del fondo", ImGuiTreeNodeFlags_None)) {
            ImGui::ColorPicker3("##BgColorPicker", bgColor, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_PickerHueWheel);
            
            // Actualizar color en la configuración cuando cambia
            Color newColor(bgColor[0], bgColor[1], bgColor[2]);
            m_app.setBackgroundColor(newColor);
            
            ImGui::TreePop();
        }
        ImGui::PopID();
        
        ImGui::PopStyleVar();
        
        ImGui::Spacing();
        
        // Opción de fondo transparente para guardar imágenes
        bool transparentBg = m_app.getConfig().transparentBackground;
        if (ImGui::Checkbox("Fondo transparente al guardar", &transparentBg)) {
            // Actualizar configuración
            m_app.getConfig().transparentBackground = transparentBg;
        }
        
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Esta opción solo afecta a las imágenes guardadas, no a la vista previa");
        }
    }
}

void Gui::renderBatchControls() {
    if (ImGui::CollapsingHeader("Procesamiento por lotes", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Selección de carpeta
        ImGui::Text("Carpeta: %s", m_batchDirectory.empty() ? "<ninguna>" : m_batchDirectory.c_str());
        
        if (ImGui::Button("Seleccionar carpeta")) {
            const std::string folder = folderDialog(m_window);
            if (!folder.empty()) {
                m_batchDirectory = folder;
                m_app.getConfig().batchDirectory = m_batchDirectory;
            }
        }

        ImGui::SameLine();
        
        // Botón para procesar todos los archivos
        if (ImGui::Button("Procesar carpeta")) {
            if (!m_batchDirectory.empty()) {
                m_processingBatch = true;
                
                // Iniciar procesamiento
                m_app.renderDirectory(m_batchDirectory);
                
                m_processingBatch = false;
            }
        }
        
        // Mostrar progreso si está procesando
        if (m_processingBatch) {
            ImGui::ProgressBar(0.5f, ImVec2(-1.0f, 0.0f), "Procesando archivos...");
        }
    }
}

void Gui::renderPreviewWindow() {
    // Crear un área para la vista previa del modelo
    ImVec2 availSize = ImGui::GetContentRegionAvail();
    
    // Área de renderizado (solo definimos el espacio, no renderizamos aquí)
    ImGui::BeginChild("ModelPreview", ImVec2(availSize.x, availSize.y - 30), true, ImGuiWindowFlags_NoScrollbar);
    
    // Capturar la posición y tamaño de la ventana para el renderizado posterior
    m_previewWindowPos = ImGui::GetWindowPos();
    m_previewWindowSize = ImGui::GetWindowSize();
    
    // Obtener posición y tamaño de la ventana para el control de la cámara con el mouse
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    // Coordenadas del mouse
    ImVec2 mousePos = ImGui::GetMousePos();
    bool mouseInWindow = (mousePos.x >= windowPos.x && mousePos.x < windowPos.x + windowSize.x &&
                         mousePos.y >= windowPos.y && mousePos.y < windowPos.y + windowSize.y);
    
    // Comprobar clic del ratón (clic izquierdo para rotar)
    if (mouseInWindow && ImGui::IsMouseDown(0)) {
        // Si estamos comenzando a arrastrar, guardar posición inicial
        if (!m_isMouseDragging) {
            m_isMouseDragging = true;
            m_lastMouseX = mousePos.x;
            m_lastMouseY = mousePos.y;
        } else {
            // Calcular movimiento del ratón
            float deltaX = mousePos.x - m_lastMouseX;
            float deltaY = mousePos.y - m_lastMouseY;
            
            // Actualizar ángulos de la cámara
            m_cameraYaw += deltaX * 0.01f;  // Aumentado para hacerlo más sensible
            m_cameraPitch += deltaY * 0.01f; // Invertido el signo para un control más intuitivo
            
            // Limitar el pitch para evitar el giro completo
            m_cameraPitch = std::max(-1.5f, std::min(1.5f, m_cameraPitch));
            
            // Actualizar posición anterior del ratón
            m_lastMouseX = mousePos.x;
            m_lastMouseY = mousePos.y;
            
            // Aplicar los cambios al renderer
            try {
                // Actualizar config
                m_app.getConfig().cameraYaw = m_cameraYaw;
                m_app.getConfig().cameraPitch = m_cameraPitch;
                m_app.getConfig().cameraDistance = m_cameraDistance;
                
                // Aplicar al renderer
                m_app.getRenderer().setCameraOrbit(m_cameraYaw, m_cameraPitch, m_cameraDistance);
            }
            catch (...) {
                std::cout << "Error al actualizar cámara desde mouse\n";
            }
        }
    } else {
        m_isMouseDragging = false;
    }
    
    // Zoom con rueda de ratón
    if (mouseInWindow && ImGui::GetIO().MouseWheel != 0) {
        // Actualizar distancia de la cámara
        m_cameraDistance -= ImGui::GetIO().MouseWheel * 0.1f;
        
        // Limitar distancia
        m_cameraDistance = std::max(1.0f, std::min(20.0f, m_cameraDistance));
        
        // Actualizar configuración y renderer
        try {
            m_app.getConfig().cameraDistance = m_cameraDistance;
            m_app.getRenderer().setCameraOrbit(m_cameraYaw, m_cameraPitch, m_cameraDistance);
        }
        catch (...) {
            std::cout << "Error al actualizar zoom de cámara\n";
        }
    }
    
    // Mostrar instrucciones de control
    ImGui::SetCursorPos(ImVec2(10, 10));
    ImGui::Text("Controles: Arrastra para rotar | Rueda para zoom");
    
    ImGui::EndChild();
}

void Gui::renderStatusBar() {
    ImGui::Separator();
    
    // Mostrar información en la barra de estado
    if (!m_currentFile.empty()) {
        ImGui::Text("Modelo: %s | Salida: %s",
                    fs::u8path(m_currentFile).filename().u8string().c_str(),
                    fs::u8path(m_saveFile).filename().u8string().c_str());
    } else {
        ImGui::Text("Listo");
    }
}

void Gui::setupCallbacks() {
    // Configurar callbacks adicionales si son necesarios
}

void Gui::updateFromConfig() {
    const AppConfig& config = m_app.getConfig();
    m_bgColor[0] = config.backgroundColor.r;
    m_bgColor[1] = config.backgroundColor.g;
    m_bgColor[2] = config.backgroundColor.b;
    m_modelColor[0] = config.modelColor.r;
    m_modelColor[1] = config.modelColor.g;
    m_modelColor[2] = config.modelColor.b;
    m_cameraYaw = config.cameraYaw;
    m_cameraPitch = config.cameraPitch;
    m_cameraDistance = config.cameraDistance;
    m_batchDirectory = config.batchDirectory;
}

void Gui::applyToConfig() {
    AppConfig& config = m_app.getConfig();
    config.backgroundColor = Color(m_bgColor[0], m_bgColor[1], m_bgColor[2]);
    config.modelColor = Color(m_modelColor[0], m_modelColor[1], m_modelColor[2]);
    config.cameraYaw = m_cameraYaw;
    config.cameraPitch = m_cameraPitch;
    config.cameraDistance = m_cameraDistance;
    config.batchDirectory = m_batchDirectory;
}

void Gui::dropCallback(GLFWwindow*, int count, const char** paths) {
    Gui* gui = g_CallbackData.gui;
    if (!gui || !g_CallbackData.app) return;
    for (int i = 0; i < count; ++i) {
        const std::string path = paths[i];
        const fs::path filePath = fs::u8path(path);
        std::error_code ec;
        if (fs::is_directory(filePath, ec)) {
            g_CallbackData.app->renderDirectory(path);
            continue;
        }
        if (!isStlPath(filePath)) {
            std::cerr << "Skipping non-STL file: " << path << '\n';
            continue;
        }
        gui->m_currentFile = path;
        gui->m_saveFile = defaultOutputPath(filePath).u8string();
        g_CallbackData.app->renderSingleFile(gui->m_currentFile, gui->m_saveFile);
    }
}

void Gui::openFileDialog() {
    const std::string file = fileDialog(m_window, false);
    if (file.empty()) return;
    m_currentFile = file;
    m_saveFile = defaultOutputPath(fs::u8path(file)).u8string();
    m_app.renderSingleFile(m_currentFile, m_saveFile);
}

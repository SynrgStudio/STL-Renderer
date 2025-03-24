#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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

// Datos para compartir entre GLFW y las funciones de callback
struct GUICallbackData {
    App* app;
    Gui* gui;
};

static GUICallbackData g_CallbackData;

Gui::Gui(App& app)
    : m_app(app)
    , m_renderer(nullptr)
    , m_stlLoader(nullptr)
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
    , m_cameraDistance(5.0f)
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
    
    std::cout << "Gui construida con versión original\n";
}

Gui::Gui(Renderer& renderer, StlLoader& stlLoader)
    : m_app(*reinterpret_cast<App*>(0))  // Esto es un hack para compilar, no debería usarse la m_app
    , m_renderer(&renderer)
    , m_stlLoader(&stlLoader)
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
    , m_cameraDistance(5.0f)
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
    
    std::cout << "Gui construida con versión alternativa\n";
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
        std::cout << "Gui::initialize() - Error: No hay contexto GLFW activo\n";
        return false;
    }
    
    std::cout << "Gui::initialize() - Iniciando interfaz gráfica\n";
    
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
    
    std::cout << "Gui::initialize() - Interfaz gráfica inicializada correctamente\n";
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
            if (ImGui::MenuItem("Abrir STL...", "Ctrl+O")) {
                // Mostrar diálogo para abrir archivo
                char filename[MAX_PATH] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = glfwGetWin32Window(m_window);
                ofn.lpstrFilter = "STL Files\0*.stl\0All Files\0*.*\0";
                ofn.lpstrFile = filename;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                ofn.lpstrDefExt = "stl";
                
                if (GetOpenFileNameA(&ofn)) {
                    m_currentFile = filename;
                    
                    // Generar nombre de archivo de salida
                    fs::path outputPath = m_currentFile;
                    outputPath.replace_extension();
                    outputPath += "_png.png";
                    m_saveFile = outputPath.string();
                    
                    // Cargar y renderizar
                    m_app.renderSingleFile(m_currentFile, m_saveFile);
                }
            }
            
            if (ImGui::MenuItem("Guardar imagen...", "Ctrl+S")) {
                // Mostrar diálogo para guardar imagen
                char filename[MAX_PATH] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = glfwGetWin32Window(m_window);
                ofn.lpstrFilter = "PNG Images\0*.png\0All Files\0*.*\0";
                ofn.lpstrFile = filename;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
                ofn.lpstrDefExt = "png";
                
                if (GetSaveFileNameA(&ofn)) {
                    m_saveFile = filename;
                    
                    // Renderizar al archivo seleccionado
                    if (!m_currentFile.empty()) {
                        m_app.renderSingleFile(m_currentFile, m_saveFile);
                    }
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
        ImGui::Text("STL Renderer v1.0.0");
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
            m_cameraDistance = 5.0f;
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
            
            std::cout << "Opción de fondo transparente cambiada a: " << (transparentBg ? "Activado" : "Desactivado") << std::endl;
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
            // Mostrar diálogo para seleccionar carpeta
            char path[MAX_PATH] = {0};
            BROWSEINFOA bi = {0};
            bi.lpszTitle = "Seleccionar carpeta con archivos STL";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
            
            LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
            if (pidl != NULL) {
                if (SHGetPathFromIDListA(pidl, path)) {
                    m_batchDirectory = path;
                    m_app.getConfig().batchDirectory = m_batchDirectory;
                }
                
                CoTaskMemFree(pidl);
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
                    fs::path(m_currentFile).filename().string().c_str(),
                    fs::path(m_saveFile).filename().string().c_str());
    } else {
        ImGui::Text("Listo");
    }
}

void Gui::setupCallbacks() {
    // Configurar callbacks adicionales si son necesarios
}

void Gui::updateFromConfig() {
    // Si estamos usando la versión con App, usar la configuración de la App
    if (m_renderer == nullptr) {
        // Versión original con App
        try {
            AppConfig& config = m_app.getConfig();
            
            // Colores
            m_bgColor[0] = config.backgroundColor.r;
            m_bgColor[1] = config.backgroundColor.g;
            m_bgColor[2] = config.backgroundColor.b;
            
            m_modelColor[0] = config.modelColor.r;
            m_modelColor[1] = config.modelColor.g;
            m_modelColor[2] = config.modelColor.b;
            
            // Cámara
            m_cameraYaw = config.cameraYaw;
            m_cameraPitch = config.cameraPitch;
            m_cameraDistance = config.cameraDistance;
            
            // Batch processing
            m_batchDirectory = config.batchDirectory;
        } catch (...) {
            std::cout << "Error al intentar acceder a la configuración a través de App\n";
        }
    } else {
        // Versión alternativa con renderer y stlLoader directos
        // Aquí no tenemos acceso a la configuración, así que usamos valores por defecto
        std::cout << "Usando la versión alternativa sin acceso a AppConfig\n";
        
        // Valores por defecto ya establecidos en el constructor
    }
}

void Gui::applyToConfig() {
    // Si estamos usando la versión con App, actualizar la configuración de la App
    if (m_renderer == nullptr) {
        // Versión original con App
        try {
            AppConfig& config = m_app.getConfig();
            
            // Colores
            config.backgroundColor = Color(m_bgColor[0], m_bgColor[1], m_bgColor[2]);
            config.modelColor = Color(m_modelColor[0], m_modelColor[1], m_modelColor[2]);
            
            // Cámara
            config.cameraYaw = m_cameraYaw;
            config.cameraPitch = m_cameraPitch;
            config.cameraDistance = m_cameraDistance;
            
            // Batch processing
            config.batchDirectory = m_batchDirectory;
        } catch (...) {
            std::cout << "Error al intentar actualizar la configuración a través de App\n";
        }
    } else {
        // Versión alternativa con renderer y stlLoader directos
        // Aplicar directamente al renderer los valores que podemos
        std::cout << "Aplicando configuración directamente al renderer\n";
        
        if (m_renderer) {
            m_renderer->setBackgroundColor(Color(m_bgColor[0], m_bgColor[1], m_bgColor[2]));
            m_renderer->setModelColor(Color(m_modelColor[0], m_modelColor[1], m_modelColor[2]));
            // También podríamos intentar aplicar las configuraciones de cámara si hay métodos disponibles
        }
    }
}

void Gui::dropCallback(GLFWwindow* window, int count, const char** paths) {
    std::cout << "========== INICIO PROCESAMIENTO ARRASTRE ==========" << std::endl;
    std::cout << "Archivos arrastrados: " << count << std::endl;
    std::cout << "========== PROCESANDO " << count << " ARCHIVOS ARRASTRADOS ==========" << std::endl;
    
    // Procesar solo si tenemos acceso a la aplicación y hay archivos
    if (count <= 0 || !g_CallbackData.app || !g_CallbackData.gui) {
        std::cout << "No hay archivos para procesar o faltan referencias necesarias" << std::endl;
        return;
    }
    
    // Recorrer cada archivo arrastrado
    for (int i = 0; i < count; i++) {
        std::string path = paths[i];
        fs::path filePath(path);
        
        std::cout << "---------- ARCHIVO " << (i+1) << "/" << count << " ----------" << std::endl;
        std::cout << "Ruta: " << path << std::endl;
        std::cout << "Procesando archivo " << (i+1) << "/" << count << ": " << filePath.filename().string() << std::endl;
        
        // Verificar si es un archivo STL
        if (filePath.extension() != ".stl") {
            std::cout << "ERROR: No es un archivo STL válido" << std::endl;
            std::cerr << "✗ Archivo no soportado: " << filePath.filename().string() << ". Solo se aceptan archivos .stl" << std::endl;
            continue; // Saltar al siguiente archivo
        }
        
        // Generar nombre de archivo de salida
        fs::path outputPath = filePath;
        outputPath.replace_extension();
        outputPath += "_png.png";
        std::string outputFile = outputPath.string();
        
        // Actualizar la interfaz con el archivo actual
        g_CallbackData.gui->m_currentFile = path;
        g_CallbackData.gui->m_saveFile = outputFile;
        
        std::cout << "Archivo STL: " << filePath.filename().string() << std::endl;
        std::cout << "Imagen salida: " << outputPath.filename().string() << std::endl;
        
        // Paso 1: Cargar el modelo actual
        std::cout << "Cargando modelo..." << std::endl;
        if (!g_CallbackData.app->loadModel(path)) {
            std::cout << "ERROR: No se pudo cargar el modelo" << std::endl;
            std::cerr << "✗ Error al cargar el modelo: " << filePath.filename().string() << std::endl;
            continue; // Saltar al siguiente archivo
        }
        
        std::cout << "Modelo cargado exitosamente" << std::endl;
        
        // Paso 2: Centrar la cámara para este modelo específico
        std::cout << "Centrando cámara..." << std::endl;
        g_CallbackData.app->getRenderer().centerCamera();
        
        // Paso 3: Aplicar configuración de cámara
        float yaw = g_CallbackData.app->getConfig().cameraYaw;
        float pitch = g_CallbackData.app->getConfig().cameraPitch;
        float distance = g_CallbackData.app->getConfig().cameraDistance;
        
        std::cout << "Aplicando configuración de cámara: yaw=" << yaw << ", pitch=" << pitch << ", distance=" << distance << std::endl;
        g_CallbackData.app->getRenderer().setCameraOrbit(yaw, pitch, distance);
        
        // Paso 4: Guardar la imagen
        std::cout << "Renderizando a archivo: " << outputFile << std::endl;
        if (!g_CallbackData.app->saveImage(outputFile)) {
            std::cout << "ERROR: No se pudo guardar la imagen" << std::endl;
            std::cerr << "✗ Error al guardar la imagen: " << outputPath.filename().string() << std::endl;
            continue; // Saltar al siguiente archivo
        }
        
        std::cout << "ÉXITO: Imagen guardada en " << outputFile << std::endl;
        std::cout << "✓ Imagen guardada: " << outputPath.filename().string() << std::endl;
    }
    
    std::cout << "========== FIN PROCESAMIENTO ARRASTRE ==========" << std::endl;
    std::cout << "Procesamiento completado." << std::endl;
}

void Gui::openFileDialog() {
    std::cout << "Gui::openFileDialog() - Mostrando diálogo para abrir archivo\n";
    
    // Mostrar diálogo para abrir archivo
    char filename[MAX_PATH] = {0};
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = glfwGetWin32Window(m_window);
    ofn.lpstrFilter = "STL Files\0*.stl\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "stl";
    
    if (GetOpenFileNameA(&ofn)) {
        std::cout << "Archivo seleccionado: " << filename << "\n";
        m_currentFile = filename;
        
        // Generar nombre de archivo de salida
        fs::path outputPath = m_currentFile;
        outputPath.replace_extension();
        outputPath += "_png.png";
        m_saveFile = outputPath.string();
        
        // Cargar y renderizar
        if (m_app.renderSingleFile(m_currentFile, m_saveFile)) {
            std::cout << "Modelo renderizado correctamente\n";
        } else {
            std::cout << "Error al renderizar modelo\n";
        }
    } else {
        std::cout << "Diálogo cancelado o error al abrir\n";
    }
} 
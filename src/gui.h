#pragma once

#include <string>
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Forward declarations
class App;
class Renderer;
class StlLoader;
struct AppConfig;

class Gui {
public:
    // Constructor que toma una referencia a la App o a sus componentes dependiendo de la versión
    Gui(App& app);
    Gui(Renderer& renderer, StlLoader& stlLoader);
    
    // Destructor
    ~Gui();
    
    // Inicialización
    bool initialize();
    
    // Procesamiento de eventos
    void processEvents();
    
    // Renderizado de interfaz
    void render();
    
    // Verificar si se debe cerrar la ventana
    bool shouldClose() const;
    
    // Abrir diálogo de selección de archivo
    void openFileDialog();
    
    // Métodos para obtener configuración de cámara
    float getCameraYaw() const { return m_cameraYaw; }
    float getCameraPitch() const { return m_cameraPitch; }
    float getCameraDistance() const { return m_cameraDistance; }
    
private:
    // Referencia a la aplicación
    App& m_app;
    
    // Referencias adicionales para la versión alternativa
    Renderer* m_renderer = nullptr;
    StlLoader* m_stlLoader = nullptr;
    
    // Ventana GLFW
    GLFWwindow* m_window;
    
    // Estado de la interfaz
    bool m_showDemoWindow;
    bool m_showColorPicker;
    bool m_showCameraControls;
    bool m_showModelInfo;
    bool m_showBatchProcessing;
    
    // Archivos
    std::string m_currentFile;
    std::string m_saveFile;
    std::string m_batchDirectory;
    bool m_processingBatch;
    
    // Configuración de cámara
    float m_cameraYaw;
    float m_cameraPitch;
    float m_cameraDistance;
    
    // Control de cámara con ratón
    bool m_isMouseDragging = false;
    float m_lastMouseX = 0.0f;
    float m_lastMouseY = 0.0f;
    bool m_isMouseInPreviewWindow = false;
    
    // Almacenar la posición y tamaño de la ventana de vista previa para renderizado
    ImVec2 m_previewWindowPos;
    ImVec2 m_previewWindowSize;
    
    // Colores
    float m_bgColor[3];
    float m_modelColor[3];
    
    // Métodos de renderizado por secciones
    void renderMainMenu();
    void renderCameraControls();
    void renderColorControls();
    void renderBatchControls();
    void renderPreviewWindow();
    void renderStatusBar();
    
    // Configuración de callbacks
    void setupCallbacks();
    
    // Sincronización con la configuración de la aplicación
    void updateFromConfig();
    void applyToConfig();
    
    // Callback para arrastrar y soltar archivos
    static void dropCallback(GLFWwindow* window, int count, const char** paths);
}; 
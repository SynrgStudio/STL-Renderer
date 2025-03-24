#pragma once

#include "renderer.h"
#include "stl_loader.h"
#include "gui.h"
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>

// Estructura de configuración de la aplicación
struct AppConfig {
    // Configuración de renderizado
    Color backgroundColor = Color(0.0f, 0.0f, 0.0f);
    Color modelColor = Color(0.7f, 0.7f, 0.7f);
    float cameraYaw = 0.0f;
    float cameraPitch = 0.0f;
    float cameraDistance = 5.0f;
    
    // Configuración de imagen
    int outputWidth = 1024;
    int outputHeight = 1024;
    bool transparentBackground = true;
    
    // Configuración de batch processing
    std::string batchDirectory = "";
};

class App {
public:
    // Constructor, recibe un parámetro que indica si se ejecuta en modo silencioso
    App(bool silentMode = false);
    // Destructor
    ~App();
    
    // Analiza los argumentos de línea de comandos e inicializa la aplicación
    bool parseArguments(int argc, char* argv[]);
    
    // Ejecuta la aplicación
    void run();
    
    // Ejecución específica
    int run(int argc, char* argv[]);
    int runInteractive();
    
    // Operaciones de archivos
    bool loadModel(const std::string& filename);
    bool saveImage(const std::string& outputFile);
    bool processDirectory(const std::string& directory);
    bool renderSingleFile(const std::string& inputFile, const std::string& outputFile);
    bool renderDirectory(const std::string& directory);
    
    // Operaciones de configuración
    bool loadConfig();
    void saveConfig();
    
    // Acceso a configuración
    AppConfig& getConfig() { return m_config; }
    
    // Acceso al renderer
    Renderer& getRenderer() { return *m_renderer; }
    
    // Métodos para establecer colores
    void setModelColor(const Color& color);
    void setBackgroundColor(const Color& color);
    
private:
    // Métodos privados
    bool init();
    void cleanup();
    
    // Utilidades
    std::string getCurrentTimestamp();
    Color parseColor(const std::string& colorStr);
    
    // Funciones para manejo de cámara
    void centerCameraIfNeeded();
    void updateRendererCamera();
    
    // Procesa los argumentos de línea de comandos
    bool processCommandLine(int argc, char* argv[]);
    
    // Inicializa los componentes
    bool initialize();
    
    // Limpia los recursos y cierra la aplicación
    void shutdown();
    
    // Muestra la ayuda
    void printHelp();
    
    // Muestra la versión
    void printVersion();
    
    // Ejecuta en modo silencioso (sin GUI)
    void runSilentMode();
    
    // Ejecuta en modo GUI
    void runGuiMode();
    
    // Componentes de la aplicación
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<StlLoader> m_stlLoader;
    std::unique_ptr<Gui> m_gui;
    
    // Configuración
    AppConfig m_config;
    bool m_silentMode = false;      // Modo silencioso
    bool m_running = false;         // Estado de ejecución
    std::string m_outputFile;       // Archivo de salida
    std::vector<std::string> m_inputFiles;  // Archivos de entrada
}; 
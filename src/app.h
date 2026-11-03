#pragma once

#include "renderer.h"
#include "stl_loader.h"
#include "gui.h"
#include <filesystem>
#include <memory>
#include <string>

// Configuración persistente de la aplicación (config.ini junto al ejecutable).
struct AppConfig {
    Color backgroundColor = Color(0.2f, 0.3f, 0.3f);
    Color modelColor = Color(1.0f, 0.5f, 0.2f);
    float cameraYaw = 0.8f;
    float cameraPitch = 0.5f;
    float cameraDistance = 2.0f;
    int outputWidth = 1024;
    int outputHeight = 1024;
    bool transparentBackground = false;
    std::string batchDirectory;
};

class App {
public:
    App();
    ~App();
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    // Punto de entrada: interpreta la línea de comandos (argv en UTF-8).
    int run(int argc, char* argv[]);

    // Operaciones usadas también por la GUI. Todas las rutas son UTF-8.
    bool loadModel(const std::string& filename);
    bool saveImage(const std::string& outputFile);
    bool renderSingleFile(const std::string& inputFile, const std::string& outputFile);
    bool renderDirectory(const std::string& directory);

    AppConfig& getConfig() { return m_config; }
    Renderer& getRenderer() { return *m_renderer; }
    void setModelColor(const Color& color);
    void setBackgroundColor(const Color& color);

private:
    int runInteractive();
    void updateRendererCamera();
    bool loadConfig();
    void saveConfig() const;
    static void printHelp();
    static void printVersion();

    std::filesystem::path m_configPath;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<StlLoader> m_stlLoader;
    std::unique_ptr<Gui> m_gui;
    AppConfig m_config;
};

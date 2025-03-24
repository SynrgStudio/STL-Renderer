#include "app.h"
#include "renderer.h"
#include "stl_loader.h"
#include "gui.h"

#include <iostream>
#include <filesystem>
#include <windows.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <shellapi.h>

namespace fs = std::filesystem;

std::ofstream logFile;

App::App(bool silentMode) : m_silentMode(silentMode) {
    // Crear archivo de log con timestamp
    std::string timestamp = getCurrentTimestamp();
    std::string logname = "stlrenderer_" + timestamp + ".log";
    logFile.open(logname);
    
    if (!logFile.is_open()) {
        std::cerr << "Error: No se pudo crear el archivo de log!" << std::endl;
    }
    
    logFile << "=== Iniciando STL Renderer v1.0 ===" << "\n";
    logFile << "Modo: " << (m_silentMode ? "Silencioso" : "Interactivo") << "\n";
    logFile << "Timestamp: " << timestamp << "\n\n";
    
    // Cargar configuración
    loadConfig();
    
    // Valores por defecto si no se cargan de archivo
    if (m_config.backgroundColor.r == 0 && m_config.backgroundColor.g == 0 && m_config.backgroundColor.b == 0) {
        m_config.backgroundColor = Color(0.2f, 0.3f, 0.3f);
    }
    
    if (m_config.modelColor.r == 0 && m_config.modelColor.g == 0 && m_config.modelColor.b == 0) {
        m_config.modelColor = Color(1.0f, 0.5f, 0.2f);
    }
    
    // Inicializar componentes
    init();
}

App::~App() {
    // Guardar configuración antes de cerrar
    saveConfig();
    
    // Limpiar recursos
    cleanup();
    
    logFile << "App destruida correctamente\n";
    logFile << "==============================================\n\n";
    logFile.close();
}

bool App::init() {
    logFile << "App::init() - Inicializando componentes\n";
    
    // Crear e inicializar el renderer
    m_renderer = std::make_unique<Renderer>();
    
    // Inicializar el loader de STL
    m_stlLoader = std::make_unique<StlLoader>();
    
    // Aplicar configuración
    if (m_renderer) {
        // Configurar colores
        Color bgColor(m_config.backgroundColor.r, m_config.backgroundColor.g, m_config.backgroundColor.b);
        Color modelColor(m_config.modelColor.r, m_config.modelColor.g, m_config.modelColor.b);
        
        m_renderer->setBackgroundColor(bgColor);
        m_renderer->setModelColor(modelColor);
        
        logFile << "Renderer configurado con colores: BG(" 
                << bgColor.r << ", " << bgColor.g << ", " << bgColor.b 
                << "), Modelo(" << modelColor.r << ", " << modelColor.g << ", " << modelColor.b << ")\n";
    }
    
    return true;
}

void App::cleanup() {
    // Liberar recursos en orden inverso
    m_gui.reset();
    m_stlLoader.reset();
    m_renderer.reset();
    
    // Recursos de la aplicación liberados
}

int App::run(int argc, char* argv[]) {
    // Inicializar configuración
    loadConfig();
    
    logFile << "App::run() - Iniciando con " << argc << " argumentos\n";
    std::cout << "STL Renderer iniciado" << std::endl;
    
    // Verificar modo de operación según argumentos
    // Si no hay argumentos, iniciar en modo interactivo
    if (argc <= 1) {
        logFile << "Iniciando en modo interactivo\n";
        std::cout << "Iniciando en modo interactivo..." << std::endl;
        return runInteractive();
    }
    
    // Inicializar el renderer en modo headless para renderizado sin ventana
    if (!m_renderer->initializeHeadless(m_config.outputWidth, m_config.outputHeight)) {
        logFile << "Error al inicializar el renderer en modo headless\n";
        std::cerr << "Error al inicializar el renderer" << std::endl;
        return -1;
    }
    
    logFile << "Renderer inicializado correctamente\n";
    
    // MODIFICACIÓN: Comprobar si hay múltiples archivos STL
    std::vector<std::string> stlFiles;
    
    // Comprobar cada argumento para ver si son archivos STL válidos
    for (int i = 1; i < argc; i++) {
        fs::path path(argv[i]);
        if (fs::exists(path) && path.extension() == ".stl") {
            stlFiles.push_back(argv[i]);
            logFile << "Archivo STL detectado: " << argv[i] << "\n";
        }
    }
    
    // Si hay múltiples archivos STL, procesarlos como conjunto
    if (stlFiles.size() > 1) {
        logFile << "===== PROCESANDO " << stlFiles.size() << " ARCHIVOS STL ARRASTRADOS =====\n";
        std::cout << "Procesando " << stlFiles.size() << " archivos STL arrastrados" << std::endl;
        
        int filesProcessed = 0;
        int filesSuccess = 0;
        
        // Procesar cada archivo como lo haría renderDirectory
        for (const auto& filePath : stlFiles) {
            fs::path path(filePath);
            fs::path outputPath = path;
            outputPath.replace_extension();
            outputPath += "_png.png";
            
            logFile << "Procesando: " << filePath << " -> " << outputPath.string() << "\n";
            std::cout << "Procesando: " << path.filename().string() << std::endl;
            
            filesProcessed++;
            if (renderSingleFile(filePath, outputPath.string())) {
                filesSuccess++;
                std::cout << "✓ Imagen guardada: " << outputPath.filename().string() << std::endl;
            } else {
                std::cerr << "✗ Error al procesar: " << path.filename().string() << std::endl;
            }
        }
        
        logFile << "Procesamiento completado. " << filesSuccess << "/" << filesProcessed 
                << " archivos procesados correctamente\n";
        std::cout << "Procesamiento completado. " << filesSuccess << "/" << filesProcessed 
                << " archivos procesados correctamente." << std::endl;
        
        saveConfig();
        return filesSuccess > 0 ? 0 : -1;
    }
    // Procesar un solo archivo (código original)
    else if (stlFiles.size() == 1) {
        std::string inputFile = stlFiles[0];
        fs::path path(inputFile);
        
        // Generar nombre para el archivo de salida
        fs::path outputPath = path;
        outputPath.replace_extension();
        outputPath += "_render.png";
        
        // Renderizar archivo
        bool success = renderSingleFile(inputFile, outputPath.string());
        logFile << "Resultado del renderizado: " << (success ? "ÉXITO" : "ERROR") << "\n";
        
        saveConfig();
        return success ? 0 : -1;
    }
    // Si llegamos aquí, no se encontraron archivos STL válidos
    else {
        logFile << "No se encontraron archivos STL válidos entre los argumentos\n";
        std::cerr << "No se encontraron archivos STL válidos" << std::endl;
        return -1;
    }
}

int App::runInteractive() {
    logFile << "App::runInteractive() - Iniciando interfaz gráfica\n";
    std::cout << "Iniciando interfaz gráfica..." << std::endl;
    
    // Inicializar renderer con una ventana más grande para mejor visualización
    if (!m_renderer->initialize(1024, 768)) {
        logFile << "Error al inicializar el renderer en modo interactivo\n";
        std::cerr << "Error al inicializar el renderer" << std::endl;
        return -1;
    }
    
    logFile << "Renderer inicializado correctamente en modo interactivo\n";
    std::cout << "Renderer inicializado correctamente" << std::endl;
    
    // Inicializar GUI (crearla primero)
    m_gui = std::make_unique<Gui>(*this);
    
    if (!m_gui->initialize()) {
        logFile << "Error al inicializar la interfaz gráfica\n";
        std::cerr << "Error al inicializar la interfaz gráfica" << std::endl;
        return -1;
    }
    
    logFile << "GUI inicializada correctamente\n";
    std::cout << "GUI inicializada correctamente" << std::endl;
    
    // Configurar renderer con valores iniciales
    Color bgColor(m_config.backgroundColor.r, m_config.backgroundColor.g, m_config.backgroundColor.b);
    Color modelColor(m_config.modelColor.r, m_config.modelColor.g, m_config.modelColor.b);
    
    m_renderer->setBackgroundColor(bgColor);
    m_renderer->setModelColor(modelColor);
    
    // Establecer un cubo predeterminado si no hay modelo cargado
    centerCameraIfNeeded();
    
    // Bucle principal
    m_running = true;
    logFile << "Entrando en bucle principal\n";
    
    while (!glfwWindowShouldClose(m_renderer->getWindow()) && m_running) {
        // Procesar eventos
        glfwPollEvents();
        
        // Limpiar pantalla (con el color de fondo)
        glClearColor(m_config.backgroundColor.r, m_config.backgroundColor.g, m_config.backgroundColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Aplicar configuración actual de cámara al renderer
        updateRendererCamera();
        
        // Renderizar escena (pero no mostrarla aún, ImGui se dibujará encima)
        m_renderer->render();
        
        // Renderizar GUI encima
        m_gui->render();
        
        // Intercambiar buffers
        glfwSwapBuffers(m_renderer->getWindow());
    }
    
    // Guardar configuración al salir
    saveConfig();
    
    logFile << "Saliendo de la aplicación normalmente\n";
    std::cout << "Aplicación cerrada correctamente" << std::endl;
    return 0;
}

// Función para centrar la cámara si es necesario
void App::centerCameraIfNeeded() {
    if (m_renderer && m_renderer->hasModel()) {
        m_renderer->centerCamera();
        logFile << "Cámara centrada automáticamente para el modelo cargado\n";
    } else if (m_renderer) {
        // Establecer una posición predeterminada para el cubo de vista previa
        m_renderer->setCameraOrbit(0.8f, 0.5f, 3.0f);
        logFile << "Posición de cámara por defecto establecida para vista previa\n";
    }
}

// Función para actualizar la cámara del renderer según la configuración actual
void App::updateRendererCamera() {
    if (!m_renderer) return;
    
    // Si hay una GUI, obtener la configuración de cámara actualizada
    if (m_gui) {
        // Estos métodos deben existir en tu clase Gui
        try {
            m_config.cameraYaw = m_gui->getCameraYaw();
            m_config.cameraPitch = m_gui->getCameraPitch();
            m_config.cameraDistance = m_gui->getCameraDistance();
        } catch (...) {
            logFile << "Error al obtener configuración de cámara desde GUI\n";
        }
    }
    
    // Aplicar la configuración de cámara al renderer
    m_renderer->setCameraOrbit(m_config.cameraYaw, m_config.cameraPitch, m_config.cameraDistance);
}

bool App::loadModel(const std::string& filename) {
    logFile << "App::loadModel() - Cargando: " << filename << "\n";
    std::cout << "Cargando modelo: " << filename << std::endl;
    
    if (m_stlLoader->loadModel(filename, *m_renderer)) {
        logFile << "Modelo cargado correctamente: " << filename << "\n";
        
        // Ajustar cámara automáticamente
        m_renderer->centerCamera();
        logFile << "Cámara centrada\n";
        
        // Aplicar configuración personalizada de cámara si existe
        if (m_config.cameraYaw != 0.0f || m_config.cameraPitch != 0.0f || m_config.cameraDistance != 0.0f) {
            float yaw = m_config.cameraYaw != 0.0f ? m_config.cameraYaw : 0.8f;
            float pitch = m_config.cameraPitch != 0.0f ? m_config.cameraPitch : 0.5f;
            float distance = m_config.cameraDistance != 0.0f ? m_config.cameraDistance : 2.0f;
            
            logFile << "Aplicando configuración de cámara: yaw=" << yaw << ", pitch=" << pitch << ", distance=" << distance << "\n";
            m_renderer->setCameraOrbit(yaw, pitch, distance);
        }
        
        return true;
    }
    
    logFile << "Error al cargar el modelo: " << filename << "\n";
    std::cerr << "Error al cargar el modelo: " << filename << std::endl;
    return false;
}

bool App::saveImage(const std::string& outputFile) {
    logFile << "App::saveImage() - Guardando imagen en: " << outputFile << "\n";
    
    // Verificar que tengamos un renderer y modelo válidos
    if (!m_renderer || !m_renderer->hasModel()) {
        logFile << "Error: No hay un modelo cargado para renderizar\n";
        return false;
    }
    
    // NOTA: Ya no centramos la cámara ni configuramos los parámetros de órbita aquí
    // porque eso lo hacemos en renderSingleFile antes de llamar a esta función
    
    // Configurar renderer con los colores actuales desde la configuración
    Color bgColor(m_config.backgroundColor.r, m_config.backgroundColor.g, m_config.backgroundColor.b);
    Color modelColor(m_config.modelColor.r, m_config.modelColor.g, m_config.modelColor.b);
    
    // Imprimir colores que vamos a usar para debugging
    logFile << "Aplicando colores - Fondo: (" << bgColor.r << ", " << bgColor.g << ", " << bgColor.b 
           << "), Modelo: (" << modelColor.r << ", " << modelColor.g << ", " << modelColor.b << ")\n";
    
    // Aplicar colores al renderer
    m_renderer->setBackgroundColor(bgColor);
    m_renderer->setModelColor(modelColor);
    
    // Renderizar a archivo
    bool success = m_renderer->renderToFile(outputFile, m_config.transparentBackground);
    if (success) {
        logFile << "Imagen guardada correctamente en: " << outputFile << "\n";
    } else {
        logFile << "Error al guardar la imagen: " << outputFile << "\n";
    }
    
    return success;
}

bool App::renderSingleFile(const std::string& inputFile, const std::string& outputFile) {
    logFile << "App::renderSingleFile() - Cargando modelo: " << inputFile << "\n";
    
    // En modo silencio (no interactivo)
    if (m_silentMode) {
        // Inicializar renderer en modo headless
        if (!m_renderer->initializeHeadless(1024, 1024)) {
            std::cerr << "Error al inicializar el renderer en modo headless" << std::endl;
            logFile << "Error al inicializar el renderer en modo headless\n";
            return false;
        }
        
        // Cargar modelo
        if (!loadModel(inputFile)) {
            std::cerr << "Error al cargar el modelo" << std::endl;
            logFile << "Error al cargar el modelo\n";
            return false;
        }
        
        // Configurar colores
        Color bgColor(m_config.backgroundColor.r, m_config.backgroundColor.g, m_config.backgroundColor.b);
        Color modelColor(m_config.modelColor.r, m_config.modelColor.g, m_config.modelColor.b);
        
        m_renderer->setBackgroundColor(bgColor);
        m_renderer->setModelColor(modelColor);
        
        // IMPORTANTE: Asegurar que la cámara esté centrada en el modelo
        // Esto es exactamente lo que hace el preview
        m_renderer->centerCamera();
        logFile << "Cámara centrada en el modelo\n";
        
        // Aplicar la configuración de cámara del usuario exactamente como en el preview
        m_renderer->setCameraOrbit(m_config.cameraYaw, m_config.cameraPitch, m_config.cameraDistance);
        logFile << "Configuración de cámara aplicada: yaw=" << m_config.cameraYaw 
                << ", pitch=" << m_config.cameraPitch << ", distance=" << m_config.cameraDistance << "\n";
        
        // Guardar imagen
        return saveImage(outputFile);
    } else {
        // En modo interactivo, debería estar ya inicializado
        if (!m_renderer) {
            std::cerr << "Error: No hay renderer inicializado" << std::endl;
            logFile << "Error: No hay renderer inicializado\n";
            return false;
        }
        
        // Cargar modelo
        if (!loadModel(inputFile)) {
            std::cerr << "Error al cargar el modelo" << std::endl;
            logFile << "Error al cargar el modelo\n";
            return false;
        }
        
        // IMPORTANTE: Asegurar que la cámara esté centrada en el modelo
        // Esto es exactamente lo que hace el preview
        m_renderer->centerCamera();
        logFile << "Cámara centrada en el modelo\n";
        
        // Aplicar la configuración de cámara del usuario exactamente como en el preview
        m_renderer->setCameraOrbit(m_config.cameraYaw, m_config.cameraPitch, m_config.cameraDistance);
        logFile << "Configuración de cámara aplicada: yaw=" << m_config.cameraYaw 
                << ", pitch=" << m_config.cameraPitch << ", distance=" << m_config.cameraDistance << "\n";
        
        // Guardar imagen
        return saveImage(outputFile);
    }
}

bool App::renderDirectory(const std::string& directory) {
    logFile << "renderDirectory(): Procesando directorio " << directory << "\n";
    std::cout << "Procesando directorio: " << directory << std::endl;
    
    try {
        // Verificar que el directorio existe
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            logFile << "Error: El directorio no existe: " << directory << "\n";
            std::cerr << "Error: El directorio no existe: " << directory << std::endl;
            return false;
        }
        
        int filesProcessed = 0;
        int filesSuccess = 0;
        
        // Iterar sobre todos los archivos .stl en el directorio
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".stl") {
                // Generar nombre de archivo de salida
                fs::path outputPath = entry.path();
                outputPath.replace_extension("png");
                
                // Procesar el archivo
                logFile << "Procesando: " << entry.path().string() << " -> " << outputPath.string() << "\n";
                std::cout << "Procesando: " << entry.path().filename().string() << std::endl;
                
                filesProcessed++;
                if (renderSingleFile(entry.path().string(), outputPath.string())) {
                    filesSuccess++;
                }
            }
        }
        
        // Mostrar resumen
        logFile << "Directorio procesado. " << filesSuccess << "/" << filesProcessed << " archivos procesados correctamente\n";
        std::cout << "Directorio procesado. " << filesSuccess << "/" << filesProcessed << " archivos procesados correctamente." << std::endl;
        
        return filesSuccess > 0;
    } catch (const std::exception& e) {
        logFile << "Error al procesar directorio: " << e.what() << "\n";
        std::cerr << "Error al procesar directorio: " << e.what() << std::endl;
        return false;
    }
}

bool App::processDirectory(const std::string& directory) {
    // Alias para renderDirectory
    return renderDirectory(directory);
}

void App::saveConfig() {
    logFile << "App::saveConfig() - Guardando configuración\n";
    
    // Guardar configuración en archivo
    std::string configPath = "config.ini";
    std::ofstream configFile(configPath);
    
    if (!configFile.is_open()) {
        logFile << "Error: No se pudo crear el archivo de configuración: " << configPath << "\n";
        return;
    }
    
    // Escribir configuración
    configFile << "# STL Renderer - Archivo de configuración\n";
    configFile << "# Generado: " << getCurrentTimestamp() << "\n\n";
    
    // Colores
    configFile << "# Colores (r,g,b en formato 0.0-1.0)\n";
    configFile << "backgroundColor=" << m_config.backgroundColor.r << "," 
               << m_config.backgroundColor.g << "," << m_config.backgroundColor.b << "\n";
    configFile << "modelColor=" << m_config.modelColor.r << "," 
               << m_config.modelColor.g << "," << m_config.modelColor.b << "\n\n";
    
    // Cámara
    configFile << "# Configuración de cámara\n";
    configFile << "cameraYaw=" << m_config.cameraYaw << "\n";
    configFile << "cameraPitch=" << m_config.cameraPitch << "\n";
    configFile << "cameraDistance=" << m_config.cameraDistance << "\n\n";
    
    // Imagen
    configFile << "# Configuración de imagen\n";
    configFile << "outputWidth=" << m_config.outputWidth << "\n";
    configFile << "outputHeight=" << m_config.outputHeight << "\n";
    configFile << "transparentBackground=" << (m_config.transparentBackground ? "true" : "false") << "\n";
    
    configFile.close();
    
    logFile << "Configuración guardada en: " << configPath << "\n";
}

bool App::loadConfig() {
    logFile << "App::loadConfig() - Cargando configuración\n";
    
    // Intentar cargar desde archivo de configuración
    std::string configPath = "config.ini";
    std::ifstream configFile(configPath);
    
    if (!configFile.is_open()) {
        logFile << "No se encontró archivo de configuración, usando valores por defecto\n";
        
        // Valores por defecto
        m_config.backgroundColor = Color(0.2f, 0.3f, 0.3f);
        m_config.modelColor = Color(1.0f, 0.5f, 0.2f);
        m_config.cameraYaw = 0.8f;
        m_config.cameraPitch = 0.5f;
        m_config.cameraDistance = 2.0f;
        m_config.outputWidth = 1024;
        m_config.outputHeight = 1024;
        m_config.transparentBackground = false;
        
        return false;
    }
    
    // Procesar archivo de configuración
    std::string line;
    while (std::getline(configFile, line)) {
        // Ignorar comentarios y líneas vacías
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string key;
        
        if (std::getline(iss, key, '=')) {
            std::string value;
            if (std::getline(iss, value)) {
                // Eliminar espacios en blanco
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                // Procesar las configuraciones
                if (key == "backgroundColor") {
                    m_config.backgroundColor = parseColor(value);
                } else if (key == "modelColor") {
                    m_config.modelColor = parseColor(value);
                } else if (key == "cameraYaw") {
                    m_config.cameraYaw = std::stof(value);
                } else if (key == "cameraPitch") {
                    m_config.cameraPitch = std::stof(value);
                } else if (key == "cameraDistance") {
                    m_config.cameraDistance = std::stof(value);
                } else if (key == "outputWidth") {
                    m_config.outputWidth = std::stoi(value);
                } else if (key == "outputHeight") {
                    m_config.outputHeight = std::stoi(value);
                } else if (key == "transparentBackground") {
                    m_config.transparentBackground = (value == "true" || value == "1");
                }
            }
        }
    }
    
    configFile.close();
    
    logFile << "Configuración cargada:\n";
    logFile << "  - backgroundColor: (" << m_config.backgroundColor.r << ", " 
            << m_config.backgroundColor.g << ", " << m_config.backgroundColor.b << ")\n";
    logFile << "  - modelColor: (" << m_config.modelColor.r << ", " 
            << m_config.modelColor.g << ", " << m_config.modelColor.b << ")\n";
    logFile << "  - cameraYaw: " << m_config.cameraYaw << "\n";
    logFile << "  - cameraPitch: " << m_config.cameraPitch << "\n";
    logFile << "  - cameraDistance: " << m_config.cameraDistance << "\n";
    logFile << "  - outputWidth: " << m_config.outputWidth << "\n";
    logFile << "  - outputHeight: " << m_config.outputHeight << "\n";
    logFile << "  - transparentBackground: " << (m_config.transparentBackground ? "true" : "false") << "\n";
    
    return true;
}

// Función auxiliar para parsear colores en formato "r,g,b"
Color App::parseColor(const std::string& colorStr) {
    std::istringstream iss(colorStr);
    float r, g, b;
    char comma;
    
    iss >> r >> comma >> g >> comma >> b;
    
    return Color(r, g, b);
}

void App::shutdown() {
    // Cerrar archivo de log
    if (logFile.is_open()) {
        logFile << "Cerrando aplicación" << std::endl;
        logFile.close();
    }
    
    // Limpiar recursos usando reset() en lugar de delete
    m_renderer.reset();
    m_gui.reset();
    m_stlLoader.reset();
}

bool App::initialize() {
    // Crear componentes
    m_renderer = std::make_unique<Renderer>();
    m_stlLoader = std::make_unique<StlLoader>();
    
    logFile << "App::initialize() - Creando componentes" << std::endl;
    
    // Inicializar renderer
    if (!m_renderer->initialize(1024, 768)) {
        logFile << "Error: No se pudo inicializar el renderer" << std::endl;
        std::cerr << "Error: No se pudo inicializar el renderer" << std::endl;
        return false;
    }
    
    logFile << "Renderer inicializado correctamente" << std::endl;
    
    // En modo con GUI, inicializar interfaz
    if (!m_silentMode) {
        m_gui = std::make_unique<Gui>(*m_renderer, *m_stlLoader);
        
        if (!m_gui->initialize()) {
            logFile << "Error: No se pudo inicializar la GUI" << std::endl;
            std::cerr << "Error: No se pudo inicializar la GUI" << std::endl;
            return false;
        }
        
        logFile << "GUI inicializada correctamente" << std::endl;
    } else {
        logFile << "Ejecutando en modo silencioso (sin GUI)" << std::endl;
    }
    
    return true;
}

bool App::processCommandLine(int argc, char* argv[]) {
    logFile << "Procesando línea de comandos con " << argc << " argumentos" << std::endl;
    
    // Procesar argumentos de línea de comandos
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        logFile << "Argumento " << i << ": " << arg << std::endl;
        
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return false;
        } else if (arg == "--version" || arg == "-v") {
            printVersion();
            return false;
        } else if (arg == "--silent" || arg == "-s") {
            m_silentMode = true;
            logFile << "Modo silencioso activado" << std::endl;
        } else if (arg == "--color" || arg == "-c") {
            if (i + 3 < argc) {
                float r = std::stof(argv[i + 1]);
                float g = std::stof(argv[i + 2]);
                float b = std::stof(argv[i + 3]);
                
                // Usar Color en lugar de glm::vec3
                m_renderer->setModelColor(Color(r, g, b));
                logFile << "Color del modelo establecido a RGB(" << r << ", " << g << ", " << b << ")" << "\n";
                i += 3;
            }
        } else if (arg == "--background" || arg == "-bg") {
            if (i + 3 < argc) {
                float r = std::stof(argv[i + 1]);
                float g = std::stof(argv[i + 2]);
                float b = std::stof(argv[i + 3]);
                
                // Usar Color en lugar de glm::vec3
                m_renderer->setBackgroundColor(Color(r, g, b));
                logFile << "Color de fondo establecido a RGB(" << r << ", " << g << ", " << b << ")" << "\n";
                i += 3;
            }
        } else if (arg == "--angle" || arg == "-a") {
            if (i + 1 < argc) {
                float angle = std::stof(argv[i + 1]);
                
                // En lugar de setCameraAngle, usamos métodos disponibles en Renderer
                // Convertir ángulo a radianes
                float radians = angle * 3.14159f / 180.0f;
                m_renderer->setCameraOrbit(radians, 0.5f, 5.0f);
                logFile << "Ángulo de cámara establecido a " << angle << " grados" << std::endl;
                i += 1;
            }
        } else if (arg == "--output" || arg == "-o") {
            if (i + 1 < argc) {
                m_outputFile = argv[i + 1];
                logFile << "Archivo de salida establecido a: " << m_outputFile << std::endl;
                i += 1;
            }
        } else {
            // Asumimos que es un archivo STL
            m_inputFiles.push_back(arg);
            logFile << "Archivo STL añadido a la lista: " << arg << std::endl;
        }
    }
    
    // En modo silencioso, debemos tener al menos un archivo de entrada
    if (m_silentMode && m_inputFiles.empty()) {
        std::cerr << "Error: En modo silencioso se requiere al menos un archivo STL" << std::endl;
        logFile << "Error: En modo silencioso se requiere al menos un archivo STL" << std::endl;
        printHelp();
        return false;
    }
    
    return true;
}

void App::printHelp() {
    std::cout << "Uso: stlrenderer [opciones] [archivos_stl]" << std::endl;
    std::cout << "Opciones:" << std::endl;
    std::cout << "  -h, --help\t\tMuestra esta ayuda" << std::endl;
    std::cout << "  -v, --version\t\tMuestra la versión" << std::endl;
    std::cout << "  -s, --silent\t\tModo silencioso (sin GUI)" << std::endl;
    std::cout << "  -c, --color R G B\tEstablece el color del modelo (valores entre 0.0 y 1.0)" << std::endl;
    std::cout << "  -bg, --background R G B\tEstablece el color de fondo (valores entre 0.0 y 1.0)" << std::endl;
    std::cout << "  -a, --angle A\t\tEstablece el ángulo de la cámara en grados" << std::endl;
    std::cout << "  -o, --output ARCHIVO\tEstablece el archivo de salida para la imagen renderizada" << std::endl;
    
    logFile << "Se mostró la ayuda al usuario" << std::endl;
}

void App::printVersion() {
    std::cout << "STL Renderer v1.0.0" << std::endl;
    std::cout << "Copyright (c) 2023" << std::endl;
    
    logFile << "Se mostró la información de versión al usuario" << std::endl;
}

void App::run() {
    if (m_silentMode) {
        runSilentMode();
    } else {
        runGuiMode();
    }
}

void App::runSilentMode() {
    logFile << "Ejecutando en modo silencioso para " << m_inputFiles.size() << " archivos" << std::endl;
    
    // Procesar cada archivo STL en modo silencioso
    for (const auto& file : m_inputFiles) {
        logFile << "Procesando archivo: " << file << std::endl;
        
        // Cargar modelo
        if (!m_stlLoader->loadModel(file, *m_renderer)) {
            std::cerr << "Error al cargar el archivo: " << file << std::endl;
            logFile << "Error al cargar el archivo: " << file << std::endl;
            continue;
        }
        
        // Generar nombre de salida si no se especificó
        std::string outputFile = m_outputFile;
        if (outputFile.empty()) {
            // Extraer nombre base del archivo de entrada
            size_t lastSlash = file.find_last_of("/\\");
            size_t lastDot = file.find_last_of(".");
            
            std::string baseName;
            if (lastSlash != std::string::npos) {
                if (lastDot != std::string::npos && lastDot > lastSlash) {
                    baseName = file.substr(lastSlash + 1, lastDot - lastSlash - 1);
                } else {
                    baseName = file.substr(lastSlash + 1);
                }
            } else {
                if (lastDot != std::string::npos) {
                    baseName = file.substr(0, lastDot);
                } else {
                    baseName = file;
                }
            }
            
            outputFile = baseName + ".png";
        }
        
        logFile << "Renderizando a archivo: " << outputFile << std::endl;
        
        // Renderizar y guardar imagen
        if (m_renderer->saveImage(outputFile)) {
            std::cout << "Imagen guardada en: " << outputFile << std::endl;
            logFile << "Imagen guardada exitosamente: " << outputFile << std::endl;
        } else {
            std::cerr << "Error al guardar la imagen: " << outputFile << std::endl;
            logFile << "Error al guardar la imagen: " << outputFile << std::endl;
        }
    }
    
    logFile << "Procesamiento en modo silencioso completado" << std::endl;
}

void App::runGuiMode() {
    logFile << "Ejecutando en modo GUI" << std::endl;
    
    // Si hay archivos especificados en la línea de comandos, cargar el primero
    if (!m_inputFiles.empty()) {
        logFile << "Cargando archivo inicial: " << m_inputFiles[0] << std::endl;
        m_stlLoader->loadModel(m_inputFiles[0], *m_renderer);
    }
    
    // Bucle principal de la aplicación
    while (!m_gui->shouldClose()) {
        // Procesar eventos
        glfwPollEvents();
        
        // Renderizar escena
        m_renderer->render();
        
        // Renderizar GUI
        m_gui->render();
    }
    
    logFile << "Bucle de renderizado terminado" << std::endl;
}

bool App::parseArguments(int argc, char* argv[]) {
    if (!processCommandLine(argc, argv)) {
        return false;
    }
    
    return initialize();
}

// Función para obtener un timestamp como string
std::string App::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time);
    
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return ss.str();
}

// Métodos para establecer colores
void App::setModelColor(const Color& color) {
    // Actualizar la configuración
    m_config.modelColor = color;
    
    // Aplicar el color al renderer si existe
    if (m_renderer) {
        m_renderer->setModelColor(color);
        logFile << "Color del modelo actualizado a: (" << color.r << ", " << color.g << ", " << color.b << ")\n";
    }
}

void App::setBackgroundColor(const Color& color) {
    // Actualizar la configuración
    m_config.backgroundColor = color;
    
    // Aplicar el color al renderer si existe
    if (m_renderer) {
        m_renderer->setBackgroundColor(color);
        logFile << "Color de fondo actualizado a: (" << color.r << ", " << color.g << ", " << color.b << ")\n";
    }
} 
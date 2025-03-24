#include "app.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <Windows.h>
#include <shellapi.h>

namespace fs = std::filesystem;

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Convertir argumentos de línea de comandos a UTF-8
    int argc;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    
    if (!argvW) {
        MessageBoxA(NULL, "Error al procesar argumentos de línea de comandos", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Convertir wchar_t** a char**
    std::vector<std::string> utf8Args;
    std::vector<char*> argv;
    
    for (int i = 0; i < argc; i++) {
        int size = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
        std::string arg(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, &arg[0], size, nullptr, nullptr);
        utf8Args.push_back(arg);
        argv.push_back(const_cast<char*>(utf8Args.back().c_str()));
    }
    
    LocalFree(argvW);
    
    // Crear y ejecutar la aplicación
    App app(false);  // Iniciar en modo no silencioso por defecto
    int result = app.run(argc, argv.data());
    
    return result;
}
#endif

// Función main estándar para garantizar compatibilidad
int main(int argc, char* argv[]) {
#ifdef _WIN32
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
#else
    // Crear instancia de la aplicación
    App app(false);  // Iniciar en modo no silencioso por defecto
    
    // Verificar si se recibió un archivo como argumento (arrastrar y soltar)
    if (argc > 1) {
        std::string filePath = argv[1];
        fs::path path(filePath);
        
        // Verificar si es un archivo STL
        if (fs::exists(path) && path.extension() == ".stl") {
            // Generar nombre para el archivo de salida (mismo nombre + _png.png)
            fs::path outputPath = path;
            outputPath.replace_extension();
            outputPath += "_png.png";
            
            // Renderizar archivo directamente sin mostrar GUI
            app.renderSingleFile(filePath, outputPath.string());
            return 0;
        }
    }
    
    // Si no hay argumentos o el procesamiento falló, iniciar la aplicación normal
    return app.run(argc, argv);
#endif
} 
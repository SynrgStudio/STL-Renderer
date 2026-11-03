#include "app.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <shellapi.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        SetConsoleOutputCP(CP_UTF8);   // rutas con acentos legibles
        FILE* stream = nullptr;
        DWORD flags;
        if (GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &flags))
            freopen_s(&stream, "CONOUT$", "w", stdout);
        if (GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &flags))
            freopen_s(&stream, "CONOUT$", "w", stderr);
    }
    int argc = 0;
    LPWSTR* wideArgs = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!wideArgs) return 1;
    std::vector<std::string> arguments;
    arguments.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        int size = WideCharToMultiByte(CP_UTF8, 0, wideArgs[i], -1, nullptr, 0, nullptr, nullptr);
        std::string value(size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wideArgs[i], -1, value.data(), size, nullptr, nullptr);
        value.pop_back();
        arguments.push_back(std::move(value));
    }
    LocalFree(wideArgs);
    std::vector<char*> argv;
    for (auto& argument : arguments) argv.push_back(argument.data());
    try {
        App app;
        return app.run(argc, argv.data());
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}

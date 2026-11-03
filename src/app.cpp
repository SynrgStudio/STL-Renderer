#include "app.h"
#include "cli.h"

#include <windows.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

namespace {
fs::path executableDirectory() {
    std::wstring buffer(MAX_PATH, L'\0');
    for (;;) {
        const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0) return fs::current_path();
        if (length < buffer.size()) {
            buffer.resize(length);
            return fs::path(buffer).parent_path();
        }
        buffer.resize(buffer.size() * 2);
    }
}

// Modo portable si existe config.ini junto al ejecutable; si no, %APPDATA%\STLRenderer
// (la carpeta de instalación en Program Files no admite escritura).
fs::path configFilePath() {
    const auto portable = executableDirectory() / "config.ini";
    std::error_code ec;
    if (fs::is_regular_file(portable, ec)) return portable;
    wchar_t* appData = nullptr;
    size_t length = 0;
    if (_wdupenv_s(&appData, &length, L"APPDATA") == 0 && appData) {
        fs::path directory = fs::path(appData) / "STLRenderer";
        free(appData);
        fs::create_directories(directory, ec);
        if (!ec) return directory / "config.ini";
    }
    return portable;
}

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r");
    if (first == std::string::npos) return {};
    return value.substr(first, value.find_last_not_of(" \t\r") - first + 1);
}

float parseFloat(const std::string& value) {
    std::istringstream stream(value);
    stream.imbue(std::locale::classic());
    float result = 0;
    if (!(stream >> result) || !(stream >> std::ws).eof() || !std::isfinite(result))
        throw std::invalid_argument("invalid number");
    return result;
}

int parseInt(const std::string& value) {
    size_t used = 0;
    const int result = std::stoi(value, &used);
    if (used != value.size()) throw std::invalid_argument("invalid integer");
    return result;
}

Color parseColor(const std::string& value) {
    std::istringstream stream(value);
    stream.imbue(std::locale::classic());
    float r = 0, g = 0, b = 0;
    char comma1 = 0, comma2 = 0;
    if (!(stream >> r >> comma1 >> g >> comma2 >> b) || comma1 != ',' || comma2 != ',' ||
        !(stream >> std::ws).eof())
        throw std::invalid_argument("invalid color");
    for (float c : {r, g, b})
        if (!std::isfinite(c) || c < 0 || c > 1) throw std::invalid_argument("invalid color");
    return Color(r, g, b);
}
} // namespace

App::App()
    : m_configPath(configFilePath()),
      m_renderer(std::make_unique<Renderer>()),
      m_stlLoader(std::make_unique<StlLoader>()) {
    loadConfig();
    m_renderer->setBackgroundColor(m_config.backgroundColor);
    m_renderer->setModelColor(m_config.modelColor);
}

App::~App() {
    // La GUI debe destruirse antes que el renderer (comparten el contexto OpenGL).
    m_gui.reset();
    m_stlLoader.reset();
    m_renderer.reset();
}

int App::run(int argc, char* argv[]) {
    CliOptions options;
    try {
        options = parseCli(std::vector<std::string>(argv + std::min(argc, 1), argv + argc));
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\nUse --help for usage.\n";
        return 2;
    }
    if (options.help) { printHelp(); return 0; }
    if (options.version) { printVersion(); return 0; }

    // Las opciones de línea de comandos sólo afectan a esta ejecución.
    auto toColor = [](const std::array<float, 3>& c) { return Color(c[0], c[1], c[2]); };
    if (options.color) m_config.modelColor = toColor(*options.color);
    if (options.background) m_config.backgroundColor = toColor(*options.background);
    if (options.angle) m_config.cameraYaw = glm::radians(std::remainder(*options.angle, 360.0f));
    if (options.width) m_config.outputWidth = *options.width;
    if (options.height) m_config.outputHeight = *options.height;
    if (options.transparent) m_config.transparentBackground = *options.transparent;

    if (options.inputs.empty()) return runInteractive();

    if (!options.output.empty()) {
        std::error_code ec;
        const auto input = fs::u8path(options.inputs.front());
        const auto output = fs::u8path(options.output);
        if (!fs::is_regular_file(input, ec) || !isStlPath(input)) {
            std::cerr << "Error: --output requires a single STL file as input\n";
            return 2;
        }
        auto extension = output.extension().u8string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (extension != ".png") {
            std::cerr << "Error: output file must have a .png extension\n";
            return 2;
        }
    }

    if (!m_renderer->initializeHeadless(m_config.outputWidth, m_config.outputHeight)) {
        std::cerr << "Error: could not initialize OpenGL\n";
        return 1;
    }

    bool success = true;
    for (const auto& input : options.inputs) {
        std::error_code ec;
        const auto path = fs::u8path(input);
        bool rendered = false;
        if (fs::is_directory(path, ec)) {
            rendered = renderDirectory(input);
        } else if (fs::is_regular_file(path, ec) && isStlPath(path)) {
            const auto output = options.output.empty() ? defaultOutputPath(path).u8string() : options.output;
            rendered = renderSingleFile(input, output);
        } else {
            std::cerr << "Not an STL file or directory: " << input << '\n';
        }
        success = rendered && success;
    }
    return success ? 0 : 1;
}

int App::runInteractive() {
    if (!m_renderer->initialize(1024, 768)) {
        std::cerr << "Error: could not initialize the main window\n";
        return 1;
    }
    m_gui = std::make_unique<Gui>(*this);
    if (!m_gui->initialize()) {
        std::cerr << "Error: could not initialize the user interface\n";
        return 1;
    }
    m_renderer->setBackgroundColor(m_config.backgroundColor);
    m_renderer->setModelColor(m_config.modelColor);

    while (!glfwWindowShouldClose(m_renderer->getWindow())) {
        glfwPollEvents();
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_renderer->getWindow(), &width, &height);
        if (width == 0 || height == 0) {   // ventana minimizada
            glfwWaitEvents();
            continue;
        }
        glViewport(0, 0, width, height);
        glClearColor(m_config.backgroundColor.r, m_config.backgroundColor.g, m_config.backgroundColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        updateRendererCamera();
        m_gui->render();
        glfwSwapBuffers(m_renderer->getWindow());
    }
    saveConfig();
    return 0;
}

void App::updateRendererCamera() {
    if (m_gui) {
        m_config.cameraYaw = m_gui->getCameraYaw();
        m_config.cameraPitch = m_gui->getCameraPitch();
        m_config.cameraDistance = m_gui->getCameraDistance();
    }
    m_renderer->setCameraOrbit(m_config.cameraYaw, m_config.cameraPitch, m_config.cameraDistance);
}

bool App::loadModel(const std::string& filename) {
    if (!m_stlLoader->loadModel(filename, *m_renderer)) {
        std::cerr << "Invalid or unreadable STL file: " << filename << '\n';
        return false;
    }
    m_renderer->centerCamera();
    m_renderer->setCameraOrbit(m_config.cameraYaw, m_config.cameraPitch, m_config.cameraDistance);
    return true;
}

bool App::saveImage(const std::string& outputFile) {
    if (!m_renderer->hasModel()) {
        std::cerr << "No model loaded\n";
        return false;
    }
    m_renderer->setBackgroundColor(m_config.backgroundColor);
    m_renderer->setModelColor(m_config.modelColor);
    if (!m_renderer->renderToFile(outputFile, m_config.transparentBackground,
                                  m_config.outputWidth, m_config.outputHeight)) {
        std::cerr << "Could not write image: " << outputFile << '\n';
        return false;
    }
    std::cout << "Saved " << outputFile << '\n';
    return true;
}

bool App::renderSingleFile(const std::string& inputFile, const std::string& outputFile) {
    return loadModel(inputFile) && saveImage(outputFile);
}

bool App::renderDirectory(const std::string& directory) {
    std::vector<fs::path> files;
    try {
        for (const auto& entry : fs::directory_iterator(fs::u8path(directory))) {
            std::error_code ec;
            if (entry.is_regular_file(ec) && isStlPath(entry.path())) files.push_back(entry.path());
        }
    } catch (const fs::filesystem_error& error) {
        std::cerr << "Could not read directory " << directory << ": " << error.what() << '\n';
        return false;
    }
    std::sort(files.begin(), files.end());
    size_t succeeded = 0;
    for (const auto& file : files)
        if (renderSingleFile(file.u8string(), defaultOutputPath(file).u8string())) ++succeeded;
    std::cout << succeeded << "/" << files.size() << " files rendered from " << directory << '\n';
    return !files.empty() && succeeded == files.size();
}

bool App::loadConfig() {
    std::ifstream file(m_configPath);
    if (!file) return false;
    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        const auto separator = line.find('=');
        if (separator == std::string::npos) continue;
        const auto key = trim(line.substr(0, separator));
        const auto value = trim(line.substr(separator + 1));
        try {
            if (key == "backgroundColor") m_config.backgroundColor = parseColor(value);
            else if (key == "modelColor") m_config.modelColor = parseColor(value);
            else if (key == "cameraYaw") m_config.cameraYaw = parseFloat(value);
            else if (key == "cameraPitch") m_config.cameraPitch = std::clamp(parseFloat(value), -1.5f, 1.5f);
            else if (key == "cameraDistance") m_config.cameraDistance = std::clamp(parseFloat(value), 1.0f, 20.0f);
            else if (key == "outputWidth") m_config.outputWidth = std::clamp(parseInt(value), 1, 8192);
            else if (key == "outputHeight") m_config.outputHeight = std::clamp(parseInt(value), 1, 8192);
            else if (key == "transparentBackground") m_config.transparentBackground = value == "true" || value == "1";
            else if (key == "batchDirectory") m_config.batchDirectory = value;
        } catch (const std::exception&) {
            std::cerr << "Ignoring invalid config value: " << key << '\n';
        }
    }
    return true;
}

void App::saveConfig() const {
    std::ofstream file(m_configPath, std::ios::trunc);
    if (!file) {
        std::cerr << "Could not write " << m_configPath.u8string() << '\n';
        return;
    }
    file.imbue(std::locale::classic());
    const auto& c = m_config;
    file << "# STL Renderer configuration\n"
         << "backgroundColor=" << c.backgroundColor.r << ',' << c.backgroundColor.g << ',' << c.backgroundColor.b << '\n'
         << "modelColor=" << c.modelColor.r << ',' << c.modelColor.g << ',' << c.modelColor.b << '\n'
         << "cameraYaw=" << c.cameraYaw << '\n'
         << "cameraPitch=" << c.cameraPitch << '\n'
         << "cameraDistance=" << c.cameraDistance << '\n'
         << "outputWidth=" << c.outputWidth << '\n'
         << "outputHeight=" << c.outputHeight << '\n'
         << "transparentBackground=" << (c.transparentBackground ? "true" : "false") << '\n'
         << "batchDirectory=" << c.batchDirectory << '\n';
}

void App::printHelp() {
    std::cout <<
        "Usage: STLRenderer [options] [files.stl | directories]\n"
        "Without inputs the graphical interface is opened.\n\n"
        "  -h, --help                 Show this help\n"
        "  -v, --version              Show version\n"
        "  -s, --silent               Render without UI (requires an input)\n"
        "  -c, --color R G B          Model color, each component 0..1\n"
        "  -bg, --background R G B    Background color, each component 0..1\n"
        "  -a, --angle DEGREES        Camera yaw\n"
        "  -o, --output FILE.png      Output path (single STL input only)\n"
        "  --width N, --height N      Image size, 1..8192\n"
        "  --transparent, --opaque    PNG background mode\n"
        "  --                         Treat the remaining arguments as inputs\n\n"
        "Default output: <input>_png.png next to each STL. Directories are not recursive.\n"
        "Exit codes: 0 success, 1 some file failed, 2 invalid arguments.\n";
}

void App::printVersion() {
    std::cout << "STL Renderer " << STL_RENDERER_VERSION << '\n';
}

void App::setModelColor(const Color& color) {
    m_config.modelColor = color;
    m_renderer->setModelColor(color);
}

void App::setBackgroundColor(const Color& color) {
    m_config.backgroundColor = color;
    m_renderer->setBackgroundColor(color);
}

#include "cli.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

CliOptions parseCli(const std::vector<std::string>& arguments) {
    CliOptions result;
    bool positional = false;
    for (size_t i = 0; i < arguments.size(); ++i) {
        const auto& arg = arguments[i];
        auto next = [&]() -> const std::string& {
            if (++i >= arguments.size()) throw std::invalid_argument("Missing value for " + arg);
            return arguments[i];
        };
        auto number = [&]() {
            const auto& value = next();
            size_t used = 0;
            float n = 0;
            try { n = std::stof(value, &used); } catch (const std::exception&) { used = 0; }
            if (used == 0 || used != value.size() || !std::isfinite(n))
                throw std::invalid_argument("Invalid number for " + arg);
            return n;
        };
        if (positional) result.inputs.push_back(arg);
        else if (arg == "--") positional = true;
        else if (arg == "--help" || arg == "-h") result.help = true;
        else if (arg == "--version" || arg == "-v") result.version = true;
        else if (arg == "--silent" || arg == "-s") result.silent = true;
        else if (arg == "--transparent") result.transparent = true;
        else if (arg == "--opaque") result.transparent = false;
        else if (arg == "--color" || arg == "-c" || arg == "--background" || arg == "-bg") {
            std::array<float, 3> color;
            for (auto& component : color) {
                component = number();
                if (component < 0 || component > 1)
                    throw std::invalid_argument("Color components must be between 0 and 1");
            }
            if (arg == "--color" || arg == "-c") result.color = color;
            else result.background = color;
        } else if (arg == "--angle" || arg == "-a") result.angle = number();
        else if (arg == "--width" || arg == "--height") {
            const auto& value = next();
            size_t used = 0;
            int n = 0;
            try { n = std::stoi(value, &used); } catch (const std::exception&) { used = 0; }
            if (used == 0 || used != value.size() || n < 1 || n > 8192)
                throw std::invalid_argument("Image dimensions must be integers between 1 and 8192");
            if (arg == "--width") result.width = n;
            else result.height = n;
        } else if (arg == "--output" || arg == "-o") {
            result.output = next();
            if (result.output.empty()) throw std::invalid_argument("Output path cannot be empty");
        } else if (!arg.empty() && arg.front() == '-')
            throw std::invalid_argument("Unknown option: " + arg);
        else result.inputs.push_back(arg);
    }
    if (!result.help && !result.version) {
        if (result.silent && result.inputs.empty())
            throw std::invalid_argument("Silent mode requires an STL file or directory");
        if (!result.output.empty() && result.inputs.size() != 1)
            throw std::invalid_argument("--output requires exactly one STL file");
    }
    return result;
}

bool isStlPath(const std::filesystem::path& path) {
    auto extension = path.extension().u8string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".stl";
}

std::filesystem::path defaultOutputPath(const std::filesystem::path& input) {
    auto output = input;
    output.replace_extension();
    output += "_png.png";
    return output;
}

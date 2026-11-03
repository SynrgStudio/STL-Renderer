#pragma once
#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct CliOptions {
    bool help = false, version = false, silent = false;
    std::optional<std::array<float, 3>> color, background;
    std::optional<float> angle;
    std::optional<int> width, height;
    std::optional<bool> transparent;
    std::string output;
    std::vector<std::string> inputs;
};
CliOptions parseCli(const std::vector<std::string>& arguments);
bool isStlPath(const std::filesystem::path& path);
std::filesystem::path defaultOutputPath(const std::filesystem::path& input);

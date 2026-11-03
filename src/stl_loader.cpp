#include "stl_loader.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>

namespace {
uint32_t littleUint(const unsigned char* b) {
    return uint32_t(b[0]) | uint32_t(b[1]) << 8 | uint32_t(b[2]) << 16 | uint32_t(b[3]) << 24;
}
float littleFloat(const unsigned char* bytes) {
    auto bits = littleUint(bytes);
    float value;
    static_assert(sizeof(value) == sizeof(bits));
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
bool finite(const glm::vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
bool prepareTriangle(Triangle& triangle) {
    for (const auto& v : triangle.vertices)
        if (!finite(v.position) || !finite(v.normal)) return false;
    glm::dvec3 normal(triangle.vertices[0].normal);
    if (glm::length(normal) == 0.0)
        normal = glm::cross(glm::dvec3(triangle.vertices[1].position) - glm::dvec3(triangle.vertices[0].position),
                            glm::dvec3(triangle.vertices[2].position) - glm::dvec3(triangle.vertices[0].position));
    const double length = glm::length(normal);
    const glm::vec3 unit = length > 0 ? glm::vec3(normal / length) : glm::vec3(0, 1, 0);
    for (auto& vertex : triangle.vertices) vertex.normal = unit;
    return true;
}
}

StlLoader::StlLoader() = default;
StlLoader::~StlLoader() = default;

bool StlLoader::loadModel(const std::string& filename) {
    // Keep the current model intact if parsing or validation fails.
    StlLoader candidate;
    std::ifstream file(std::filesystem::u8path(filename), std::ios::binary | std::ios::ate);
    if (!file) return false;
    const auto size = file.tellg();
    bool binary = false;
    if (size >= 84) {
        file.seekg(80);
        std::array<unsigned char, 4> count{};
        file.read(reinterpret_cast<char*>(count.data()), count.size());
        binary = uint64_t(size) == 84ULL + 50ULL * littleUint(count.data());
    }
    file.close();
    if (!(binary ? candidate.loadBinarySTL(filename) : candidate.loadAsciiSTL(filename))) return false;
    candidate.calculateModelInfo();
    if (!finite(candidate.m_model.center) || !std::isfinite(candidate.m_model.scale) || candidate.m_model.scale <= 0)
        return false;
    const auto center = candidate.m_model.center;
    for (auto& triangle : candidate.m_model.triangles) {
        for (auto& vertex : triangle.vertices) {
            auto p = vertex.position - center;
            vertex.position = glm::vec3(p.x, p.z, -p.y);
            auto n = vertex.normal;
            vertex.normal = glm::vec3(n.x, n.z, -n.y);
            if (!finite(vertex.position)) return false;
        }
    }
    candidate.calculateModelInfo();
    if (!finite(candidate.m_model.center) || !std::isfinite(candidate.m_model.scale) || candidate.m_model.scale <= 0)
        return false;
    m_model = std::move(candidate.m_model);
    return true;
}

bool StlLoader::loadBinarySTL(const std::string& filename) {
    std::ifstream file(std::filesystem::u8path(filename), std::ios::binary | std::ios::ate);
    const auto size = file.tellg();
    if (size < 84) return false;
    file.seekg(80);
    std::array<unsigned char, 4> countBytes{};
    if (!file.read(reinterpret_cast<char*>(countBytes.data()), countBytes.size())) return false;
    const uint32_t count = littleUint(countBytes.data());
    if (!count || 84ULL + 50ULL * count != uint64_t(size)) return false;
    m_model.triangles.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        std::array<unsigned char, 50> record{};
        if (!file.read(reinterpret_cast<char*>(record.data()), record.size())) return false;
        Triangle triangle{};
        glm::vec3 normal(littleFloat(&record[0]), littleFloat(&record[4]), littleFloat(&record[8]));
        for (int v = 0; v < 3; ++v) {
            const auto* p = record.data() + 12 + v * 12;
            triangle.vertices[v] = {glm::vec3(littleFloat(p), littleFloat(p + 4), littleFloat(p + 8)), normal};
        }
        if (!prepareTriangle(triangle)) return false;
        m_model.triangles.push_back(triangle);
    }
    return true;
}

bool StlLoader::loadAsciiSTL(const std::string& filename) {
    std::ifstream file(std::filesystem::u8path(filename));
    file.imbue(std::locale::classic());
    std::string token, rest;
    auto expect = [&](const char* expected) { return bool(file >> token) && token == expected; };
    if (!expect("solid")) return false;
    std::getline(file, rest);
    while (file >> token) {
        if (token == "endsolid") {
            std::getline(file, rest);
            file >> std::ws;
            return file.eof() && !m_model.triangles.empty();
        }
        if (token != "facet" || !expect("normal")) return false;
        glm::vec3 normal{};
        if (!(file >> normal.x >> normal.y >> normal.z) || !expect("outer") || !expect("loop")) return false;
        Triangle triangle{};
        for (auto& vertex : triangle.vertices) {
            if (!expect("vertex") || !(file >> vertex.position.x >> vertex.position.y >> vertex.position.z)) return false;
            vertex.normal = normal;
        }
        if (!expect("endloop") || !expect("endfacet") || !prepareTriangle(triangle)) return false;
        m_model.triangles.push_back(triangle);
    }
    return false;
}

void StlLoader::calculateModelInfo() {
    m_model.minBounds = glm::vec3(std::numeric_limits<float>::max());
    m_model.maxBounds = glm::vec3(std::numeric_limits<float>::lowest());
    for (const auto& triangle : m_model.triangles)
        for (const auto& vertex : triangle.vertices) {
            m_model.minBounds = glm::min(m_model.minBounds, vertex.position);
            m_model.maxBounds = glm::max(m_model.maxBounds, vertex.position);
        }
    const glm::dvec3 min(m_model.minBounds), max(m_model.maxBounds);
    m_model.center = glm::vec3((min + max) * 0.5);
    const auto dimensions = max - min;
    const double extent = std::max({dimensions.x, dimensions.y, dimensions.z});
    m_model.scale = extent > 0 ? static_cast<float>(1.0 / extent) : 1.0f;
}

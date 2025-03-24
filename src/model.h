#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct Triangle {
    Vertex vertices[3];
};

struct Model {
    std::vector<Triangle> triangles;
    glm::vec3 minBounds;
    glm::vec3 maxBounds;
    glm::vec3 center;
    float scale;
}; 
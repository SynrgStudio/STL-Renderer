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
    glm::vec3 minBounds{0.0f};
    glm::vec3 maxBounds{0.0f};
    glm::vec3 center{0.0f};
    float scale = 1.0f;
}; 

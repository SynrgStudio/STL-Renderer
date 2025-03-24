#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <memory>
#include "model.h"
#include "shader.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Estructura para representar el color (RGB)
struct Color {
    float r, g, b;
    
    Color() : r(1.0f), g(1.0f), b(1.0f) {}
    Color(float red, float green, float blue) : r(red), g(green), b(blue) {}
    
    // Conversión a Vector3 de GLM
    operator glm::vec3() const { return glm::vec3(r, g, b); }
};

// Clase para manejar la renderización
class Renderer {
public:
    Renderer();
    ~Renderer();
    
    // Inicialización
    bool initialize(int width, int height, bool headless = false);
    bool initializeHeadless(int width, int height);
    void cleanup();
    
    // Operaciones
    void render();
    void renderModel();
    bool renderToFile(const std::string& filename, bool transparentBg = false);
    bool loadModel(const std::string& filename);
    bool saveImage(const std::string& filename, bool transparentBg = false);
    
    // Configuración
    void setBackgroundColor(const Color& color);
    void setModelColor(const Color& color);
    void setModel(const Model& model);
    
    // Operaciones de cámara
    void setCameraOrbit(float yaw, float pitch, float distance);
    void setCameraPosition(const glm::vec3& position);
    void setCameraTarget(const glm::vec3& target);
    void centerCamera();
    
    // Configuración de proyección
    void setProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane);
    
    // Getters
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    GLFWwindow* getWindow() const { return m_window; }
    bool hasModel() const { return m_hasModel; }
    
private:
    // Ventana y contexto
    GLFWwindow* m_window;
    int m_width, m_height;
    bool m_headless;
    bool m_initialized;
    
    // Modelo y renderizado
    Model m_model;
    bool m_hasModel;
    
    // Shader
    std::unique_ptr<Shader> m_shader;
    
    // Buffers de OpenGL
    unsigned int m_vao, m_vbo;
    unsigned int m_defaultCubeVAO, m_defaultCubeVBO;
    
    // Framebuffer para renderizado a imagen
    unsigned int m_fbo;
    unsigned int m_colorAttachment;
    unsigned int m_depthAttachment;
    
    // Colores
    Color m_backgroundColor;
    Color m_modelColor;
    
    // Cámara
    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewMatrix;
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraTarget;
    float m_cameraYaw;
    float m_cameraPitch;
    float m_cameraDistance;
    
    // Métodos privados
    void updateViewMatrix();
    void createShaders();
    void setupBuffers();
    void setupFramebuffer();
    void destroyGLResources();
    void createDefaultCube();
}; 
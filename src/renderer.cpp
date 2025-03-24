#include "renderer.h"
#include "stl_loader.h"

// Incluir glad primero
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h> // Para ImGuiContext

// Definir GL_SCISSOR_TEST si no está disponible
#ifndef GL_SCISSOR_TEST
#define GL_SCISSOR_TEST 0x0C11
#endif

// Definir GL_LESS si no está disponible
#ifndef GL_LESS
#define GL_LESS 0x0201
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <iostream>
#include <vector>
#include <fstream>

// Referenciar el logFile desde app.cpp
extern std::ofstream logFile;

// Shaders
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    out vec3 FragPos;
    out vec3 Normal;
    
    void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 FragPos;
    in vec3 Normal;
    
    uniform vec3 lightPos;
    uniform vec3 viewPos;
    uniform vec3 objectColor;
    
    out vec4 FragColor;
    
    void main() {
        // Luz ambiental
        float ambientStrength = 0.3;
        vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
        
        // Luz difusa
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
        
        // Luz especular
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);
        
        // Resultado final
        vec3 result = (ambient + diffuse + specular) * objectColor;
        FragColor = vec4(result, 1.0);
    }
)";

Renderer::Renderer()
    : m_window(nullptr)
    , m_width(1024)
    , m_height(1024)
    , m_headless(false)
    , m_vao(0)
    , m_vbo(0)
    , m_shader(nullptr)
    , m_fbo(0)
    , m_colorAttachment(0)
    , m_depthAttachment(0)
    , m_backgroundColor(0.0f, 0.0f, 0.0f)
    , m_modelColor(0.7f, 0.7f, 0.7f)
    , m_cameraPos(0.0f, 0.0f, 5.0f)
    , m_cameraTarget(0.0f, 0.0f, 0.0f)
    , m_cameraYaw(0.0f)
    , m_cameraPitch(0.0f)
    , m_cameraDistance(5.0f)
    , m_hasModel(false)
    , m_initialized(false)
    , m_defaultCubeVAO(0)
    , m_defaultCubeVBO(0)
{
    // Inicializar matrices
    m_projectionMatrix = glm::mat4(1.0f);
    m_viewMatrix = glm::mat4(1.0f);
}

Renderer::~Renderer() {
    destroyGLResources();
    
    // Terminar GLFW
    if (m_window) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
}

bool Renderer::initialize(int width, int height, bool headless) {
    m_width = width;
    m_height = height;
    m_headless = headless;
    
    logFile << "Renderer::initialize() - Iniciando con tamaño " << width << "x" << height << std::endl;
    
    // Inicializar GLFW
    if (!glfwInit()) {
        logFile << "Error al inicializar GLFW" << std::endl;
        return false;
    }
    
    // Configurar opciones de GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Crear ventana
    m_window = glfwCreateWindow(width, height, "STL Renderer", NULL, NULL);
    if (!m_window) {
        logFile << "Error al crear ventana GLFW" << std::endl;
        glfwTerminate();
        return false;
    }
    
    // Hacer el contexto actual
    glfwMakeContextCurrent(m_window);
    
    // Activar VSync para evitar parpadeo
    glfwSwapInterval(1);
    
    // Cargar funciones de OpenGL con GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        logFile << "Error al inicializar GLAD" << std::endl;
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return false;
    }
    
    // Configurar viewport
    glViewport(0, 0, width, height);
    
    // Habilitar depth test
    glEnable(GL_DEPTH_TEST);
    
    // Crear shaders
    createShaders();
    
    // Configurar VAO y VBO
    setupBuffers();
    
    // Crear el cubo por defecto
    createDefaultCube();
    
    // Configurar matriz de proyección
    float aspectRatio = (float)width / (float)height;
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    logFile << "Matriz de proyección configurada, aspect ratio: " << aspectRatio << std::endl;
    
    // Matriz de vista inicial
    updateViewMatrix();
    
    logFile << "Renderer inicializado correctamente" << std::endl;
    m_initialized = true;
    return true;
}

bool Renderer::initializeHeadless(int width, int height) {
    m_width = width;
    m_height = height;
    m_headless = true;
    
    std::cout << "Inicializando renderer en modo headless: " << width << "x" << height << std::endl;
    
    // Inicializar GLFW
    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW en modo headless" << std::endl;
        return false;
    }
    
    // Configurar GLFW para modo headless
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    
    // Crear ventana oculta
    m_window = glfwCreateWindow(width, height, "STL Renderer Headless", NULL, NULL);
    if (!m_window) {
        std::cerr << "Error al crear ventana GLFW headless" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(m_window);
    
    // Inicializar GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error al inicializar GLAD en modo headless" << std::endl;
        return false;
    }
    
    // Configurar viewport
    glViewport(0, 0, width, height);
    
    // Crear shaders y buffers
    createShaders();
    setupBuffers();
    setupFramebuffer();
    createDefaultCube();
    
    // Configurar matriz de proyección (Ajustar FOV y distancias)
    float aspectRatio = (float)width / (float)height;
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    std::cout << "Matriz de proyección configurada, aspect ratio: " << aspectRatio << std::endl;
    
    // Configurar matriz de vista inicial
    m_cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    m_cameraYaw = 0.0f;
    m_cameraPitch = 0.0f;
    m_cameraDistance = 5.0f;
    updateViewMatrix();
    
    // Configuración OpenGL
    glEnable(GL_DEPTH_TEST);
    
    std::cout << "Renderer en modo headless inicializado correctamente" << std::endl;
    return true;
}

void Renderer::setBackgroundColor(const Color& color) {
    m_backgroundColor = color;
}

void Renderer::setModelColor(const Color& color) {
    m_modelColor = color;
}

void Renderer::setCameraPosition(const glm::vec3& position) {
    m_cameraPos = position;
    updateViewMatrix();
}

void Renderer::setCameraTarget(const glm::vec3& target) {
    m_cameraTarget = target;
    updateViewMatrix();
}

void Renderer::setCameraOrbit(float yaw, float pitch, float distance) {
    m_cameraYaw = yaw;
    m_cameraPitch = pitch;
    m_cameraDistance = distance;
    
    std::cout << "setCameraOrbit: yaw=" << yaw << ", pitch=" << pitch << ", distance=" << distance << std::endl;
    
    // Usar los ángulos directamente (ya están en radianes)
    float yawRad = yaw;
    float pitchRad = pitch;
    
    // Calcular posición de la cámara usando coordenadas esféricas
    float x = m_cameraTarget.x + distance * cos(pitchRad) * cos(yawRad);
    float y = m_cameraTarget.y + distance * sin(pitchRad);
    float z = m_cameraTarget.z + distance * cos(pitchRad) * sin(yawRad);
    
    m_cameraPos = glm::vec3(x, y, z);
    std::cout << "Posición calculada de la cámara: (" << m_cameraPos.x << ", " << m_cameraPos.y << ", " << m_cameraPos.z << ")" << std::endl;
    
    updateViewMatrix();
}

void Renderer::centerCamera() {
    if (m_hasModel) {
        // El modelo ya está centrado en el origen (0,0,0)
        m_cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        
        // Calcular dimensión máxima del modelo para ajustar la distancia
        glm::vec3 dimensions = (m_model.maxBounds - m_model.minBounds);
        float maxDimension = std::max(std::max(dimensions.x, dimensions.y), dimensions.z);
        
        // Ajustar distancia para asegurar que el modelo sea visible
        float dist = maxDimension * 1.5f;
        
        // Valores predeterminados optimizados para una mejor vista 3/4
        float yaw = 0.8f;    // ~45 grados
        float pitch = 0.5f;  // ~30 grados
        
        std::cout << "Cámara centrada en el origen. Dimensiones del modelo: (" 
                 << dimensions.x << ", " << dimensions.y << ", " << dimensions.z 
                 << "), Distancia: " << dist << std::endl;
        
        logFile << "Cámara centrada. Target: (0,0,0), Dimensiones: " 
                << dimensions.x << ", " << dimensions.y << ", " << dimensions.z
                << ", Distancia: " << dist << std::endl;
        
        // Establecer la órbita con los nuevos valores
        setCameraOrbit(yaw, pitch, dist);
    }
}

void Renderer::render() {
    if (!m_initialized) return;
    
    // Verificar si estamos dentro de un contexto ImGui
    ImGuiContext* context = ImGui::GetCurrentContext();
    if (context && context->WithinFrameScope) {
        // Esta función es llamada desde el interior de ImGui::Begin/End
        // Obtener el tamaño y posición de la ventana de ImGui actual
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        
        // Ajustar el viewport
        int viewport[4] = {
            (int)windowPos.x, 
            (int)(m_height - (windowPos.y + windowSize.y)),
            (int)windowSize.x, 
            (int)windowSize.y
        };
        
        // Proyección
        float aspectRatio = (float)viewport[2] / (float)viewport[3];
        m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        
        // Ajustar viewport a sección de ImGui
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        
        // Usar scissor test para limpiar solo el área de nuestra ventana
        glEnable(GL_SCISSOR_TEST);
        glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
        
        // Limpiar el contenido anterior
        glClearColor(m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Renderizar modelo
        renderModel();
        
        // Desactivar scissor test
        glDisable(GL_SCISSOR_TEST);
    } else {
        // Renderizado normal fuera de ImGui
        glViewport(0, 0, m_width, m_height);
        
        // Establecer color de fondo
        glClearColor(m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Renderizar modelo
        renderModel();
    }
}

void Renderer::renderModel() {
    // Activar el shader
    m_shader->use();
    
    // Establecer matrices de proyección y vista
    m_shader->setMat4("projection", m_projectionMatrix);
    m_shader->setMat4("view", m_viewMatrix);
    
    // Establecer colores y posiciones de luz con mejor iluminación
    m_shader->setVec3("objectColor", m_modelColor.r, m_modelColor.g, m_modelColor.b);
    m_shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);  // Luz blanca brillante
    m_shader->setVec3("lightPos", 2.0f, 5.0f, 2.0f);    // Posición de luz optimizada
    m_shader->setVec3("viewPos", m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
    
    // Calcular y establecer la matriz de modelo
    glm::mat4 model = glm::mat4(1.0f);
    m_shader->setMat4("model", model);
    
    // Renderizar el modelo si hay uno cargado
    if (m_hasModel && m_vao != 0) {
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, m_model.triangles.size() * 3);
        glBindVertexArray(0);
    } else {
        // Renderizar un cubo por defecto usando el VAO del cubo
        if (m_defaultCubeVAO != 0) {
            glBindVertexArray(m_defaultCubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36); // Un cubo tiene 36 vértices (6 caras * 2 triángulos * 3 vértices)
            glBindVertexArray(0);
        }
    }
}

bool Renderer::renderToFile(const std::string& filename, bool transparentBackground) {
    logFile << "=== INICIO RENDERTOFILE ===" << std::endl;
    logFile << "Renderizando a archivo: " << filename << (transparentBackground ? " (con fondo transparente)" : " (con fondo sólido)") << std::endl;
    
    // Verificar estado del modelo
    logFile << "Estado del modelo: hasModel=" << (m_hasModel ? "true" : "false") 
            << ", triangles=" << (m_hasModel ? std::to_string(m_model.triangles.size()) : "N/A") 
            << ", vao=" << m_vao 
            << ", defaultCubeVAO=" << m_defaultCubeVAO << std::endl;
    
    // Información sobre la configuración actual de la cámara
    logFile << "Cámara actual: posición=(" << m_cameraPos.x << ", " << m_cameraPos.y << ", " << m_cameraPos.z 
            << "), target=(" << m_cameraTarget.x << ", " << m_cameraTarget.y << ", " << m_cameraTarget.z 
            << "), yaw=" << m_cameraYaw << ", pitch=" << m_cameraPitch << ", distance=" << m_cameraDistance << std::endl;
    
    // Verificar que el framebuffer esté configurado
    if (m_fbo == 0) {
        logFile << "Error: Framebuffer no inicializado, intentando crear" << std::endl;
        setupFramebuffer();
        if (m_fbo == 0) {
            logFile << "Error: No se pudo crear el framebuffer" << std::endl;
            return false;
        }
    }
    
    // IMPORTANTE: No recalculamos la posición de la cámara ni nada
    // Usamos exactamente lo que ya está configurado desde renderSingleFile
    
    // Realizar renderizado al framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    
    // Verificar estado del framebuffer
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        logFile << "ERROR: El framebuffer no está completo" << std::endl;
    } else {
        logFile << "Framebuffer completo y listo para renderizar" << std::endl;
    }
    
    // Establecer el viewport al tamaño del framebuffer
    glViewport(0, 0, m_width, m_height);
    
    // Configurar la proyección para el renderizado
    float aspectRatio = (float)m_width / (float)m_height;
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    
    // Limpiar el buffer con el color adecuado
    if (transparentBackground) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    } else {
        glClearColor(m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, 1.0f);
    }
    
    // Limpiar buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Asegurarse de que el depth test esté habilitado
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Activar shader
    m_shader->use();
    
    // Establecer uniformes
    m_shader->setMat4("projection", m_projectionMatrix);
    m_shader->setMat4("view", m_viewMatrix);
    
    // Matriz modelo (sin transformaciones adicionales)
    glm::mat4 model = glm::mat4(1.0f);
    m_shader->setMat4("model", model);
    
    // Configuración de luz y color - usar los colores configurados
    m_shader->setVec3("objectColor", m_modelColor.r, m_modelColor.g, m_modelColor.b);
    m_shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f); // Luz blanca brillante
    
    // Posicionar la luz cerca de la cámara para buena iluminación (igual que en preview)
    glm::vec3 lightPos = m_cameraPos + glm::vec3(0.0f, 1.0f, 0.0f); // Luz justo encima de la cámara
    m_shader->setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
    m_shader->setVec3("viewPos", m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
    
    // Renderizar el modelo
    if (m_hasModel && !m_model.triangles.empty() && m_vao != 0) {
        logFile << "Renderizando modelo cargado" << std::endl;
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, m_model.triangles.size() * 3);
        glBindVertexArray(0);
    } else {
        // Renderizar el cubo por defecto si no hay modelo
        if (m_defaultCubeVAO != 0) {
            logFile << "Renderizando cubo por defecto" << std::endl;
            glBindVertexArray(m_defaultCubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        } else {
            logFile << "ERROR: No hay cubo por defecto disponible" << std::endl;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }
    }
    
    // Verificar errores de OpenGL
    int err = glGetError();
    if (err != 0) {
        logFile << "ERROR OpenGL: " << err << std::endl;
    }
    
    // Asegurarse de que todo se haya dibujado
    glFlush();
    glFinish();
    
    // Guardar a archivo
    bool success = saveImage(filename, transparentBackground);
    
    // Restaurar framebuffer por defecto
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    logFile << "=== FIN RENDERTOFILE ===" << std::endl;
    return success;
}

void Renderer::setModel(const Model& model) {
    logFile << "=== INICIO SETMODEL ===" << std::endl;
    m_model = model;
    m_hasModel = true;
    
    // Información básica del modelo
    logFile << "Modelo cargado: " << model.triangles.size() << " triángulos" << std::endl;
    logFile << "Centro: (" << model.center.x << ", " << model.center.y << ", " << model.center.z << ")" << std::endl;
    logFile << "Escala: " << model.scale << std::endl;
    logFile << "Límites - Min: (" << model.minBounds.x << ", " << model.minBounds.y << ", " << model.minBounds.z << ")" << std::endl;
    logFile << "Límites - Max: (" << model.maxBounds.x << ", " << model.maxBounds.y << ", " << model.maxBounds.z << ")" << std::endl;
    
    // Verificar que el modelo tenga triángulos
    if (model.triangles.empty()) {
        logFile << "ERROR: Intentando cargar un modelo sin triángulos" << std::endl;
        m_hasModel = false;
        logFile << "=== FIN SETMODEL (ERROR) ===" << std::endl;
        return;
    }
    
    // Mostrar algunos triángulos de ejemplo para diagnóstico
    logFile << "Muestreo de triángulos:" << std::endl;
    int sampleCount = std::min(3, static_cast<int>(model.triangles.size()));
    for (int i = 0; i < sampleCount; i++) {
        const auto& tri = model.triangles[i];
        logFile << "  Triángulo " << i << ":" << std::endl;
        for (int v = 0; v < 3; v++) {
            logFile << "    Vértice " << v << ": Pos=(" 
                    << tri.vertices[v].position.x << ", " 
                    << tri.vertices[v].position.y << ", " 
                    << tri.vertices[v].position.z << "), Norm=(" 
                    << tri.vertices[v].normal.x << ", " 
                    << tri.vertices[v].normal.y << ", " 
                    << tri.vertices[v].normal.z << ")" << std::endl;
        }
    }
    
    // Actualizar VBO con los datos del modelo
    std::vector<float> vertexData;
    vertexData.reserve(model.triangles.size() * 3 * 6); // 3 vértices por triángulo, 6 floats por vértice (3 pos + 3 normal)
    
    logFile << "Preparando datos normalizados con centro: (" << model.center.x << ", " << model.center.y << ", " << model.center.z 
            << "), escala: " << model.scale << std::endl;
    
    for (const auto& triangle : model.triangles) {
        for (int i = 0; i < 3; ++i) {
            // Normalizar la posición del vértice aplicando centrado y escalado
            glm::vec3 normalizedPos = (triangle.vertices[i].position - model.center) * model.scale;
            
            // Imprimir ejemplo de vértices para diagnóstico (solo algunos vértices)
            if (&triangle == &model.triangles[0] && i == 0) {
                logFile << "Primer vértice - Original: (" << triangle.vertices[i].position.x << ", " 
                        << triangle.vertices[i].position.y << ", " << triangle.vertices[i].position.z << ")" << std::endl;
                logFile << "Primer vértice - Normalizado: (" << normalizedPos.x << ", " 
                        << normalizedPos.y << ", " << normalizedPos.z << ")" << std::endl;
            }
            
            // Agregar posición normalizada
            vertexData.push_back(normalizedPos.x);
            vertexData.push_back(normalizedPos.y);
            vertexData.push_back(normalizedPos.z);
            
            // Agregar normal
            vertexData.push_back(triangle.vertices[i].normal.x);
            vertexData.push_back(triangle.vertices[i].normal.y);
            vertexData.push_back(triangle.vertices[i].normal.z);
        }
    }
    
    // Verificar que tenemos datos de vértices
    if (vertexData.empty()) {
        logFile << "ERROR: No se generaron datos de vértices" << std::endl;
        m_hasModel = false;
        logFile << "=== FIN SETMODEL (ERROR) ===" << std::endl;
        return;
    }
    
    logFile << "Generados " << vertexData.size() / 6 << " vértices para renderizado" << std::endl;
    
    // Configuración del VAO y VBO para el modelo
    glBindVertexArray(m_vao);
    logFile << "VAO activado: " << m_vao << std::endl;
    
    // Bind el VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // Cargar datos
    logFile << "Cargando " << vertexData.size() * sizeof(float) << " bytes en el VBO" << std::endl;
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
    
    // Verificar si hubo error
    int err = glGetError();
    if (err != 0) {
        logFile << "ERROR de OpenGL al cargar datos en el VBO: " << err << std::endl;
    }
    
    // Restaurar VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    logFile << "Modelo cargado con éxito en la GPU" << std::endl;
    logFile << "=== FIN SETMODEL ===" << std::endl;
}

void Renderer::createShaders() {
    // Utilizar la nueva clase Shader para crear los shaders
    m_shader = std::make_unique<Shader>(vertexShaderSource, fragmentShaderSource);
    
    if (m_shader) {
        logFile << "Shaders creados correctamente" << std::endl;
    } else {
        logFile << "Error al crear los shaders" << std::endl;
    }
}

void Renderer::setupBuffers() {
    // Crear VAO y VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    
    // Configurar VAO
    glBindVertexArray(m_vao);
    
    // Enlazar VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // Configurar atributos de vértices
    // Posición
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Desenlazar
    glBindVertexArray(0);
}

void Renderer::setupFramebuffer() {
    // Crear framebuffer
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    
    // Crear textura de color
    glGenTextures(1, &m_colorAttachment);
    glBindTexture(GL_TEXTURE_2D, m_colorAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorAttachment, 0);
    
    // Crear renderbuffer para profundidad
    glGenRenderbuffers(1, &m_depthAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthAttachment);
    
    // Verificar que el framebuffer esté completo
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer incompleto" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::destroyGLResources() {
    // Liberar recursos de OpenGL
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    
    if (m_defaultCubeVAO != 0) {
        glDeleteVertexArrays(1, &m_defaultCubeVAO);
        m_defaultCubeVAO = 0;
    }
    
    if (m_defaultCubeVBO != 0) {
        glDeleteBuffers(1, &m_defaultCubeVBO);
        m_defaultCubeVBO = 0;
    }
    
    // Liberar recursos del framebuffer
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    
    if (m_colorAttachment != 0) {
        glDeleteTextures(1, &m_colorAttachment);
        m_colorAttachment = 0;
    }
    
    if (m_depthAttachment != 0) {
        glDeleteRenderbuffers(1, &m_depthAttachment);
        m_depthAttachment = 0;
    }
    
    // El shader se liberará automáticamente por el unique_ptr
    m_shader.reset();
}

void Renderer::updateViewMatrix() {
    // Crear matriz de vista
    m_viewMatrix = glm::lookAt(m_cameraPos, m_cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    
    std::cout << "Matriz de vista actualizada. Posición: (" << m_cameraPos.x << ", " << m_cameraPos.y << ", " << m_cameraPos.z << "), Target: (" << m_cameraTarget.x << ", " << m_cameraTarget.y << ", " << m_cameraTarget.z << ")" << std::endl;
}

bool Renderer::saveImage(const std::string& filename, bool transparentBackground) {
    logFile << "=== INICIO SAVEIMAGE ===" << std::endl;
    logFile << "Guardando imagen " << m_width << "x" << m_height << " con formato " 
            << (transparentBackground ? "RGBA" : "RGB") << " en: " << filename << std::endl;
    
    // Leer píxeles del framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    
    // Formato: RGBA si es transparente, RGB si no
    unsigned int format = transparentBackground ? GL_RGBA : GL_RGB;
    int numChannels = transparentBackground ? 4 : 3;
    
    // Crear buffer para los datos de la imagen
    std::vector<unsigned char> buffer(m_width * m_height * numChannels);
    
    if (buffer.size() == 0 || buffer.data() == nullptr) {
        logFile << "ERROR: No se pudo asignar memoria para el buffer de imagen (" 
                << (m_width * m_height * numChannels) << " bytes)" << std::endl;
        return false;
    }
    
    logFile << "Buffer asignado: " << buffer.size() << " bytes" << std::endl;
    
    // Leer píxeles
    glReadPixels(0, 0, m_width, m_height, format, GL_UNSIGNED_BYTE, buffer.data());
    
    // Verificar si hubo error en glReadPixels
    int err = glGetError();
    if (err != 0) {
        logFile << "ERROR en glReadPixels: " << err << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
    
    logFile << "Lectura de píxeles completada, volteando imagen..." << std::endl;
    
    // Voltear imagen (OpenGL y stb_image tienen coordenadas y invertidas)
    for (int y = 0; y < m_height / 2; ++y) {
        for (int x = 0; x < m_width; ++x) {
            for (int c = 0; c < numChannels; ++c) {
                std::swap(buffer[(y * m_width + x) * numChannels + c],
                          buffer[((m_height - 1 - y) * m_width + x) * numChannels + c]);
            }
        }
    }
    
    // DIAGNÓSTICO: Detectar el contenido real de la imagen de manera más detallada
    bool allBlack = true;
    bool allWhite = true;
    long totalPixels = 0;
    long blackPixels = 0;
    long whitePixels = 0;
    
    // Comprobar todos los píxeles
    for (int y = 0; y < m_height; y += 10) {  // Sampling every 10 lines for efficiency
        for (int x = 0; x < m_width; x += 10) {  // Sampling every 10 columns
            totalPixels++;
            
            // Comprobar si el píxel es negro o blanco
            int pixelOffset = (y * m_width + x) * numChannels;
            bool isBlack = true;
            bool isWhite = true;
            
            for (int c = 0; c < std::min(3, numChannels); ++c) {
                if (buffer[pixelOffset + c] > 10) isBlack = false;  // Tolerancia para "casi negro"
                if (buffer[pixelOffset + c] < 240) isWhite = false; // Tolerancia para "casi blanco"
            }
            
            if (isBlack) blackPixels++;
            if (isWhite) whitePixels++;
            
            if (!isBlack) allBlack = false;
            if (!isWhite) allWhite = false;
        }
    }
    
    float blackPercent = (totalPixels > 0) ? (100.0f * blackPixels / totalPixels) : 0;
    float whitePercent = (totalPixels > 0) ? (100.0f * whitePixels / totalPixels) : 0;
    
    logFile << "DIAGNÓSTICO - Análisis de imagen: " << std::endl;
    logFile << "  Total píxeles muestreados: " << totalPixels << std::endl;
    logFile << "  Píxeles negros: " << blackPixels << " (" << blackPercent << "%)" << std::endl; 
    logFile << "  Píxeles blancos: " << whitePixels << " (" << whitePercent << "%)" << std::endl;
    
    if (allBlack || blackPercent > 95.0f) {
        logFile << "ADVERTENCIA: La imagen es casi completamente negra! Posible problema de renderizado." << std::endl;
    } else if (allWhite || whitePercent > 95.0f) {
        logFile << "ADVERTENCIA: La imagen es casi completamente blanca! Posible problema de iluminación." << std::endl;
    }
    
    // Guardar imagen a archivo
    logFile << "Guardando imagen usando stbi_write_png..." << std::endl;
    int result = stbi_write_png(filename.c_str(), m_width, m_height, numChannels,
                               buffer.data(), m_width * numChannels);
    
    // Volver al framebuffer por defecto
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    if (result == 0) {
        logFile << "ERROR: stbi_write_png falló al guardar la imagen" << std::endl;
    } else {
        logFile << "stbi_write_png completado correctamente, imagen guardada en: " << filename << std::endl;
    }
    
    logFile << "=== FIN SAVEIMAGE ===" << std::endl;
    return result != 0;
}

void Renderer::createDefaultCube() {
    // Vértices de un cubo simple con coordenadas de normales
    // Reducido al 70% del tamaño original para una mejor visualización
    float scale = 0.35f; // Reducido de 0.5 a 0.35 (70% del tamaño original)
    float cubeVertices[] = {
        // Posiciones            // Normales
        -scale, -scale, -scale,     0.0f,  0.0f, -1.0f,
         scale, -scale, -scale,     0.0f,  0.0f, -1.0f,
         scale,  scale, -scale,     0.0f,  0.0f, -1.0f,
         scale,  scale, -scale,     0.0f,  0.0f, -1.0f,
        -scale,  scale, -scale,     0.0f,  0.0f, -1.0f,
        -scale, -scale, -scale,     0.0f,  0.0f, -1.0f,

        -scale, -scale,  scale,     0.0f,  0.0f,  1.0f,
         scale, -scale,  scale,     0.0f,  0.0f,  1.0f,
         scale,  scale,  scale,     0.0f,  0.0f,  1.0f,
         scale,  scale,  scale,     0.0f,  0.0f,  1.0f,
        -scale,  scale,  scale,     0.0f,  0.0f,  1.0f,
        -scale, -scale,  scale,     0.0f,  0.0f,  1.0f,

        -scale,  scale,  scale,    -1.0f,  0.0f,  0.0f,
        -scale,  scale, -scale,    -1.0f,  0.0f,  0.0f,
        -scale, -scale, -scale,    -1.0f,  0.0f,  0.0f,
        -scale, -scale, -scale,    -1.0f,  0.0f,  0.0f,
        -scale, -scale,  scale,    -1.0f,  0.0f,  0.0f,
        -scale,  scale,  scale,    -1.0f,  0.0f,  0.0f,

         scale,  scale,  scale,     1.0f,  0.0f,  0.0f,
         scale,  scale, -scale,     1.0f,  0.0f,  0.0f,
         scale, -scale, -scale,     1.0f,  0.0f,  0.0f,
         scale, -scale, -scale,     1.0f,  0.0f,  0.0f,
         scale, -scale,  scale,     1.0f,  0.0f,  0.0f,
         scale,  scale,  scale,     1.0f,  0.0f,  0.0f,

        -scale, -scale, -scale,     0.0f, -1.0f,  0.0f,
         scale, -scale, -scale,     0.0f, -1.0f,  0.0f,
         scale, -scale,  scale,     0.0f, -1.0f,  0.0f,
         scale, -scale,  scale,     0.0f, -1.0f,  0.0f,
        -scale, -scale,  scale,     0.0f, -1.0f,  0.0f,
        -scale, -scale, -scale,     0.0f, -1.0f,  0.0f,

        -scale,  scale, -scale,     0.0f,  1.0f,  0.0f,
         scale,  scale, -scale,     0.0f,  1.0f,  0.0f,
         scale,  scale,  scale,     0.0f,  1.0f,  0.0f,
         scale,  scale,  scale,     0.0f,  1.0f,  0.0f,
        -scale,  scale,  scale,     0.0f,  1.0f,  0.0f,
        -scale,  scale, -scale,     0.0f,  1.0f,  0.0f
    };

    // Crear buffers
    glGenVertexArrays(1, &m_defaultCubeVAO);
    glGenBuffers(1, &m_defaultCubeVBO);

    // Enlazar VAO
    glBindVertexArray(m_defaultCubeVAO);

    // Enlazar VBO y cargar datos
    glBindBuffer(GL_ARRAY_BUFFER, m_defaultCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Configurar atributos de vértices
    // Posición (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal (3 floats)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Desenlazar
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "Cubo predeterminado creado para vista previa" << std::endl;
    logFile << "Cubo predeterminado creado para vista previa" << std::endl;
}

void Renderer::setProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane) {
    // Crear una nueva matriz de proyección con los parámetros dados
    m_projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    
    // Debug
    std::cout << "Matriz de proyección recalculada con FOV: " << fov 
              << ", AspectRatio: " << aspectRatio 
              << ", Near: " << nearPlane 
              << ", Far: " << farPlane << std::endl;
} 
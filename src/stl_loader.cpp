#include "stl_loader.h"
#include "renderer.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <limits>

StlLoader::StlLoader() {
    // Inicializar modelo vacío
    m_model.minBounds = glm::vec3(std::numeric_limits<float>::max());
    m_model.maxBounds = glm::vec3(std::numeric_limits<float>::lowest());
    m_model.center = glm::vec3(0.0f);
    m_model.scale = 1.0f;
}

StlLoader::~StlLoader() {
    // Limpiar recursos si es necesario
    std::cout << "StlLoader destruido\n";
}

bool StlLoader::loadModel(const std::string& filename, Renderer& renderer) {
    // Limpiar modelo anterior
    m_model.triangles.clear();
    
    std::cout << "Intentando cargar modelo: " << filename << std::endl;
    
    // Intentar abrir el archivo en modo binario
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
        return false;
    }
    
    // Verificar tamaño del archivo
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::cout << "Tamaño del archivo: " << fileSize << " bytes" << std::endl;
    
    if (fileSize < 84) { // 80 bytes header + 4 bytes triangleCount
        std::cerr << "Archivo STL inválido: demasiado pequeño" << std::endl;
        file.close();
        return false;
    }
    
    // Leer los primeros bytes para determinar si es ASCII o binario
    char header[6];
    file.read(header, 5);
    header[5] = '\0';
    file.close();
    
    bool success = false;
    
    // Comprobar si es ASCII (comienza con "solid")
    if (std::strncmp(header, "solid", 5) == 0) {
        std::cout << "Detectado formato ASCII" << std::endl;
        success = loadAsciiSTL(filename);
    } else {
        std::cout << "Detectado formato binario" << std::endl;
        success = loadBinarySTL(filename);
    }
    
    if (success) {
        // Calcular información del modelo
        calculateModelInfo();
        
        // NUEVO: Trasladar todos los vértices para que el centro del bounding box esté en (0,0,0)
        std::cout << "Trasladando modelo al origen (0,0,0)..." << std::endl;
        glm::vec3 translationVector = -m_model.center; // Negativo del centro para moverlo a (0,0,0)
        
        // Aplicar traslación a todos los vértices
        for (auto& triangle : m_model.triangles) {
            for (int i = 0; i < 3; ++i) {
                triangle.vertices[i].position += translationVector;
            }
        }
        
        // NUEVO: Rotar el modelo para que Z sea el eje vertical (como en miniaturas de D&D)
        std::cout << "Rotando modelo para correcta orientación Z-up..." << std::endl;

        // Aplicar rotación a todos los vértices para una correcta orientación de miniaturas
        for (auto& triangle : m_model.triangles) {
            for (int i = 0; i < 3; ++i) {
                // Almacenar coordenadas originales
                float originalY = triangle.vertices[i].position.y;
                float originalZ = triangle.vertices[i].position.z;
                
                // Intercambiar Y y Z con la orientación correcta para miniaturas e invertir Z
                triangle.vertices[i].position.y = originalZ;
                triangle.vertices[i].position.z = -originalY; // Negamos Y original para invertir Z
                
                // También ajustar las normales
                originalY = triangle.vertices[i].normal.y;
                originalZ = triangle.vertices[i].normal.z;
                triangle.vertices[i].normal.y = originalZ;
                triangle.vertices[i].normal.z = -originalY; // Invertimos también la normal
            }
        }
        
        // Recalcular límites y centro después de la traslación y rotación
        m_model.minBounds = glm::vec3(std::numeric_limits<float>::max());
        m_model.maxBounds = glm::vec3(std::numeric_limits<float>::lowest());
        
        // Recalcular límites del modelo
        for (const auto& triangle : m_model.triangles) {
            for (int i = 0; i < 3; ++i) {
                const glm::vec3& pos = triangle.vertices[i].position;
                
                // Actualizar min/max
                m_model.minBounds = glm::min(m_model.minBounds, pos);
                m_model.maxBounds = glm::max(m_model.maxBounds, pos);
            }
        }
        
        m_model.center = glm::vec3(0.0f, 0.0f, 0.0f); // El centro sigue en el origen
        
        // Registrar la traslación en el log
        std::cout << "Modelo trasladado al origen y rotado. Nuevos límites - Min: (" 
                << m_model.minBounds.x << ", " << m_model.minBounds.y << ", " << m_model.minBounds.z 
                << "), Max: (" << m_model.maxBounds.x << ", " << m_model.maxBounds.y << ", " << m_model.maxBounds.z << ")\n";
        
        // Informar sobre el modelo cargado
        std::cout << "Modelo cargado exitosamente. Triángulos: " << m_model.triangles.size() << std::endl;
        std::cout << "Dimensiones: Min(" << m_model.minBounds.x << ", " << m_model.minBounds.y << ", " << m_model.minBounds.z 
                  << "), Max(" << m_model.maxBounds.x << ", " << m_model.maxBounds.y << ", " << m_model.maxBounds.z << ")" << std::endl;
        std::cout << "Centro: (" << m_model.center.x << ", " << m_model.center.y << ", " << m_model.center.z 
                  << "), Escala: " << m_model.scale << std::endl;
        
        // Verificar que tenemos al menos un triángulo
        if (m_model.triangles.empty()) {
            std::cerr << "Advertencia: El modelo no contiene triángulos" << std::endl;
            return false;
        }
        
        // Verificar algunos vértices como muestra
        if (!m_model.triangles.empty()) {
            const auto& firstTri = m_model.triangles.front();
            std::cout << "Primer triángulo - Vértice 1: (" 
                    << firstTri.vertices[0].position.x << ", " 
                    << firstTri.vertices[0].position.y << ", " 
                    << firstTri.vertices[0].position.z << ")\n";
            
            // También verificar la normal
            std::cout << "Primer triángulo - Normal: (" 
                    << firstTri.vertices[0].normal.x << ", " 
                    << firstTri.vertices[0].normal.y << ", " 
                    << firstTri.vertices[0].normal.z << ")\n";
        }
        
        // Enviar modelo al renderer
        renderer.setModel(m_model);
    } else {
        std::cerr << "Error al cargar el modelo" << std::endl;
    }
    
    return success;
}

bool StlLoader::loadBinarySTL(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo binario: " << filename << "\n";
        return false;
    }
    
    // Saltar los 80 bytes del encabezado
    file.seekg(80);
    
    // Leer número de triángulos
    uint32_t numTriangles;
    file.read(reinterpret_cast<char*>(&numTriangles), sizeof(uint32_t));
    
    std::cout << "Número de triángulos en archivo binario: " << numTriangles << "\n";
    
    // Reservar espacio para todos los triángulos
    m_model.triangles.reserve(numTriangles);
    
    // Leer cada triángulo
    for (uint32_t i = 0; i < numTriangles; ++i) {
        Triangle tri;
        
        // Leer normal
        glm::vec3 normal;
        file.read(reinterpret_cast<char*>(&normal.x), sizeof(float) * 3);
        
        // Leer los tres vértices
        for (int j = 0; j < 3; ++j) {
            glm::vec3 position;
            file.read(reinterpret_cast<char*>(&position.x), sizeof(float) * 3);
            
            tri.vertices[j].position = position;
            tri.vertices[j].normal = normal;
        }
        
        // Saltar los 2 bytes de atributo
        file.seekg(2, std::ios::cur);
        
        // Agregar triángulo al modelo
        m_model.triangles.push_back(tri);
    }
    
    file.close();
    std::cout << "Carga binaria completada. Triángulos leídos: " << m_model.triangles.size() << "\n";
    return !m_model.triangles.empty();
}

bool StlLoader::loadAsciiSTL(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo ASCII: " << filename << "\n";
        return false;
    }
    
    std::string line;
    glm::vec3 normal;
    int vertexIndex = 0;
    Triangle currentTriangle;
    
    std::cout << "Iniciando lectura de archivo ASCII STL\n";
    
    // Leer línea por línea
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;
        
        if (keyword == "facet") {
            // Leer normal
            iss >> keyword; // "normal"
            iss >> normal.x >> normal.y >> normal.z;
        } else if (keyword == "vertex") {
            // Leer vértice
            glm::vec3 position;
            iss >> position.x >> position.y >> position.z;
            
            currentTriangle.vertices[vertexIndex].position = position;
            currentTriangle.vertices[vertexIndex].normal = normal;
            
            vertexIndex++;
            
            // Si hemos leído los tres vértices, agregar el triángulo al modelo
            if (vertexIndex == 3) {
                m_model.triangles.push_back(currentTriangle);
                vertexIndex = 0;
            }
        }
    }
    
    file.close();
    std::cout << "Carga ASCII completada. Triángulos leídos: " << m_model.triangles.size() << "\n";
    return !m_model.triangles.empty();
}

void StlLoader::calculateModelInfo() {
    if (m_model.triangles.empty()) {
        std::cerr << "Error: No hay triángulos para calcular información del modelo\n";
        return;
    }
    
    // Reiniciar límites
    m_model.minBounds = glm::vec3(std::numeric_limits<float>::max());
    m_model.maxBounds = glm::vec3(std::numeric_limits<float>::lowest());
    
    // Calcular límites del modelo
    for (const auto& triangle : m_model.triangles) {
        for (int i = 0; i < 3; ++i) {
            const glm::vec3& pos = triangle.vertices[i].position;
            
            // Actualizar min/max
            m_model.minBounds = glm::min(m_model.minBounds, pos);
            m_model.maxBounds = glm::max(m_model.maxBounds, pos);
        }
    }
    
    // Calcular centro del modelo
    m_model.center = (m_model.minBounds + m_model.maxBounds) * 0.5f;
    
    // Calcular dimensiones
    glm::vec3 dimensions = m_model.maxBounds - m_model.minBounds;
    std::cout << "Dimensiones del modelo: (" << dimensions.x << ", " << dimensions.y << ", " << dimensions.z << ")\n";
    
    // Calcular escala del modelo para normalizar su tamaño
    float maxDimension = std::max(std::max(dimensions.x, dimensions.y), dimensions.z);
    
    if (maxDimension > 0.0f) {
        // Normalizar a un rango que permita visualizarlo bien
        // Un valor menor hará que el modelo parezca más grande
        m_model.scale = 1.0f / maxDimension;
    } else {
        m_model.scale = 1.0f;
    }
    
    std::cout << "Centro del modelo: (" << m_model.center.x << ", " << m_model.center.y << ", " << m_model.center.z << ")\n";
    std::cout << "Factor de escala: " << m_model.scale << "\n";
} 
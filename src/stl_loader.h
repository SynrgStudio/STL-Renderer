#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "model.h"

class Renderer;

class StlLoader {
public:
    StlLoader();
    ~StlLoader();

    // Cargar un modelo STL desde archivo y enviarlo al renderer
    bool loadModel(const std::string& filename, Renderer& renderer);
    
    // Obtener información del modelo cargado
    const Model& getModel() const { return m_model; }

private:
    Model m_model;
    
    // Métodos privados para cargar diferentes formatos de STL
    bool loadBinarySTL(const std::string& filename);
    bool loadAsciiSTL(const std::string& filename);
    
    // Calcular información del modelo después de cargarlo
    void calculateModelInfo();
}; 
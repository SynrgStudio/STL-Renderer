#include "stl_loader.h"
#include "renderer.h"

bool StlLoader::loadModel(const std::string& filename, Renderer& renderer) {
    if (!loadModel(filename)) return false;
    renderer.setModel(m_model);
    return true;
}

#ifndef __WATER_MESH_H__
#define __WATER_MESH_H__

#include "waterMeshChunk.hpp"
#include "abstractPhysics.hpp"
#include <vector>

class WaterMesh {
private:
    std::vector<WaterMeshChunk*> chunks;
public:
    WaterMesh();
    ~WaterMesh();

    void show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const;
    
    void process(AbstractPhysics *phys, float t);
};

#endif

#ifndef __WATER_MESH_H__
#define __WATER_MESH_H__

#include "waterMeshChunk.hpp"
#include "abstractPhysics.hpp"
#include <map>

class WaterMesh {
private:
    std::map<std::pair<int, int>, WaterMeshChunk*> chunks;

    friend class WaterMeshChunk;

public:
    WaterMesh();
    ~WaterMesh();

    void show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const;
    void process(AbstractPhysics *phys, float t);
    const WaterMeshChunk* getChunk(int x, int z) const;
};

#endif

#ifndef __WATER_MESH_CHUNK_H__
#define __WATER_MESH_CHUNK_H__

#include "util/glew.hpp"
#include "GLFW/glfw3.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include "util/shader.hpp"
#include "util/camera.hpp"

class WaterMesh;

class WaterMeshChunk {
public:
    enum MeshType {
        INNER = 0,
        EDGE_NX = 1,
        EDGE_PX = 2,
        EDGE_NZ = 4,
        EDGE_PZ = 8,
        CORN_NXNZ = EDGE_NX | EDGE_NZ,
        CORN_PXNZ = EDGE_PX | EDGE_NZ,
        CORN_NXPZ = EDGE_NX | EDGE_PZ,
        CORN_PXPZ = EDGE_PX | EDGE_PZ,
        OUTER = EDGE_NX | EDGE_PX | EDGE_NZ | EDGE_PZ,
    };

private:
    int posx, posz;

    int width, height;
    float size;
    int meshType;
    glm::vec3 offset;
    int elementsCount;

    GLuint vao, vbo, ebo;
    GLuint normalMapID;

    Shader showShader;
    Shader physShader, normShader;
    
    // Debug
    GLuint debugVAO, debugVBO;
    Shader txShader;

    std::vector<std::pair<int, int> > getElements() const;

    friend class WaterMesh;
    WaterMesh *parent;

public:
    WaterMeshChunk(int wh, float size, int type, int xs, int ys);
    ~WaterMeshChunk();

    void computePhysics(float absTime) const;
    void show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const;
    void showDebugImage(const glm::mat4 &m_ortho, float time) const;

    int getWidth() const;
    int getHeight() const;
    float getSize() const;
    glm::vec3 getOffset() const;
    int getMeshType() const;
    std::pair<int, int> getChunkPos() const;
};

#endif

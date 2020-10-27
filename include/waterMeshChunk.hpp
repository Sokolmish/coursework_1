#ifndef __WATER_MESH_H__
#define __WATER_MESH_H__

#include "util/glew.hpp"
#include "GLFW/glfw3.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include "util/shader.hpp"
#include "util/camera.hpp"

class WaterMeshChunk {
public:
    enum MeshType {
        INNER = 0,
        EDGE_NX = 1,
        EDGE_PX = 2,
        EDGE_NZ = 4,
        EDGE_PZ = 8,
        OUTER = 15 // EDGE_NX | EDGE_PX | EDGE_NZ | EDGE_PZ
    };

    struct Node {
        int origx, origy;
        glm::vec3 pos, norm;
    };

private:
    int width, height;
    float size;
    std::vector<Node> *nodes;
    int meshType;

    GLuint vao, vbo, ebo;
    GLfloat *buff;
    int elementsCount;

    Shader shader, normShader;

    std::vector<std::pair<int, int> > getElements() const;

public:
    WaterMeshChunk(int w, int h, float size, int type);
    ~WaterMeshChunk();

    void show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const;

    int getWidth() const;
    int getHeight() const;
    float getSize() const;

    const std::vector<Node>& getNodes() const;
    std::vector<Node>& getNodes();

    int getMeshType() const;
};

#endif

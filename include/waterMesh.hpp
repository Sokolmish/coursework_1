#ifndef __WATER_MESH_H__
#define __WATER_MESH_H__

#include "glew.hpp"
#include "GLFW/glfw3.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include "shader.hpp"
#include "camera.hpp"

class WaterMesh {
public:
    struct Node {
        int origx, origy;
        glm::vec3 pos, norm;
    };

private:
    int width, height;
    float size;
    std::vector<Node> *nodes;

    GLuint vao, vbo, ebo;
    GLfloat *buff;

    Shader shader, normShader;

    friend class AbstractPhysics;

public:
    WaterMesh(int w, int h, float size);
    ~WaterMesh();

    void show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const;

    int getWidth() const;
    int getHeight() const;
    float getSize() const;

    const std::vector<Node>& getNodes() const;
    std::vector<Node>& getNodes();
};

#endif

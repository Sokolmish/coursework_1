#ifndef __WATER_MESH_H__
#define __WATER_MESH_H__

#include "glew.hpp"
#include "GLFW/glfw3.h"
#include <glm/mat4x4.hpp>
#include <vector>
#include "shader.hpp"

class WaterMesh {
private:
    int width, height;
    float size;
    std::vector<glm::vec3> *nodes;

    GLuint vao, vbo, ebo;
    GLfloat *buff;

    Shader shader;

    friend class AbstractPhysics;

public:
    WaterMesh(int w, int h, float size);
    ~WaterMesh();

    void show(const glm::mat4 &m_proj_view, bool isMesh) const;

    int getWidth() const;
    int getHeight() const;
    float getSize() const;

    const std::vector<glm::vec3>& getNodes() const;
    std::vector<glm::vec3>& getNodes();
};

#endif
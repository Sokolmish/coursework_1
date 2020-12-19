#ifndef __ENV_SKY_H__
#define __ENV_SKY_H__

#include "util/glew.hpp"
#include "GLFW/glfw3.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "util/shader.hpp"
#include <string>

class EnvSky {
private:
    Shader sunShader;
    GLuint vbo, vao;

    int corners = 20;

    glm::vec3 sunDir;
    float sunDist;
    float sunRad;
    glm::vec3 sunCol = glm::vec3(255.f, 249.f, 23.f) / 255.f;

public:
    EnvSky() = default;
    EnvSky(const std::string &envMap, const glm::vec3 &_sunDir, float sunDist, float sunRadius);
    
    void show(const glm::mat4 &m_proj_view) const;

    void setSunCol(const glm::vec3 &col);
    
    float getSunAngle() const;
    glm::vec3 getSunColor() const;
    glm::vec3 getSunDir() const;
};

#endif
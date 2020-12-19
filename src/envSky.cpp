#include "../include/envSky.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

EnvSky::EnvSky(const std::string &envMap, const glm::vec3 &_sunDir, float sunDist, float sunRadius) {
    sunShader = Shader("./shaders/sun.vert", "./shaders/sun.frag");

    this->sunDir = glm::normalize(_sunDir);
    this->sunDist = sunDist;
    this->sunRad = sunRadius;

    glm::vec3 cent = sunDist * sunDir;
    glm::vec3 cur = glm::normalize(glm::vec3((cent.y + cent.z) / cent.x, 1.f, 1.f));
    cur *= sunRadius;
    glm::mat4 rot = glm::rotate(glm::mat4(1.f), 2.f * (float)M_PI / corners, sunDir);

    GLfloat *buff = new GLfloat[3 * corners];
    for (int i = 0; i < corners; i++) {
        glm::vec3 pos = cent + cur;
        for (int j = 0; j < 3; j++)
            buff[i * 3 + j] = pos[j];
        cur = glm::vec3(rot * glm::vec4(cur, 1.f));
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * corners * 3, buff, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    delete[] buff;
}
    
void EnvSky::show(const glm::mat4 &m_proj_view) const {
    sunShader.use();
    sunShader.setUniform("projection", m_proj_view);
    sunShader.setUniform("sunColor", sunCol);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glBindVertexArray(vao);
    glDrawArrays(GL_POLYGON, 0, corners);

    glBindVertexArray(0);
}

void EnvSky::setSunCol(const glm::vec3 &col) {
    this->sunCol = col;
}

float EnvSky::getSunAngle() const {
    return atanf(sunRad / sunDist);
}

glm::vec3 EnvSky::getSunColor() const {
    return sunCol;
}

glm::vec3 EnvSky::getSunDir() const {
    return sunDir;
}
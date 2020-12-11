#ifndef __WATER_MESH_CHUNK_H__
#define __WATER_MESH_CHUNK_H__

#include "util/glew.hpp"
#include "GLFW/glfw3.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "util/shader.hpp"
#include "util/camera.hpp"
#include "wave2.hpp"

#include <vector>
#include <initializer_list>
#include <random>
#include <complex>

class WaterMeshChunk {
private:
    int width, height;
    float size;
    glm::vec3 offset;

    int elementsCount;

    GLuint vao, vbo, ebo;
    GLuint normalMapID;
    GLuint wavesBuffID;

    std::vector<std::pair<Wave2, std::complex<float> > > waves;
    glm::vec3 windDir;
    float windSpeed;

    mutable std::mt19937 gen; // Standard mersenne twister engine
    mutable std::uniform_real_distribution<float> dis;

    Shader showShader;
    Shader physShader, normShader;

    // Debug
    GLuint debugVAO, debugVBO;
    Shader txShader;

    std::vector<std::pair<int, int> > getElements() const;
    void fillWavesBuff(float absTime) const;
    std::complex<float> computeH0(const Wave2 &w) const;

public:
    WaterMeshChunk(int wh, float size, int xs, int ys);

    void addWave(const Wave2 &w);
    void addWaves(const std::initializer_list<Wave2> &ws);
    void clearWaves();

    void computePhysics(float absTime) const;
    void show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const;
    void showDebugImage(const glm::mat4 &m_ortho, float time) const;

    void setWind(const glm::vec3 &dir, float speed);

    int getWidth() const;
    int getHeight() const;
    float getSize() const;
    glm::vec3 getOffset() const;
};

#endif

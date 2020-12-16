#ifndef __WATER_MESH_CHUNK_H__
#define __WATER_MESH_CHUNK_H__

#include "util/glew.hpp"
#include "GLFW/glfw3.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "util/shader.hpp"
#include "util/camera.hpp"

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

    glm::vec3 windDir;
    float windSpeed;
    float amplitude;

    typedef std::normal_distribution<float> rand_distrib; // uniform_real_distribution<float>
    mutable std::mt19937 gen; // Standard mersenne twister engine
    mutable rand_distrib dis;

    Shader showShader;
    Shader normShader;

    int logN;
    Shader htShader, buttShader, fourShader, perlinShader;
    GLuint h0Tex, buttTex, perlinTex;
    GLuint htHTex, htxTex, htzTex, ppTex;

    // Debug
    GLuint debugVAO, debugVBO;
    Shader txShader;

    std::vector<std::pair<int, int> > getElements() const;
    void initDebug();
    void initTextures();
    void ifft(GLuint src, int bPos) const;

    GLuint loadTextureFromFile(const std::string &path, GLenum wrap, GLenum filter) const;
    GLuint generateEmptyTexture(int width, int height, GLenum type) const;
    GLuint generateButterflyTexture(int N) const;
    GLuint generateH0Texture() const;

public:
    WaterMeshChunk(int wh, float size, int xs, int ys);

    void computePhysics(float absTime) const;
    void show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const;
    void showDebugImage(const glm::mat4 &m_ortho, float time) const;

    void setWind(const glm::vec3 &dir, float speed);
    void setAmplitude(float amp);
    void update();

    int getWidth() const;
    int getHeight() const;
    float getSize() const;
    glm::vec3 getOffset() const;
};

#endif

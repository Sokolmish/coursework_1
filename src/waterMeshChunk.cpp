#include "../include/waterMeshChunk.hpp"
#include "../include/waterMesh.hpp"
#include <cassert>
#include <math.h>
#include <iostream>

WaterMeshChunk::WaterMeshChunk(int wh, float size, int type, int xs, int ys) {
    assert(wh > 0 && size > 1e-4f);

    offset = glm::vec3(xs * wh * size, 0.f, ys * wh * size);
    //// wh += (wh % 2 == 0) ? 1 : 0; // Warning?
    width = wh;
    height = wh;
    this->size = size;
    this->meshType = type;

    // glGenVertexArrays(1, &vao);
    // glGenBuffers(1, &vbo);
    // glBindVertexArray(vao);
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // glBufferData(GL_ARRAY_BUFFER, width * height * 6 * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);
    // size_t stride = 6 * sizeof(GLfloat);
    // glEnableVertexAttribArray(0); // Location
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(0 * sizeof(GLfloat)));
    // glEnableVertexAttribArray(1); // Normal
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));

    // glBindVertexArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTextures(sizeof(texIds) / sizeof(GLuint), texIds);

    glBindTexture(GL_TEXTURE_2D, texIds[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, texIds[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    showShader = Shader("./shaders/poly.vert", "./shaders/poly.frag");
    physShader = Shader("./shaders/phys.comp");
    normShader = Shader("./shaders/normc.comp");

    // Init debug
    txShader = Shader("./shaders/tx.vert", "./shaders/tx.frag");
    GLfloat vertices[12][4] = {
        { 0, 450, 0, 0 }, { 0, 0,   0, 1 }, { 450, 0,   1, 1 },
        { 0, 450, 0, 0 }, { 450, 0, 1, 1 }, { 450, 450, 1, 0 },
        { 500, 450, 0, 0 }, { 500, 0, 0, 1 }, { 950, 0, 1, 1 },
        { 500, 450, 0, 0 }, { 950, 0, 1, 1 }, { 950, 450, 1, 0 }
    };
    glGenVertexArrays(1, &debugVAO);
    glGenBuffers(1, &debugVBO);
    glBindVertexArray(debugVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12 * 4, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void WaterMeshChunk::show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const {
    showShader.use();

    showShader.setUniform("is_mesh", isMesh);
    if (isMesh)
        showShader.setUniform("mesh_color", 0.1, 0.1, 0.1); // 0.03, 0.1, 0.95

    showShader.setUniform("m_proj_view", m_proj_view);
    showShader.setUniform("eye_pos", cam.pos);
    showShader.setUniform("view_dir", cam.getViewDir());

    showShader.setUniform("mat.ambient", glm::vec3(0.03f, 0.391f, 0.9993f) * 0.4f);
    showShader.setUniform("mat.diffuse", glm::vec3(0.03f, 0.391f, 0.9993f) * 0.35f);
    showShader.setUniform("mat.specular", glm::vec3(0.2f, 0.2f, 0.2f));
    showShader.setUniform("mat.exponent", 70.f);

    showShader.setUniform("globalAmb", glm::vec3(0.35f, 0.35f, 0.45f));
    showShader.setUniform("sunDir", glm::vec3(0.5f, 0.5f, 0.0f));

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    if (isMesh)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, width * height * 6 * sizeof(GLfloat), buff);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, elementsCount, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void WaterMeshChunk::showDebugImage(const glm::mat4 &m_ortho, float time) const {
    physShader.use();
    physShader.setUniform("heightMap", 0);
    physShader.setUniform("time", time);
    physShader.setUniform("meshSize", size);
    glBindImageTexture(0, texIds[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    normShader.use();
    normShader.setUniform("heightMap", 0);
    normShader.setUniform("normalMap", 1);
    glBindImageTexture(0, texIds[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texIds[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    txShader.use();
    txShader.setUniform("projection", m_ortho);
    txShader.setUniform("tex", 0);
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(debugVAO);
    glBindTexture(GL_TEXTURE_2D, texIds[0]); 
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, texIds[1]); 
    glDrawArrays(GL_TRIANGLES, 6, 6);
    glBindVertexArray(0);
}

// Getters

int WaterMeshChunk::getWidth() const {
    return this->width;
}

int WaterMeshChunk::getHeight() const {
    return this->height;
}

float WaterMeshChunk::getSize() const {
    return this->size;
}

int WaterMeshChunk::getMeshType() const {
    return this->meshType;
}

glm::vec3 WaterMeshChunk::getOffset() const {
    return offset;
}

std::pair<int, int> WaterMeshChunk::getChunkPos() const {
    return std::pair<int, int>(posx, posz);
}

WaterMeshChunk::~WaterMeshChunk() {
    // delete[] buff;
}
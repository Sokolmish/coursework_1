#include "../include/waterMeshChunk.hpp"
#include "../include/util/image.hpp"
#include <cassert>
#include <math.h>
#include <iostream>

using namespace std::complex_literals;

static constexpr bool useTrueRandom = false;
static uint rseed = 3907355480; // 3060

template<class T>
inline void push_tr(std::vector<T> &dst, T p1, T p2, T p3) {
    dst.push_back(p1);
    dst.push_back(p2);
    dst.push_back(p3);
}

std::vector<std::pair<int, int> > WaterMeshChunk::getElements() const {
    std::vector<std::pair<int, int> > tElements;
    // TODO: downsampling
    for (int zz = 0; zz < height - 1; zz++) {
        for (int xx = 0; xx < width - 1; xx++) {
            push_tr(tElements, { xx, zz }, { xx + 1, zz }, { xx + 1, zz + 1 });
            push_tr(tElements, { xx, zz }, { xx + 1, zz + 1 }, { xx, zz + 1 });
        }
    }
    return tElements;
}


WaterMeshChunk::WaterMeshChunk(int wh, float size, int xs, int ys) {
    assert(wh > 0 && size > 1e-4f);

    this->offset = glm::vec3(xs * wh * size, 0.f, ys * wh * size);
    this->width = wh;
    this->height = wh;
    this->size = size;

    if constexpr(useTrueRandom) {
        rseed = (std::random_device())();
        std::cout << "Rd = " << rseed << std::endl;
    }
    gen = std::mt19937(rseed);
    dis = std::normal_distribution<float>(0.f, 1.f);

    // EBO computation
    auto tElements = getElements();
    elementsCount = tElements.size();
    GLuint *ebuff = new GLuint[elementsCount];
    size_t ind = 0;
    for (const auto &e : tElements) {
        int x = e.first;
        int z = e.second;
        ebuff[ind++] = z * width + x;
    }

    // Main buffers init
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * width * height * 3, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * elementsCount, ebuff, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    delete[] ebuff;

    // Texture init
    glGenTextures(1, &normalMapID);
    glBindTexture(GL_TEXTURE_2D, normalMapID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Waves buffer init
    glGenBuffers(1, &wavesBuffID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, wavesBuffID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLfloat) * 7, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Shaders loading
    showShader = Shader("./shaders/water.vert", "./shaders/water.frag");
    physShader = Shader("./shaders/phys.comp");
    normShader = Shader("./shaders/norm.comp");

    // Init debug
    txShader = Shader("./shaders/tx.vert", "./shaders/tx.frag");
    GLfloat vertices[12][4] = {
        { 0, 300, 0, 0 }, { 0, 0,   0, 1 }, { 300, 0,   1, 1 },
        { 0, 300, 0, 0 }, { 300, 0, 1, 1 }, { 300, 300, 1, 0 },
        { 500, 300, 0, 0 }, { 500, 0, 0, 1 }, { 800, 0, 1, 1 },
        { 500, 300, 0, 0 }, { 800, 0, 1, 1 }, { 800, 300, 1, 0 }
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

    showShader.setUniform("gWidth", width * size);
    showShader.setUniform("gHeight", height * size);
    showShader.setUniform("normalMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, normalMapID);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    if (isMesh)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, elementsCount, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void WaterMeshChunk::showDebugImage(const glm::mat4 &m_ortho, float time) const {
    txShader.use();
    txShader.setUniform("projection", m_ortho);
    txShader.setUniform("tex", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(debugVAO);
    glBindTexture(GL_TEXTURE_2D, normalMapID);
    glDrawArrays(GL_TRIANGLES, 6, 6);
    glBindVertexArray(0);
}

void WaterMeshChunk::computePhysics(float absTime) const {
    fillWavesBuff(absTime);

    physShader.use();
    physShader.setUniform("time", absTime);
    physShader.setUniform("meshSize", size);
    physShader.setUniform("wavesCount", (int)waves.size());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, wavesBuffID);
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    normShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo);
    glBindImageTexture(1, normalMapID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void WaterMeshChunk::fillWavesBuff(float absTime) const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, wavesBuffID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * waves.size() * 7, nullptr, GL_STATIC_DRAW);
    GLfloat *ptr = static_cast<GLfloat*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY));

    uint ind = 0;
    for (const auto &w : waves) {
        auto coeff = w.second * exp(1if * w.first.w * absTime) + conj(w.second * exp(-1if * w.first.w * absTime));

        ptr[ind++] = w.first.dir.x; // 0
        ptr[ind++] = w.first.dir.y; // 1
        ptr[ind++] = w.first.dir.z; // 2
        ptr[ind++] = w.first.A;     // 3
        ptr[ind++] = w.first.w;     // 4
        ptr[ind++] = coeff.real();  // 5
        ptr[ind++] = coeff.imag();  // 6
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// Waves

std::complex<float> WaterMeshChunk::computeH0(const Wave2 &w) const {
    float cosphi = glm::dot(windDir, glm::normalize(w.dir));
    float L = windSpeed * windSpeed / 9.81f;
    float Ph = w.A * cosphi * cosphi / (w.k * w.k * w.k * w.k) * expf(-1.f / (w.k * L));
    float xi_re = dis(gen);
    float xi_im = dis(gen);
    return (float)M_SQRT1_2 * (xi_re + 1if * xi_im) * sqrtf(Ph);
}

void WaterMeshChunk::addWave(const Wave2 &w) {
    waves.push_back({ w, computeH0(w) });
    // fillWavesBuff();
}

void WaterMeshChunk::addWaves(const std::initializer_list<Wave2> &ws) {
    for (const auto &w : ws)
        waves.push_back({ w, computeH0(w) });
    // fillWavesBuff();
}

void WaterMeshChunk::clearWaves() {
    waves.clear();
}

void WaterMeshChunk::setWind(const glm::vec3 &dir, float speed) {
    windDir = glm::normalize(dir);
    windSpeed = speed;
    for (auto &w : waves)
        w = { w.first, computeH0(w.first) };
    // fillWavesBuff();
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

glm::vec3 WaterMeshChunk::getOffset() const {
    return offset;
}

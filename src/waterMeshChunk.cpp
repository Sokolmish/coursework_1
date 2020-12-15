#include "../include/waterMeshChunk.hpp"
#include "../include/util/image.hpp"
#include <cassert>
#include <math.h>
#include <iostream>
#include "../include/util/utility.hpp"

using namespace std::complex_literals;

static constexpr bool useTrueRandom = true;
static uint rseed = 3907355480; // 3060

template<class T>
inline void push_tr(std::vector<T> &dst, T p1, T p2, T p3) {
    dst.push_back(p1);
    dst.push_back(p2);
    dst.push_back(p3);
}

std::vector<std::pair<int, int> > WaterMeshChunk::getElements() const {
    std::vector<std::pair<int, int> > tElements;
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
    dis = rand_distrib(0.f, 1.f);

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

    logN = log2i(width);
    h0Shader = Shader("./shaders/h0.comp");
    htShader = Shader("./shaders/ht.comp");
    buttShader = Shader("./shaders/butt.comp");
    fourShader = Shader("./shaders/fourier.comp");
    convShader = Shader("./shaders/conv.comp");
    xiTex = generateGaussTexture(width, height);
    h0Tex = generateEmptyTexture(width, height, GL_FLOAT);
    htHeighTex = generateEmptyTexture(width, height, GL_FLOAT);
    htxTex = generateEmptyTexture(width, height, GL_FLOAT);
    htzTex = generateEmptyTexture(width, height, GL_FLOAT);
    buttTex = generateButterflyTexture(width);
    ppTex = generateEmptyTexture(width, height, GL_FLOAT);
    resHTex = generateEmptyTexture(width, height, GL_FLOAT);
    resXTex = generateEmptyTexture(width, height, GL_FLOAT);
    resZTex = generateEmptyTexture(width, height, GL_FLOAT);

    // Init debug
    txShader = Shader("./shaders/tx.vert", "./shaders/tx.frag"); // sht
    GLfloat vertices[18][4] = {
        { 0, 300, 0, 1 }, { 0, 0,   0, 0 }, { 300, 0,   1, 0 },
        { 0, 300, 0, 1 }, { 300, 0, 1, 0 }, { 300, 300, 1, 1 },
        { 500, 300, 0, 1 }, { 500, 0, 0, 0 }, { 800, 0, 1, 0 },
        { 500, 300, 0, 1 }, { 800, 0, 1, 0 }, { 800, 300, 1, 1 },
        { 0, 600, 0, 1 }, { 0,   0, 0, 0 }, { 900, 0, 1, 0 },
        { 0, 600, 0, 1 }, { 900, 0, 1, 0 }, { 900, 600, 1, 1 }
    };
    glGenVertexArrays(1, &debugVAO);
    glGenBuffers(1, &debugVBO);
    glBindVertexArray(debugVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 18 * 4, vertices, GL_STATIC_DRAW);
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
    glBindVertexArray(debugVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, htHeighTex);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, resHTex);
    glDrawArrays(GL_TRIANGLES, 6, 6);
    glBindVertexArray(0);
}

void WaterMeshChunk::computePhysics(float time) const {
    h0Shader.use();
    h0Shader.setUniform("L", width * size);
    h0Shader.setUniform("N", width);
    h0Shader.setUniform("Amp", 400.f);
    h0Shader.setUniform("windDir", windDir);
    h0Shader.setUniform("windSpeed", windSpeed);
    glBindImageTexture(0, xiTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, h0Tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    htShader.use();
    htShader.setUniform("L", width * size);
    htShader.setUniform("N", width);
    htShader.setUniform("time", time);
    glBindImageTexture(0, h0Tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, htHeighTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(2, htxTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(3, htzTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    ifft(htHeighTex, resHTex);
    ifft(htxTex, resXTex);
    ifft(htzTex, resZTex);

    convShader.use();
    convShader.setUniform("meshSize", size);
    glBindImageTexture(0, resHTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, resXTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(2, resZTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, vbo);
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    normShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo);
    glBindImageTexture(1, normalMapID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void WaterMeshChunk::ifft(GLuint src, GLuint dst) const {
    int pp = 0;
    buttShader.use();
    buttShader.setUniform("dir", (int)0);
    glBindImageTexture(0, buttTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, src, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(2, ppTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    for (int i = 0; i < logN; i++) {
        buttShader.setUniform("stage", (int)i);
        buttShader.setUniform("pp", (int)pp);
        glDispatchCompute(width, height, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        pp = 1 - pp;
    }
    buttShader.setUniform("dir", (int)1);
    for (int i = 0; i < logN; i++) {
        buttShader.setUniform("stage", i);
        buttShader.setUniform("pp", pp);
        glDispatchCompute(width, height, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        pp = 1 - pp;
    }

    fourShader.use();
    fourShader.setUniform("pp", pp);
    fourShader.setUniform("N", width);
    glBindImageTexture(0, src, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, ppTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(2, dst, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void WaterMeshChunk::fillWavesBuff() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, wavesBuffID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * waves.size() * 6, nullptr, GL_STATIC_DRAW);
    GLfloat *ptr = static_cast<GLfloat*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY));

    uint ind = 0;
    for (const auto &w : waves) {
        ptr[ind++] = w.dir.x; // 0
        ptr[ind++] = w.dir.y; // 1
        ptr[ind++] = w.dir.z; // 2
        ptr[ind++] = w.A;     // 3
        ptr[ind++] = w.w;     // 4
        ptr[ind++] = w.k;     // 5
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// Waves

void WaterMeshChunk::addWave(const Wave2 &w) {
    waves.push_back(w);
    fillWavesBuff();
}

void WaterMeshChunk::addWaves(const std::initializer_list<Wave2> &ws) {
    for (const auto &w : ws)
        waves.push_back(w);
    fillWavesBuff();
}

void WaterMeshChunk::clearWaves() {
    waves.clear();
}

void WaterMeshChunk::setWind(const glm::vec3 &dir, float speed) {
    windDir = glm::normalize(dir);
    windSpeed = speed;
}

// Texture generators

GLuint WaterMeshChunk::generateGaussTexture(int width, int height) const {
    GLfloat *buff = new GLfloat[width * height * 4];
    for (int j = 0; j < width * height * 4; j++)
        buff[j] = dis(gen);
    
    GLuint id;    
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, buff);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] buff;
    return id;
}

GLuint WaterMeshChunk::loadTextureFromFile(const std::string &path, GLenum wrap, GLenum filter) const {
    ImageRGB img = ImageRGB::fromFile(path);
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.getWidth(), img.getHeight(), 0,
                    GL_RGB, GL_UNSIGNED_BYTE, img.getData());
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

GLuint WaterMeshChunk::generateEmptyTexture(int width, int height, GLenum type) const {
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, type, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

GLuint WaterMeshChunk::generateButterflyTexture(int N) const {
    int logN = log2i(N);

    GLfloat *buff = new GLfloat[N * logN * 4];
    for (int x = 0; x < logN; x++) {
        for (int y = 0; y < N; y++) {
            float k = fmodf(y * ((float)N / (float)(1 << (x + 1))), N);
            float twiddleRe = cosf(2.f * M_PI * k / (float) N);
            float twiddleIm = sinf(2.f * M_PI * k / (float) N);
            int bSpan = 1 << x;
            bool bWing = fmodf(y, 1 << (x + 1)) < (1 << x);

            if (x == 0) {
                if (bWing) {
                    buff[((y * logN) + x) * 4 + 0] = twiddleRe;
                    buff[((y * logN) + x) * 4 + 1] = twiddleIm;
                    buff[((y * logN) + x) * 4 + 2] = reverseBits(y, logN);
                    buff[((y * logN) + x) * 4 + 3] = reverseBits(y + 1, logN);
                }
                else {
                    buff[((y * logN) + x) * 4 + 0] = twiddleRe;
                    buff[((y * logN) + x) * 4 + 1] = twiddleIm;
                    buff[((y * logN) + x) * 4 + 2] = reverseBits(y - 1, logN);
                    buff[((y * logN) + x) * 4 + 3] = reverseBits(y, logN);
                }
            }
            else {
                if (bWing) {
                    buff[((y * logN) + x) * 4 + 0] = twiddleRe;
                    buff[((y * logN) + x) * 4 + 1] = twiddleIm;
                    buff[((y * logN) + x) * 4 + 2] = y;
                    buff[((y * logN) + x) * 4 + 3] = y + bSpan;
                }
                else {
                    buff[((y * logN) + x) * 4 + 0] = twiddleRe;
                    buff[((y * logN) + x) * 4 + 1] = twiddleIm;
                    buff[((y * logN) + x) * 4 + 2] = y - bSpan;
                    buff[((y * logN) + x) * 4 + 3] = y;
                }
            }
        }
    }
    
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, logN, N, 0, GL_RGBA, GL_FLOAT, buff);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] buff;
    return id;
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

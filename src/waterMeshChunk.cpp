#include "../include/waterMeshChunk.hpp"
#include "../include/util/image.hpp"
#include "../include/util/utility.hpp"
#include <cmath>
#include <iostream>

#define WG_SIZE 16

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
    this->logN = log2i(width);

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
    configGlTexture(GL_CLAMP_TO_EDGE, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Shaders loading
    showShader = Shader("./shaders/water.vert", "./shaders/water.frag");
    normShader = Shader("./shaders/norm.comp");

    perlinShader = Shader("./shaders/perlin.comp");

    htShader =   Shader("./shaders/ht.comp");
    buttShader = Shader("./shaders/butt.comp");
    fourShader = Shader("./shaders/fourier.comp");

    // Fourier buffer-textures allocation
    htHTex = generateEmptyTexture(width, height, GL_FLOAT);
    htxTex = generateEmptyTexture(width, height, GL_FLOAT);
    htzTex = generateEmptyTexture(width, height, GL_FLOAT);
    ppTex =  generateEmptyTexture(width, height, GL_FLOAT);

    // Init debug
    initDebug();
}

void WaterMeshChunk::update() {
    initTextures();
}

void WaterMeshChunk::show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const {
    showShader.use();

    showShader.setUniform("is_mesh", isMesh);
    if (isMesh)
        showShader.setUniform("mesh_color", 0.1, 0.1, 0.1); // 0.03, 0.1, 0.95

    showShader.setUniform("m_proj_view", m_proj_view);
    showShader.setUniform("eye_pos", cam.pos);

    showShader.setUniform("globalAmb", globalAmb);
    showShader.setUniform("sunDir", sunDir);

    showShader.setUniform("mat.ambient", ambient); 
    showShader.setUniform("mat.diffuse", diffuse);
    showShader.setUniform("mat.specular", specular);
    showShader.setUniform("mat.exponent", specExpoenent);

    showShader.setUniform("gWidth", width * size);
    showShader.setUniform("gHeight", height * size);

    showShader.setUniform("normalMap", 0);
    showShader.setUniform("perlinNoise", 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, normalMapID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, perlinTex);

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

void WaterMeshChunk::initTextures() {
    buttTex = generateButterflyTexture(width);
    h0Tex = generateH0Texture();
    
    int perlinTexSize = 256;
    perlinTex = generateEmptyTexture(perlinTexSize, perlinTexSize, GL_FLOAT);
    perlinShader.use();
    glBindImageTexture(0, perlinTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(perlinTexSize / WG_SIZE, perlinTexSize / WG_SIZE, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void WaterMeshChunk::computePhysics(float time) const {
    htShader.use();
    htShader.setUniform("L", width * size);
    htShader.setUniform("N", width);
    htShader.setUniform("time", time);
    glBindImageTexture(0, h0Tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, htHTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(2, htxTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(3, htzTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width / WG_SIZE, height / WG_SIZE, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    ifft(htxTex, 0);
    ifft(htHTex, 1);
    ifft(htzTex, 2);

    normShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo);
    glBindImageTexture(1, normalMapID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width / WG_SIZE, height / WG_SIZE, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void WaterMeshChunk::ifft(GLuint src, int bPos) const {
    GLenum barrier = GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;

    int pp = 0;
    buttShader.use();
    buttShader.setUniform("dir", (int)0);
    glBindImageTexture(0, buttTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, src, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(2, ppTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    for (int i = 0; i < logN; i++) {
        buttShader.setUniform("stage", i);
        buttShader.setUniform("pp", pp);
        glDispatchCompute(width / WG_SIZE, height / WG_SIZE, 1);
        glMemoryBarrier(barrier);
        pp = 1 - pp;
    }
    buttShader.setUniform("dir", (int)1);
    for (int i = 0; i < logN; i++) {
        buttShader.setUniform("stage", i);
        buttShader.setUniform("pp", pp);
        glDispatchCompute(width / WG_SIZE, height / WG_SIZE, 1);
        glMemoryBarrier(barrier);
        pp = 1 - pp;
    }

    fourShader.use();
    fourShader.setUniform("pp", pp);
    fourShader.setUniform("N", width);
    fourShader.setUniform("buffPos", bPos);
    fourShader.setUniform("meshSize", size);
    glBindImageTexture(0, src, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, ppTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vbo);
    glDispatchCompute(width / WG_SIZE, height / WG_SIZE, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

// Debug

void WaterMeshChunk::initDebug() {
    txShader = Shader("./shaders/tx.vert", "./shaders/tx.frag");
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

void WaterMeshChunk::showDebugImage(const glm::mat4 &m_ortho, float time) const {
    htShader.use();
    htShader.setUniform("L", width * size);
    htShader.setUniform("N", width);
    htShader.setUniform("time", time);
    glBindImageTexture(0, h0Tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, htHTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(2, htxTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(3, htzTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width / WG_SIZE, height / WG_SIZE, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    
    txShader.use();
    txShader.setUniform("projection", m_ortho);
    txShader.setUniform("tex", 0);
    glBindVertexArray(debugVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, perlinTex);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, htHTex);
    glDrawArrays(GL_TRIANGLES, 6, 6);
    glBindVertexArray(0);
}

// Texture generators

GLuint WaterMeshChunk::loadTextureFromFile(const std::string &path, GLenum wrap, GLenum filter) const {
    ImageRGB img = ImageRGB::fromFile(path);
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    configGlTexture(wrap, filter);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.getWidth(), img.getHeight(), 0,
                    GL_RGB, GL_UNSIGNED_BYTE, img.getData());
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

GLuint WaterMeshChunk::generateEmptyTexture(int width, int height, GLenum type) const {
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    configGlTexture(GL_CLAMP_TO_EDGE, GL_LINEAR);
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
            int bSpan = 1 << x;
            bool bWing = fmodf(y, 1 << (x + 1)) < (1 << x);

            buff[((y * logN) + x) * 4 + 0] = cosf(2.f * M_PI * k / (float) N);
            buff[((y * logN) + x) * 4 + 1] = sinf(2.f * M_PI * k / (float) N);
            if (x == 0) {
                if (bWing) {
                    buff[((y * logN) + x) * 4 + 2] = reverseBits(y, logN);
                    buff[((y * logN) + x) * 4 + 3] = reverseBits(y + bSpan, logN);
                }
                else {
                    buff[((y * logN) + x) * 4 + 2] = reverseBits(y - bSpan, logN);
                    buff[((y * logN) + x) * 4 + 3] = reverseBits(y, logN);
                }
            }
            else {
                if (bWing) {
                    buff[((y * logN) + x) * 4 + 2] = y;
                    buff[((y * logN) + x) * 4 + 3] = y + bSpan;
                }
                else {
                    buff[((y * logN) + x) * 4 + 2] = y - bSpan;
                    buff[((y * logN) + x) * 4 + 3] = y;
                }
            }
        }
    }

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    configGlTexture(GL_CLAMP_TO_EDGE, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, logN, N, 0, GL_RGBA, GL_FLOAT, buff);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] buff;
    return id;
}

GLuint WaterMeshChunk::generateH0Texture() const {  
    GLuint id;
    GLfloat *buff = new GLfloat[width * height * 4];

    float Lx = width * size;
    float Lz = height * size;
    float E = (windSpeed * windSpeed) / 9.81f;

    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            glm::vec4 rnd = gaussRand({ dis(gen), dis(gen), dis(gen), dis(gen) });
            float nx = x - width / 2.f;
            float nz = z - height / 2.f;
           
            glm::vec2 k = glm::vec2(2.f * (float)M_PI * nx / Lx, 2.f * (float)M_PI * nz / Lz);
            glm::vec2 nk = glm::normalize(k);
            float mg = std::max(glm::length(k), 1e-4f);
            float mg2 = mg * mg;

            float h0 = std::min(4000.f, std::max(-4000.f,
                sqrtf(amplitude / (mg2 * mg2)) *
                powf(fabsf(nk.x * windDir.x + nk.y * windDir.z), 6.f) *
                expf(-1.f / (mg2 * E * E)) * (float)M_SQRT1_2
            ));

            int base = ((z * width) + x) * 4;
            buff[base + 0] = rnd[0] * h0;
            buff[base + 1] = rnd[1] * h0;
            buff[base + 2] = rnd[2] * h0;
            buff[base + 3] = rnd[3] * h0 * -1.f; // conj
        }
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    configGlTexture(GL_CLAMP_TO_EDGE, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, buff);
    glBindTexture(GL_TEXTURE_2D, 0);
    delete[] buff;
    return id;
}

// Getters and setters

void WaterMeshChunk::setAmplitude(float amp) {
    this->amplitude = amp;
}

void WaterMeshChunk::setWind(const glm::vec3 &dir, float speed) {
    this->windDir = glm::normalize(dir);
    this->windSpeed = speed;
}

void WaterMeshChunk::setSun(const glm::vec3 &dir) {
    this->sunDir = dir;
}

void WaterMeshChunk::setGlobalAmbient(const glm::vec3 &color) {
    this->globalAmb = color;
}

void WaterMeshChunk::setDiffuse(const glm::vec3 &color) {
    this->ambient = color;
}

void WaterMeshChunk::setAmbient(const glm::vec3 &color) {
    this->diffuse = color;
}

void WaterMeshChunk::setSpecular(const glm::vec3 &color, float exp) {
    this->specular = color;
    this->specExpoenent = exp;
}



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

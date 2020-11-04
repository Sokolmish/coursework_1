#include "../include/sineSumPhysics.hpp"
#include <math.h>

SineSumPhysics::Wave::Wave() {}

SineSumPhysics::Wave::Wave(const glm::vec3 &dir, float amp, float freq, float velocity, float stepness) :
    dir(glm::normalize(dir)), amp(amp), freq(freq), velocity(velocity), stepness(stepness) {}

SineSumPhysics::SineSumPhysics(std::initializer_list<Wave> list) {
    waves = std::vector<Wave>();
    for (auto e : list) {
        e.dir = glm::normalize(e.dir);
        waves.push_back(e);
    }

    GLfloat *wavesBuff = new GLfloat[waves.size() * 6 * sizeof(GLfloat)];
    for (size_t i = 0; i < waves.size(); i++) {
        wavesBuff[i * 6 + 0] = waves[i].dir.x;
        wavesBuff[i * 6 + 1] = waves[i].dir.y;
        wavesBuff[i * 6 + 2] = waves[i].amp;
        wavesBuff[i * 6 + 3] = waves[i].freq;
        wavesBuff[i * 6 + 4] = waves[i].velocity;
        wavesBuff[i * 6 + 5] = waves[i].stepness;
    }

    compShader = Shader("./shaders/sineSum.comp");

    glGenBuffers(sizeof(ssbo) / sizeof(GLuint), ssbo);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 64 * 64 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, waves.size() * 6 * sizeof(GLfloat), wavesBuff, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

    delete[] wavesBuff;
}

void SineSumPhysics::addWave(const glm::vec3 &dir, float amp, float freq, float velocity, float stepness) {
    waves.push_back(Wave(dir, amp, freq, velocity, stepness));
}

void SineSumPhysics::process(WaterMeshChunk &mesh, float t) {
    compShader.use();

    compShader.setUniform("meshSize", mesh.getSize());
    compShader.setUniform("time", t);

    // TODO: output buffer reallocation
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo[1]);

    glDispatchCompute(mesh.getWidth(), mesh.getHeight(), 1);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    GLfloat *ptr = (GLfloat*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    // for (int i = 0; i < 16 * 16; i++) {
    size_t ind = 0;
    for (auto &node : mesh.getNodes()) {
        node.pos.y = ptr[ind++];
    }
    // }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

    // for (auto &node : mesh.getNodes()) {
    //     float dx = 0;
    //     float dz = 0;
    //     static_cast<void>(dx);
    //     static_cast<void>(dz);

    //     node.pos.y = 0.f;
    //     for (const auto &w : waves) {
    //         float S = (w.dir.x * node.pos.x + w.dir.z * node.pos.z + w.velocity * t) * w.freq;
    //         node.pos.y += w.amp * powf((sinf(S) + 1.f) * .5f, w.stepness);
            
    //         if constexpr (computeNormal) {
    //             float gradPart = w.amp * w.stepness * powf(.5f, w.stepness) * powf(sinf(S) + 1.f, w.stepness - 1) * cosf(S);
    //             dx += gradPart * w.dir.x;
    //             dz += gradPart * w.dir.z;
    //         }
    //     }
    //     if constexpr (computeNormal)
    //         node.norm = glm::normalize(glm::vec3(-dx, 1.f, -dz));
    // }
}

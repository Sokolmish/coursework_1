#include "../include/sineSumPhysics.hpp"
#include <math.h>

static const bool computeNormal = false;

SineSumPhysics::Wave::Wave() {}

SineSumPhysics::Wave::Wave(const glm::vec3 &dir, float amp, float freq, float velocity, float stepness) :
    dir(glm::normalize(dir)), amp(amp), freq(freq), velocity(velocity), stepness(stepness) {}

SineSumPhysics::SineSumPhysics() : waves(std::vector<Wave>()) {}

SineSumPhysics::SineSumPhysics(std::initializer_list<Wave> list) {
    waves = std::vector<Wave>();
    for (auto e : list) {
        e.dir = glm::normalize(e.dir);
        waves.push_back(e);
    }
}

void SineSumPhysics::addWave(const glm::vec3 &dir, float amp, float freq, float velocity, float stepness) {
    waves.push_back(Wave(dir, amp, freq, velocity, stepness));
}

void SineSumPhysics::process(WaterMeshChunk &mesh, float t) {
    for (auto &node : mesh.getNodes()) {
        float dx = 0;
        float dz = 0;
        static_cast<void>(dx);
        static_cast<void>(dz);

        node.pos.y = 0.f;
        for (const auto &w : waves) {
            float S = (w.dir.x * node.pos.x + w.dir.z * node.pos.z + w.velocity * t) * w.freq;
            node.pos.y += w.amp * powf((sinf(S) + 1.f) * .5f, w.stepness);
            
            if constexpr (computeNormal) {
                float gradPart = w.amp * w.stepness * powf(.5f, w.stepness) * powf(sinf(S) + 1.f, w.stepness - 1) * cosf(S);
                dx += gradPart * w.dir.x;
                dz += gradPart * w.dir.z;
            }
        }
        if constexpr (computeNormal)
            node.norm = glm::normalize(glm::vec3(-dx, 1.f, -dz));
    }
}

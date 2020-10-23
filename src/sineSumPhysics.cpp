#include "../include/sineSumPhysics.hpp"
#include <math.h>

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

void SineSumPhysics::process(WaterMesh &mesh, float t) {
    for (auto &node : mesh.getNodes()) {
        node.pos.y = 0.f;
        for (const auto &w : waves) {
            float S = (w.dir.x * node.pos.x + w.dir.z * node.pos.z + w.velocity * t) * w.freq;
            node.pos.y += w.amp * powf((sinf(S) + 1.f) / 2.f, w.stepness);
        }
    }
}

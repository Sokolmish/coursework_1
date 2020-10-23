#include "../include/dumbPhysics.hpp"
#include <math.h>

DumbPhysics::DumbPhysics(float amp, float freq, float velocity) {
    this->amp = amp;
    this->freq = freq;
    this->velocity = velocity;
}

void DumbPhysics::process(WaterMesh &mesh, float t) {
    for (auto &node : mesh.getNodes()) {
        node.pos.y = amp * (sinf((node.origx + velocity * t) * freq) + 1.f) / 2.f;
    }
}

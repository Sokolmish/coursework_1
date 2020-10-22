#include "../include/dumbPhysics.hpp"
#include <math.h>

DumbPhysics::DumbPhysics(float amp, float freq, float velocity) {
    this->amp = amp;
    this->freq = freq;
    this->velocity = velocity;
}

void DumbPhysics::process(WaterMesh &mesh, float t) {
    for (int xx = 0; xx < mesh.getWidth(); xx++) {
        for (int zz = 0; zz < mesh.getHeight(); zz++) {
            mesh.getNodes()[zz * mesh.getWidth() + xx].y = amp * (sinf(xx * freq + velocity * freq * t) + 1.f) / 2.f;
        }
    }
}
#ifndef __WAVE_1_H__
#define __WAVE_1_H__

#include <glm/vec3.hpp>

struct Wave1 {
    glm::vec3 dir;
    float amp; // amplitude
    float freq; // frequency
    float vel; // velocity
    float st; // stepness

    Wave1() = default;
    Wave1(const glm::vec3 &dir) : dir(dir), amp(1.f), freq(1.f), vel(1.f), st(1.f) {}
    Wave1(const glm::vec3 &dir, float a, float f, float v, float s) :
        dir(dir), amp(a), freq(f), vel(v), st(s) {}
};

#endif
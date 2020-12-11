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

// Wave1(glm::vec3{ 1.f, 0.f, -0.18f },     4.18f, 0.19f, 4.98f, 1.87f)
// Wave1(glm::vec3{ 0.5f, 0.f, 0.9f },      1.15f, 0.98f, 0.97f, 2.47f)
// Wave1(glm::vec3{ -0.27f, 0.f, 0.14f },   1.91f, 0.91f, 0.12f, 3.92f)
// Wave1(glm::vec3{ .15f, 0.f, 0.54f },     1.32f, 1.87f, 1.02f, 2.37f)

#endif

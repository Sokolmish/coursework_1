#ifndef __WAVE_2_H__
#define __WAVE_2_H__

#include <cmath>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>

struct Wave2 {
    static constexpr float g = 9.81f;

    float A; // Amplitude
    float lambda; // Wavelength

    float k; // Wavenumber
    float w; // Phase speed

    glm::vec3 dir;

    Wave2() = default;
    Wave2(const glm::vec3 &dir, float amp, float wavelen) {
        this->A = amp;
        this->lambda = wavelen;
        this->k = 2.f * M_PI / lambda;
        this->w = sqrtf(g * k);

        this->dir = glm::normalize(dir) * k;
    }
};

#endif

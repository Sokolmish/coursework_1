#ifndef __SINE_SUM_PHYSICS_H__
#define __SINE_SUM_PHYSICS_H__

#include "abstractPhysics.hpp"
#include <vector>
#include <initializer_list>
#include <glm/vec3.hpp>

class SineSumPhysics : public AbstractPhysics {
public:
    struct Wave {
        glm::vec3 dir;
        float amp, freq, velocity, stepness;

        Wave();
        Wave(const glm::vec3 &dir, float amp, float freq, float velocity, float stepness);
    };

private:
    std::vector<Wave> waves;

public:
    SineSumPhysics();
    SineSumPhysics(std::initializer_list<Wave> list);

    void addWave(const glm::vec3 &dir, float amp, float freq, float velocity, float stepness);

    void process(WaterMesh &mesh, float t) override;
};

#endif

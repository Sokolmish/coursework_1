#ifndef __DUMB_PHYSICS_H__
#define __DUMB_PHYSICS_H__

#include "abstractPhysics.hpp"

class DumbPhysics : public AbstractPhysics {
private:
    float amp;
    float freq;
    float velocity;
public:
    DumbPhysics(float amp, float freq, float velocity);
    void process(WaterMeshChunk &mesh, float t) override;
};

#endif

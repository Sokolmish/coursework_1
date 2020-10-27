#ifndef __ABSTRACT_PHYSICS_H__
#define __ABSTRACT_PHYSICS_H__

#include "waterMeshChunk.hpp"

class AbstractPhysics {
public:
    virtual ~AbstractPhysics() {};
    virtual void process(WaterMeshChunk &mesh, float t) = 0;
};

#endif

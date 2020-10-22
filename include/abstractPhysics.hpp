#ifndef __ABSTRACT_PHYSICS_H__
#define __ABSTRACT_PHYSICS_H__

#include "waterMesh.hpp"

class AbstractPhysics {
public:
    virtual void process(WaterMesh &mesh, float t) = 0;
};

#endif
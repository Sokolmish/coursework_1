#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/vec3.hpp>
#include <math.h>
#include <glm/trigonometric.hpp>

struct Camera {
    glm::vec3 pos;
    float yaw, pitch, roll;
    float zoom;

    Camera() : pos(glm::vec3(0.f)), yaw(0), pitch(0), roll(0), zoom(1.f) {}
    Camera(const glm::vec3 &pos, float yaw, float pitch) : pos(pos), yaw(yaw), pitch(pitch), roll(0), zoom(1.f) {}

    glm::vec3 getViewDir() const {
        return glm::vec3(cosf(pitch) * sinf(yaw), sinf(pitch), -cosf(pitch) * cosf(yaw));
    }
    glm::vec3 getMoveDir() const {
        return glm::vec3(-sinf(yaw), 0, cosf(yaw));
    }
    glm::vec3 getLeftDir() const {
        return glm::vec3(cosf(yaw), 0, sinf(yaw));
    }

    void setPos(float x, float y, float z) {
        this->pos = glm::vec3(x, y, z);
    }
    void setViewDeg(float yaw, float pitch) {
        this->yaw = glm::radians(yaw);
        this->pitch = glm::radians(pitch);
    }
    void setViewRad(float yaw, float pitch) {
        this->yaw = yaw;
        this->pitch = pitch;
    }
};

#endif
#ifndef __UTILITY_H__
#define __UTILITY_H__

#include "glew.hpp"
#include "GLFW/glfw3.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <string>

inline float stepYaw(float yaw, float d) {
    yaw = fmodf(yaw - d, 2 * M_PI);
    if (yaw < 0)
        return 2 * M_PI + yaw;
    else
        return yaw;
}

inline float stepPitch(float pitch, float d) {
    pitch -= d;
    if (pitch > M_PI_2)
        return M_PI_2;
    else if (pitch < -M_PI_2)
        return -M_PI_2;
    else
        return pitch;
}

std::string formatFloat(const std::string &format, float num);

std::ostream& operator<<(std::ostream &os, const glm::vec2 &v);
std::ostream& operator<<(std::ostream &os, const glm::vec3 &v);
std::ostream& operator<<(std::ostream &os, const glm::vec4 &v);

inline int reverseBits(int x, int N) {
    int rev = 0;
    for (int i = 0; i < N; i++)
        if (((1 << i) & x) != 0)
            rev |= (1 << (N - i - 1));
    return rev;
}

inline int log2i(int x) {
    int logx = 0;
    for (int i = 31; i >= 0; i--) {
        if ((x & (1 << i)) != 0)
            logx = i;
    }
    return logx;
}

inline void configGlTexture(GLenum wrap, GLenum filter) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}

#endif
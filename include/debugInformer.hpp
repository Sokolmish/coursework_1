#ifndef __DEBUG_INFORMER__
#define __DEBUG_INFORMER__

#include "glew.hpp"
#include "GLFW/glfw3.h"
#include <glm/mat4x4.hpp>
#include "shader.hpp"
#include "font.hpp"

class DebugInformer {
private:
    Shader shader;
    Font *font;

    glm::vec3 pos;
    float yaw, pitch;
    uint fps;

    std::string customMsg;

public:
    DebugInformer();
    ~DebugInformer();

    void show(const glm::mat4 &m_ortho, float width, float height) const;

    void setPos(const glm::vec3 &pos);
    void setPos(float x, float y, float z);
    void setView(float yaw, float pitch);
    void setCustomMsg(const std::string &str);
    void setFPS(uint fps);
};

std::string formatFloat(const std::string &format, float num);
std::ostream& operator<<(std::ostream &os, const glm::vec3 &v);

#endif

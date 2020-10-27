#include "../include/debugInformer.hpp"
#include <sstream>

DebugInformer::DebugInformer() {
    shader = Shader("./shaders/font.vert", "./shaders/font.frag");
    font = new Font("./resources/ConsolaMono-Bold.ttf", 0, 36);
}

DebugInformer::~DebugInformer() {
    delete font;
}

void DebugInformer::show(const glm::mat4 &m_ortho, float width, float height) const {
    shader.use();
    shader.setUniform("projection", m_ortho);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Camera
    std::stringstream builder;
    builder << "pos:" << pos << " ";
    builder << "yaw:" << formatFloat("%.2f", glm::degrees(yaw)) << " ";
    builder << "pitch:" << formatFloat("%.2f", glm::degrees(pitch)) << " ";
    font->RenderText(shader, builder.str(), 10, height - 20, 0.5, glm::vec3(0.f));

    // Custom text
    font->RenderText(shader, customMsg, 10, height - 40, 0.5, glm::vec3(0.f));

    // Perfomance
    builder = std::stringstream();
    builder << "fps: " << fps << " ftime: " << formatFloat("%.3f", 1000.f / fps) << "us ";
    font->RenderText(shader, builder.str(), width - 280, height - 20, 0.5, glm::vec3(0.f));
}


void DebugInformer::setPos(const glm::vec3 &pos) {
    this->pos = pos;
}

void DebugInformer::setPos(float x, float y, float z) {
    this->pos = glm::vec3(x, y, z);
}

void DebugInformer::setView(float yaw, float pitch) {
    this->yaw = yaw;
    this->pitch = pitch;
}

void DebugInformer::setCustomMsg(const std::string &str) {
    this->customMsg = str;
}

void DebugInformer::setFPS(uint fps) {
    this->fps = fps;
}

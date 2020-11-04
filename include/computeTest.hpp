#ifndef __COMPUTE_TEST_H__
#define __COMPUTE_TEST_H__

#include "util/glew.hpp"
#include "util/shader.hpp"
#include "util/image.hpp"
#include "util/camera.hpp"

class compTest {
private:
    Shader shader;
    GLuint ssbo;

public:
    compTest();
    // void show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const;
    // void process(float t);
    void saveImage(const std::string &path);
};  

#endif
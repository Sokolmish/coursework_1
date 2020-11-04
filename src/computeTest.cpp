#include "../include/computeTest.hpp"
#include <iostream>

compTest::compTest() {
    shader = Shader("./shaders/sineSum.comp");

    glGenBuffers(1, &ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 16 * 16 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
}

void compTest::saveImage(const std::string &path) {
    shader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glDispatchCompute(16, 16, 1);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    GLfloat *ptr = (GLfloat*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    for (int i = 0; i < 16 * 16; i++) {
        std::cout << i << " : " << ptr[i] << std::endl;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
}
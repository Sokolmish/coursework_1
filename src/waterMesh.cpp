#include "../include/waterMesh.hpp"
#include <cassert>

WaterMesh::WaterMesh(int w, int h, float size) {
    assert(w > 0 || h > 0 || size > 1e-4f);

    shader = Shader("./shaders/poly.vert", "./shaders/poly.frag");

    this->width = w;
    this->height = h;
    this->size = size;

    float xoff = -size * width / 2.f;
    float zoff = -size * height / 2.f;

    nodes = new std::vector<glm::vec3>(width * height);
    for (int zz = 0; zz < height; zz++)
        for (int xx = 0; xx < width; xx++)
            (*nodes)[zz * width + xx] = glm::vec3(xoff + xx * size, 0.f, zoff + zz * size);
    buff = new GLfloat[width * height * 6];

    GLuint *ebuff = new GLuint[(width - 1) * (height - 1) * 6];
    size_t ind = 0;
    for (int zz = 0; zz < height - 1; zz++) {
        for (int xx = 0; xx < width - 1; xx++) {
            GLuint p1 = zz * width + xx;
            GLuint p2 = zz * width + xx + 1;
            GLuint p3 = (zz + 1) * width + xx;
            GLuint p4 = (zz + 1) * width + xx + 1;
            ebuff[ind++] = p1;
            ebuff[ind++] = p2;
            ebuff[ind++] = p3;
            ebuff[ind++] = p3;
            ebuff[ind++] = p2;
            ebuff[ind++] = p4;
        }
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(GL_ARRAY_BUFFER, w * h * 6 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
    size_t stride = 6 * sizeof(GLfloat);
    glEnableVertexAttribArray(0); // Location
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1); // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (width - 1) * (height - 1) * 6 * sizeof(GLuint), ebuff, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    delete[] ebuff;
}

WaterMesh::~WaterMesh() {
    delete nodes;
    delete[] buff;
}

void WaterMesh::show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const {
    int ind = 0;
    for (int zz = 0; zz < height; zz++) {
        for (int xx = 0; xx < width; xx++) {
            glm::vec3 norm(0.f);
            glm::vec3 p0 = (*nodes)[zz * width + xx];
            glm::vec3 p_negz = (*nodes)[(zz - 1) * width + xx];
            glm::vec3 p_posz = (*nodes)[(zz + 1) * width + xx];
            glm::vec3 p_negx = (*nodes)[zz * width + (xx - 1)];
            glm::vec3 p_posx = (*nodes)[zz * width + (xx + 1)];
            if (zz != 0 && xx != 0)
                norm += glm::normalize(glm::cross(p_negz - p0, p_negx - p0));
            if (zz != height - 1 && xx != 0)
                norm += glm::normalize(glm::cross(p_negx - p0, p_posz - p0));
            if (zz != 0 && xx != width - 1)
                norm += glm::normalize(glm::cross(p_posx - p0, p_negz - p0));
            if (zz != height - 1 && xx != width - 1)
                norm += glm::normalize(glm::cross(p_posz - p0, p_posx - p0));
            norm = glm::normalize(norm);

            for (int i = 0; i < 3; i++)
                buff[ind++] = (*nodes)[zz * width + xx][i];
            for (int i = 0; i < 3; i++)
                buff[ind++] = norm[i];
        }
    }

    shader.use();
    shader.setUniform("m_proj_view", m_proj_view);
    shader.setUniform("is_mesh", isMesh);
    shader.setUniform("eye_pos", cam.pos);
    shader.setUniform("view_dir", cam.getViewDir());

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    if (isMesh)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, width * height * 6 * sizeof(GLfloat), buff);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, (width - 1) * (height - 1) * 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Getters

const std::vector<glm::vec3>& WaterMesh::getNodes() const {
    return *this->nodes;
}

std::vector<glm::vec3>& WaterMesh::getNodes() {
    return *this->nodes;
}

int WaterMesh::getWidth() const {
    return this->width;
}

int WaterMesh::getHeight() const {
    return this->height;
}

float WaterMesh::getSize() const {
    return this->size;
}

#include "../include/waterMeshChunk.hpp"
#include <cassert>
#include <math.h>
#include <iostream>

template<class T>
inline void push_tr(std::vector<T> &dst, T p1, T p2, T p3) {
    dst.push_back(p1);
    dst.push_back(p2);
    dst.push_back(p3);
}

std::vector<std::pair<int, int> > WaterMeshChunk::getElements() const {
    std::vector<std::pair<int, int> > tElements;

    if (meshType & EDGE_NZ) {
        for (int i = 0; i < width - 2; i += 2) {
            push_tr(tElements, { i, 0 }, { i + 2, 0 }, { i + 1, 1 });
            if (i != 0 || !(meshType & EDGE_NX))
                push_tr(tElements, { i, 0 }, { i + 1, 1 }, { i, 1 });
            if (i != width - 3 || !(meshType & EDGE_PX))
                push_tr(tElements, { i + 1, 1 }, { i + 2, 0 }, { i + 2, 1 });
        }
    }

    if (meshType & EDGE_PZ) {
        for (int i = 0; i < width - 2; i += 2) {
            push_tr(tElements, { i, height - 1 }, { i + 1, height - 2 }, { i + 2, height - 1 });
            if (i != 0 || !(meshType & EDGE_NX))
                push_tr(tElements, { i, height - 1 }, { i, height - 2 }, { i + 1, height - 2 });
            if (i != width - 3 || !(meshType & EDGE_PX))
                push_tr(tElements, { i + 1, height - 2 }, { i + 2, height - 2 }, { i + 2, height - 1 });
        }
    }

    if (meshType & EDGE_NX) {
        for (int i = 0; i < height - 2; i += 2) {
            push_tr(tElements, { 0, i }, { 1, i + 1}, { 0, i + 2});
            if (i != 0 || !(meshType & EDGE_NZ))
                push_tr(tElements, { 0, i }, { 1, i }, { 1, i + 1});
            if (i != height - 3 || !(meshType & EDGE_PZ))
                push_tr(tElements, { 1, i + 1 }, { 1, i + 2 }, { 0, i + 2});
        }
    }

    if (meshType & EDGE_PX) {
        for (int i = 0; i < height - 2; i += 2) {
            push_tr(tElements, { width - 1, i }, { width - 1, i + 2}, { width - 2, i + 1});
            if (i != 0 || !(meshType & EDGE_NZ))
                push_tr(tElements, { width - 1, i }, { width - 2, i + 1}, { width - 2, i });
            if (i != height - 3 || !(meshType & EDGE_PZ))
                push_tr(tElements, { width - 2, i + 1 }, { width - 1, i + 2}, { width - 2, i + 2 });
        }
    }

    int startx = (meshType & EDGE_NX) ? 1 : 0;
    int startz = (meshType & EDGE_NZ) ? 1 : 0;
    int stopx = (meshType & EDGE_PX) ? width - 2 : width - 1;
    int stopz = (meshType & EDGE_PZ) ? height - 2 : height - 1;

    for (int zz = startz; zz < stopz; zz++) {
        for (int xx = startx; xx < stopx; xx++) {
            push_tr(tElements, { xx, zz }, { xx + 1, zz }, { xx + 1, zz + 1 });
            push_tr(tElements, { xx, zz }, { xx + 1, zz + 1 }, { xx, zz + 1 });
        }
    }

    return tElements;
}

WaterMeshChunk::WaterMeshChunk(int w, int h, float size, int type, const glm::vec3 &offset) {
    assert(w > 0 || h > 0 || size > 1e-4f);

    if (w % 2 == 0 || h % 2 == 0) {
        std::cerr << "Width and height of the mesh chunk must be odd!" << std::endl;
        w = (w % 2 == 0) ? 1 : 0;
        h = (h % 2 == 0) ? 1 : 0;
    }

    shader = Shader("./shaders/poly.vert", "./shaders/poly.frag");
    normShader = Shader("./shaders/norm.vert", "./shaders/norm.frag", "./shaders/norm.geom");

    this->width = w;
    this->height = h;
    this->size = size;
    this->meshType = type;
    this->offset = offset;

    auto tElements = getElements();
    elementsCount = tElements.size();
    GLuint *ebuff = new GLuint[elementsCount];
    size_t ind = 0;
    for (const auto &e : tElements) {
        int x = e.first;
        int z = e.second;
        ebuff[ind++] = z * width + x;
    }

    nodes = new std::vector<Node>(width * height);
    buff = new GLfloat[width * height * 6];

    for (int zz = 0; zz < height; zz++) {
        for (int xx = 0; xx < width; xx++) {
            (*nodes)[zz * width + xx] = {
                xx, zz,                               // Orig indices
                glm::vec3(xx * size, 0.f, zz * size), // Current pos
                glm::vec3(0.f, 1.f, 0.f)              // Normal
            };
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

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementsCount * sizeof(GLuint), ebuff, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    delete[] ebuff;
}

void WaterMeshChunk::show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const {
    int ind = 0;
    for (int zz = 0; zz < height; zz++) {
        for (int xx = 0; xx < width; xx++) {
            glm::vec3 norm(0.f);
            glm::vec3 p0 = (*nodes)[zz * width + xx].pos;
            glm::vec3 p_negz = (*nodes)[(zz - 1) * width + xx].pos;
            glm::vec3 p_posz = (*nodes)[(zz + 1) * width + xx].pos;
            glm::vec3 p_negx = (*nodes)[zz * width + (xx - 1)].pos;
            glm::vec3 p_posx = (*nodes)[zz * width + (xx + 1)].pos;
            // Here we must add offset to each point
            // But for normals calculation it's doesn't matter
            if (zz != 0 && xx != 0)
                norm += glm::normalize(glm::cross(p_negz - p0, p_negx - p0));
            if (zz != height - 1 && xx != 0)
                norm += glm::normalize(glm::cross(p_negx - p0, p_posz - p0));
            if (zz != 0 && xx != width - 1)
                norm += glm::normalize(glm::cross(p_posx - p0, p_negz - p0));
            if (zz != height - 1 && xx != width - 1)
                norm += glm::normalize(glm::cross(p_posz - p0, p_posx - p0));
            norm = glm::normalize(norm);

            // glm::vec3 p0 = (*nodes)[zz * width + xx].pos;
            // glm::vec3 norm = (*nodes)[zz * width + xx].norm;

            for (int i = 0; i < 3; i++)
                buff[ind++] = p0[i] + offset[i];
            for (int i = 0; i < 3; i++)
                buff[ind++] = norm[i];
        }
    }

    shader.use();
    shader.setUniform("m_proj_view", m_proj_view);
    shader.setUniform("eye_pos", cam.pos);
    shader.setUniform("view_dir", cam.getViewDir());
    shader.setUniform("is_mesh", isMesh);
    if (isMesh)
        shader.setUniform("mesh_color", 0.1, 0.1, 0.1); // 0.03, 0.1, 0.95

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
    glDrawElements(GL_TRIANGLES, elementsCount, GL_UNSIGNED_INT, nullptr);

    // glDisable(GL_DEPTH_TEST);
    // normShader.use();
    // normShader.setUniform("m_proj_view", m_proj_view);
    // glDrawElements(GL_TRIANGLES, elementsCount, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Getters

const std::vector<WaterMeshChunk::Node>& WaterMeshChunk::getNodes() const {
    return *this->nodes;
}

std::vector<WaterMeshChunk::Node>& WaterMeshChunk::getNodes() {
    return *this->nodes;
}

int WaterMeshChunk::getWidth() const {
    return this->width;
}

int WaterMeshChunk::getHeight() const {
    return this->height;
}

float WaterMeshChunk::getSize() const {
    return this->size;
}

int WaterMeshChunk::getMeshType() const {
    return this->meshType;
}

glm::vec3 WaterMeshChunk::getOffset() const {
    return offset;
}

WaterMeshChunk::~WaterMeshChunk() {
    delete nodes;
    delete[] buff;
}

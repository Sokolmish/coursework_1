#include "../include/waterMeshChunk.hpp"
#include "../include/waterMesh.hpp"
#include <cassert>
#include <math.h>
#include <iostream>

static const bool showMeshNormals = false;
static const bool useAnaliticalNormal = false;

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
            push_tr(tElements, { 0, i }, { 1, i + 1}, { 0, i + 2 });
            if (i != 0 || !(meshType & EDGE_NZ))
                push_tr(tElements, { 0, i }, { 1, i }, { 1, i + 1 });
            if (i != height - 3 || !(meshType & EDGE_PZ))
                push_tr(tElements, { 1, i + 1 }, { 1, i + 2 }, { 0, i + 2 });
        }
    }

    if (meshType & EDGE_PX) {
        for (int i = 0; i < height - 2; i += 2) {
            push_tr(tElements, { width - 1, i }, { width - 1, i + 2}, { width - 2, i + 1 });
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

WaterMeshChunk::WaterMeshChunk(int wh, float size, int type, int xs, int ys) {
    assert(wh > 0 && size > 1e-4f);

    offset = glm::vec3(xs * wh * size, 0.f, ys * wh * size);
    wh += (wh % 2 == 0) ? 1 : 0; // Warning?

    posx = xs;
    posz = ys;
    width = wh;
    height = wh;
    this->size = size;
    this->meshType = type;

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
                xx * size + offset.x, zz * size + offset.z,                  // Orig indices
                glm::vec3(xx * size + offset.x, 0.f, zz * size + offset.z),  // Current pos
                glm::vec3(0.f, 1.f, 0.f)                                     // Normal
            };
        }
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(GL_ARRAY_BUFFER, width * height * 6 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
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

    shader = Shader("./shaders/poly.vert", "./shaders/poly.frag");
    if constexpr (showMeshNormals)
        normShader = Shader("./shaders/norm.vert", "./shaders/norm.frag", "./shaders/norm.geom");
}

glm::vec3 WaterMeshChunk::getNormal(int xx, int zz) const {
    glm::vec3 norm(0.f);
    glm::vec3 p0 = (*nodes)[zz * width + xx].pos;
    glm::vec3 p_negz, p_posz, p_negx, p_posx;

    auto neigh_nx = parent->getChunk(posx - 1, posz);
    auto neigh_px = parent->getChunk(posx + 1, posz);
    auto neigh_nz = parent->getChunk(posx, posz - 1);
    auto neigh_pz = parent->getChunk(posx, posz + 1);

    if (xx == 0) {
        if (neigh_nx != nullptr) {
            if (meshType & EDGE_NX)
                p_negx = neigh_nx->getNode(neigh_nx->width - 2, zz / 2).pos;
            else if (neigh_nx->getMeshType() & EDGE_PX)
                p_negx = neigh_nx->getNode(neigh_nx->width - 2, zz * 2).pos;
            else
                p_negx = neigh_nx->getNode(neigh_nx->width - 2, zz).pos;
        }
        else
            p_negx = glm::vec3(0.f, 1.f, 0.f);
    }
    else
        p_negx = (*nodes)[zz * width + (xx - 1)].pos;

    if (xx == width - 1) {
        if (neigh_px != nullptr) {
            if (meshType & EDGE_PX)
                p_posx = neigh_px->getNode(1, zz / 2).pos;
            else if (neigh_px->getMeshType() & EDGE_NX)
                p_posx = neigh_px->getNode(1, zz * 2).pos;
            else
                p_posx = neigh_px->getNode(1, zz).pos;
        }
        else
            p_posx = glm::vec3(0.f, 1.f, 0.f);
    }
    else
        p_posx = (*nodes)[zz * width + (xx + 1)].pos;

    if (zz == 0) {
        if (neigh_nz != nullptr) {
            if (meshType & EDGE_NZ)
                p_negz = neigh_nz->getNode(xx / 2, neigh_nz->height - 2).pos;
            else if (neigh_nz->getMeshType() & EDGE_PZ)
                p_negz = neigh_nz->getNode(xx * 2, neigh_nz->height - 2).pos;
            else
                p_negz = neigh_nz->getNode(xx, neigh_nz->height - 2).pos;
        }
        else
            p_negz = glm::vec3(0.f, 1.f, 0.f);
    }
    else
        p_negz = (*nodes)[(zz - 1) * width + xx].pos;

    if (zz == height - 1) {
        if (neigh_pz != nullptr) {
            if (meshType & EDGE_PZ)
                p_posz = neigh_pz->getNode(xx / 2, 1).pos;
            else if (neigh_pz->getMeshType() & EDGE_NZ)
                p_posz = neigh_pz->getNode(xx * 2, 1).pos;
            else
                p_posz = neigh_pz->getNode(xx, 1).pos;
        }
        else
            p_posz = glm::vec3(0.f, 1.f, 0.f);
    }
    else
        p_posz = (*nodes)[(zz + 1) * width + xx].pos;

    norm += (glm::cross(p_negz - p0, p_negx - p0)); // glm::normalize
    norm += (glm::cross(p_negx - p0, p_posz - p0));
    norm += (glm::cross(p_posz - p0, p_posx - p0));
    norm += (glm::cross(p_posx - p0, p_negz - p0));
    return glm::normalize(norm);
}

void WaterMeshChunk::show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const {
    int ind = 0;
    for (int zz = 0; zz < height; zz++) {
        for (int xx = 0; xx < width; xx++) {
            glm::vec3 p0 = (*nodes)[zz * width + xx].pos;
            glm::vec3 norm;
            if constexpr (!useAnaliticalNormal)
                norm = getNormal(xx, zz);
            else
                norm = (*nodes)[zz * width + xx].norm;

            for (int i = 0; i < 3; i++)
                buff[ind++] = p0[i];
            for (int i = 0; i < 3; i++)
                buff[ind++] = norm[i];
        }
    }

    shader.use();

    shader.setUniform("is_mesh", isMesh);
    if (isMesh)
        shader.setUniform("mesh_color", 0.1, 0.1, 0.1); // 0.03, 0.1, 0.95

    shader.setUniform("m_proj_view", m_proj_view);
    shader.setUniform("eye_pos", cam.pos);
    shader.setUniform("view_dir", cam.getViewDir());

    shader.setUniform("mat.ambient", glm::vec3(0.03f, 0.391f, 0.9993f) * 0.4f);
    shader.setUniform("mat.diffuse", glm::vec3(0.03f, 0.391f, 0.9993f) * 0.35f);
    shader.setUniform("mat.specular", glm::vec3(0.2f, 0.2f, 0.2f));
    shader.setUniform("mat.exponent", 70.f);

    shader.setUniform("globalAmb", glm::vec3(0.35f, 0.35f, 0.45f));
    shader.setUniform("sunDir", glm::vec3(0.5f, 0.5f, 0.0f));

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

    if constexpr (showMeshNormals) {
        glDisable(GL_DEPTH_TEST);
        normShader.use();
        normShader.setUniform("m_proj_view", m_proj_view);
        glDrawElements(GL_TRIANGLES, elementsCount, GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Getters

WaterMeshChunk::Node WaterMeshChunk::getNode(int xx, int zz) const {
    return (*nodes)[zz * width + xx];
}

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

std::pair<int, int> WaterMeshChunk::getChunkPos() const {
    return std::pair<int, int>(posx, posz);
}

WaterMeshChunk::~WaterMeshChunk() {
    delete nodes;
    delete[] buff;
}
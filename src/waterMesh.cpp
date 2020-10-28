#include "../include/waterMesh.hpp"

WaterMesh::WaterMesh() {
    chunks = std::vector<WaterMeshChunk*>();

    int chDis = 61;
    float chBaseSize = 3.0f;
    float chSize = (chDis - 1) * chBaseSize;

    chunks.push_back(
        new WaterMeshChunk(chDis, chDis, chBaseSize, WaterMeshChunk::INNER, { 0, 0, 0 }));

    chunks.push_back(
        new WaterMeshChunk(chDis, chDis, chBaseSize, WaterMeshChunk::EDGE_PX, { 1 * chSize, 0, 0 }));
    chunks.push_back(
        new WaterMeshChunk(chDis, chDis, chBaseSize, WaterMeshChunk::EDGE_NX, { -1 * chSize, 0, 0 }));
    chunks.push_back(
        new WaterMeshChunk(chDis, chDis, chBaseSize, WaterMeshChunk::EDGE_PZ, { 0, 0, 1 * chSize }));
    chunks.push_back(
        new WaterMeshChunk(chDis, chDis, chBaseSize, WaterMeshChunk::EDGE_NZ, { 0, 0, -1 * chSize }));

    chunks.push_back(
        new WaterMeshChunk(chDis, chDis, chBaseSize, WaterMeshChunk::EDGE_PX | WaterMeshChunk::EDGE_PZ,
            { 1 * chSize, 0, 1 * chSize }));
    chunks.push_back(
        new WaterMeshChunk(chDis, chDis, chBaseSize, WaterMeshChunk::EDGE_PX | WaterMeshChunk::EDGE_NZ,
            { 1 * chSize, 0, -1 * chSize }));
    chunks.push_back(
        new WaterMeshChunk(chDis, chDis, chBaseSize, WaterMeshChunk::EDGE_NX | WaterMeshChunk::EDGE_PZ,
            { -1 * chSize, 0, 1 * chSize }));
    chunks.push_back(
        new WaterMeshChunk(chDis, chDis, chBaseSize, WaterMeshChunk::EDGE_NX | WaterMeshChunk::EDGE_NZ,
            { -1 * chSize, 0, -1 * chSize }));


    for (int i = -1; i <= 1; i++) {
        chunks.push_back(new WaterMeshChunk(31, 31, chBaseSize * 2, WaterMeshChunk::INNER, { 2 * chSize, 0.f, i * chSize }));
        chunks.push_back(new WaterMeshChunk(31, 31, chBaseSize * 2, WaterMeshChunk::INNER, { -2 * chSize, 0.f, i * chSize }));
        chunks.push_back(new WaterMeshChunk(31, 31, chBaseSize * 2, WaterMeshChunk::INNER, { i * chSize, 0.f, 2 * chSize }));
        chunks.push_back(new WaterMeshChunk(31, 31, chBaseSize * 2, WaterMeshChunk::INNER, { i * chSize, 0.f, -2 * chSize }));
    }
}

void WaterMesh::show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const {
    for (auto e : chunks)
        e->show(m_proj_view, isMesh, cam);
}

void WaterMesh::process(AbstractPhysics *phys, float t) {
    for (auto &e : chunks)
        phys->process(*e, t);
}

WaterMesh::~WaterMesh() {
    for (auto e : chunks)
        delete e;
}
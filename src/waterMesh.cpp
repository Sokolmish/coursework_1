#include "../include/waterMesh.hpp"

inline void ins_mc(std::map<std::pair<int, int>, WaterMeshChunk*> &dst, WaterMeshChunk *val) {
    dst.insert({val->getChunkPos(), val});
}

WaterMesh::WaterMesh() {
    chunks = std::map<std::pair<int, int>, WaterMeshChunk*>();

    int chDis = 64;
    float chBaseSize = 3.5f;

    ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::INNER, 0, 0));

    ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::EDGE_PX, 1, 0));
    ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::EDGE_NX, -1, 0));
    ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::EDGE_PZ, 0, 1));
    ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::EDGE_NZ, 0, -1));

    ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::CORN_PXPZ, 1, 1));
    ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::CORN_PXNZ, 1, -1));
    ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::CORN_NXPZ, -1, 1));
    ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::CORN_NXNZ, -1, -1));

    chDis /= 2;
    chBaseSize *= 2;
    for (int i = -2; i <= 2; i++) {
        ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::INNER, 2, i));
        ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::INNER, -2, i));
    }
    for (int i = -1; i <= 1; i++) {
        ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::INNER, i, 2));
        ins_mc(chunks, new WaterMeshChunk(chDis, chBaseSize, WaterMeshChunk::INNER, i, -2));
    }

    for (auto &e : chunks)
        e.second->parent = this;
}

void WaterMesh::show(const glm::mat4 &m_proj_view, bool isMesh, const Camera &cam) const {
    for (auto e : chunks)
        e.second->show(m_proj_view, isMesh, cam);
}

void WaterMesh::process(AbstractPhysics *phys, float t) {
    for (auto e : chunks)
        phys->process(*e.second, t);
}

WaterMesh::~WaterMesh() {
    for (auto e : chunks)
        delete e.second;
}

const WaterMeshChunk* WaterMesh::getChunk(int x, int z) const {
    auto it = chunks.find(std::pair<int, int>(x, z));
    if (it == chunks.end())
        return nullptr;
    else
        return it->second;
}
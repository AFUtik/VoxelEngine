#ifndef CUBEMESHER_HPP
#define CUBEMESHER_HPP

#include <glm/glm.hpp>

#include "Drawable.hpp"
#include "../../logic/blocks/Block.hpp"
#include "../../logic/blocks/ChunkInfo.hpp"
#include "../../logic/blocks/Chunk.hpp"
#include "../../logic/LogicSystem.hpp"
#include "../model/Mesh.hpp"

#include "GLController.hpp"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

class Mesher;

using namespace glm;
using namespace std;

struct VoxelFace {
    FaceDirection faceDirection;
    int block;
    int minX, maxX;
    int minY, maxY;
    int minZ, maxZ;

    struct Light {
        float r[4];
        float g[4];
        float b[4];
        float s[4];
    } light;
    
    VoxelFace(FaceDirection fd, int block, int x, int y, int z) : 
        faceDirection(fd), 
        block(block), 
        minX(x), maxX(x), minY(y), 
        maxY(y), minZ(z), maxZ(z) {}

    VoxelFace(const VoxelFace& a, const VoxelFace& b)
    : faceDirection(a.faceDirection),
      block(a.block),
      minX(std::min(a.minX, b.minX)),
      minY(std::min(a.minY, b.minY)),
      minZ(std::min(a.minZ, b.minZ)),
      maxX(std::max(a.maxX, b.maxX)),
      maxY(std::max(a.maxY, b.maxY)),
      maxZ(std::max(a.maxZ, b.maxZ)), light(a.light) {}

    inline static bool checkCanCombineLight(VoxelFace &faceA, VoxelFace &faceB) {
        if(glm::epsilonEqual(faceA.light.s[0]+faceA.light.s[1]+faceA.light.s[2]+faceA.light.s[3], faceB.light.s[0]+faceB.light.s[1]+faceB.light.s[2]+faceB.light.s[3], 1e-5f)) return false;
        
        return true;
    }

    template<FaceDirection FD> static bool canCombineZWise(VoxelFace &faceA, VoxelFace &faceB) {
        if(faceA.block != faceB.block || faceA.faceDirection != faceB.faceDirection || checkCanCombineLight(faceA, faceB)) return false;

        int faceAXLen = faceA.maxX - faceA.minX;
        int faceAYLen = faceA.maxY - faceA.minY;
        int faceAZLen = faceA.maxZ - faceA.minZ;

        int faceBXLen = faceB.maxX - faceB.minX;
        int faceBYLen = faceB.maxY - faceB.minY;
        int faceBZLen = faceB.maxZ - faceB.minZ;

        if(faceA.minZ == faceB.minZ) {
            if(faceA.minY == faceB.minY) 
            {
                if(faceAZLen!=faceBZLen || faceAYLen!=faceBYLen) return false;
                if(abs(faceA.minX + faceAXLen - faceB.minX) > 1 && abs(faceB.minX + faceBXLen - faceA.minX) > 1) return false;
                return true;
            }
            if(faceA.minX == faceB.minX)
            {
                if(faceAXLen!=faceBXLen || faceAZLen!=faceBZLen) return false;
                if(abs(faceA.minY + faceAYLen - faceB.minY) > 1 && abs(faceB.minY + faceBYLen - faceA.minY) > 1) return false;
                return true;
            }
        }
        return false;
    }
};



struct BlockModelCube { 
    Mesh* mesh;

    void addFaceNXPlane(float x1, float y1, float z1, float x2, float y2, float z2, float u1, float v1, float u2, float v2, const VoxelFace::Light &l);
    void addFaceXPlane (float x1, float y1, float z1, float x2, float y2, float z2, float u1, float v1, float u2, float v2, const VoxelFace::Light &l);
    void addFaceNYPlane(float x1, float y1, float z1, float x2, float y2, float z2, float u1, float v1, float u2, float v2, const VoxelFace::Light &l);
    void addFaceYPlane (float x1, float y1, float z1, float x2, float y2, float z2, float u1, float v1, float u2, float v2, const VoxelFace::Light &l);
    void addFaceNZPlane(float x1, float y1, float z1, float x2, float y2, float z2, float u1, float v1, float u2, float v2, const VoxelFace::Light &l);
    void addFaceZPlane (float x1, float y1, float z1, float x2, float y2, float z2, float u1, float v1, float u2, float v2, const VoxelFace::Light &l);

    BlockModelCube(Mesh* mesh) : mesh(mesh) {}
};

class ChunkMesher {
    vector<VoxelFace> y_faces [6];
    unordered_map<size_t, vector<VoxelFace>> zx_faces[6];

    BlockModelCube cubeModel;
    ChunkSnapshot* chunk;

    template<size_t Corner>
    inline void mix4_light(
        VoxelFace &face,
        float cr, float cg, float cb, float cs,
        int x0, int y0, int z0,
        int x1, int y1, int z1,
        int x2, int y2, int z2
        ) {
        face.light.r[Corner] = (chunk->getBoundLight(x0, y0, z0, 0) + cr * 30 + chunk->getBoundLight(x1, y1, z1, 0) + chunk->getBoundLight(x2, y2, z2, 0)) / 5.0f / 15.0f;
        face.light.g[Corner] = (chunk->getBoundLight(x0, y0, z0, 1) + cg * 30 + chunk->getBoundLight(x1, y1, z1, 1) + chunk->getBoundLight(x2, y2, z2, 1)) / 5.0f / 15.0f;
        face.light.b[Corner] = (chunk->getBoundLight(x0, y0, z0, 2) + cb * 30 + chunk->getBoundLight(x1, y1, z1, 2) + chunk->getBoundLight(x2, y2, z2, 2)) / 5.0f / 15.0f;
        face.light.s[Corner] = (chunk->getBoundLight(x0, y0, z0, 3) + cs * 30 + chunk->getBoundLight(x1, y1, z1, 3) + chunk->getBoundLight(x2, y2, z2, 3)) / 5.0f / 15.0f;
    }

    template<FaceDirection FD> inline void addFace(int block, int x, int y, int z) 
    {
        constexpr int dx = FaceDirection::POS_X  == FD ? 1 : FaceDirection::NEG_X  == FD ? -1 : 0;
        constexpr int dy = FaceDirection::POS_Y  == FD ? 1 : FaceDirection::NEG_Y  == FD ? -1 : 0;
        constexpr int dz = FaceDirection::POS_Z  == FD ? 1 : FaceDirection::NEG_Z  == FD ? -1 : 0;
        float lr = chunk->getBoundLight(x+dx, y+dy, z+dz, 0) / 15.0f;
        float lg = chunk->getBoundLight(x+dx, y+dy, z+dz, 1) / 15.0f;
        float lb = chunk->getBoundLight(x+dx, y+dy, z+dz, 2) / 15.0f;
        float ls = chunk->getBoundLight(x+dx, y+dy, z+dz, 3) / 15.0f;
        VoxelFace curFace(FD, block, x, y, z);
        if constexpr (FD == FaceDirection::POS_Y) {
            mix4_light<0>(curFace, lr, lg, lb, ls, x-1,y+1,z, x-1,y+1,z-1, x,y+1,z-1);
            mix4_light<1>(curFace, lr, lg, lb, ls, x-1,y+1,z, x-1,y+1,z+1, x,y+1,z+1);
            mix4_light<2>(curFace, lr, lg, lb, ls, x+1,y+1,z, x+1,y+1,z+1, x,y+1,z+1);
            mix4_light<3>(curFace, lr, lg, lb, ls, x+1,y+1,z, x+1,y+1,z-1, x,y+1,z-1);
        } else if constexpr (FD == FaceDirection::NEG_Y) {
            mix4_light<0>(curFace, lr, lg, lb, ls, x-1,y-1,z, x-1,y-1,z-1, x,y-1,z-1);
            mix4_light<1>(curFace, lr, lg, lb, ls, x-1,y-1,z, x-1,y-1,z+1, x,y-1,z+1);
            mix4_light<2>(curFace, lr, lg, lb, ls, x+1,y-1,z, x+1,y-1,z+1, x,y-1,z+1);
            mix4_light<3>(curFace, lr, lg, lb, ls, x+1,y-1,z, x+1,y-1,z-1, x,y-1,z-1);
        } else if constexpr (FD == FaceDirection::POS_X) {
            mix4_light<0>(curFace, lr, lg, lb, ls, x+1,y-1,z-1, x+1,y,z-1, x+1,y-1,z);
            mix4_light<1>(curFace, lr, lg, lb, ls, x+1,y+1,z-1, x+1,y,z-1, x+1,y+1,z);
            mix4_light<2>(curFace, lr, lg, lb, ls, x+1,y+1,z+1, x+1,y,z+1, x+1,y+1,z);
            mix4_light<3>(curFace, lr, lg, lb, ls, x+1,y-1,z+1, x+1,y,z+1, x+1,y-1,z);
        } else if constexpr (FD == FaceDirection::NEG_X) {
            mix4_light<0>(curFace, lr, lg, lb, ls, x-1,y-1,z-1, x-1,y,z-1, x-1,y-1,z);
            mix4_light<1>(curFace, lr, lg, lb, ls, x-1,y+1,z-1, x-1,y,z-1, x-1,y+1,z);
            mix4_light<2>(curFace, lr, lg, lb, ls, x-1,y+1,z+1, x-1,y,z+1, x-1,y+1,z);
            mix4_light<3>(curFace, lr, lg, lb, ls, x-1,y-1,z+1, x-1,y,z+1, x-1,y-1,z);
        } else if constexpr (FD == FaceDirection::POS_Z) {
            mix4_light<0>(curFace, lr, lg, lb, ls, x-1,y-1,z+1, x,y-1,z+1, x-1,y,z+1);
            mix4_light<1>(curFace, lr, lg, lb, ls, x-1,y+1,z+1, x,y+1,z+1, x-1,y,z+1);
            mix4_light<2>(curFace, lr, lg, lb, ls, x+1,y+1,z+1, x,y+1,z+1, x+1,y,z+1);
            mix4_light<3>(curFace, lr, lg, lb, ls, x+1,y-1,z+1, x,y-1,z+1, x+1,y,z+1);
        } else if constexpr (FD == FaceDirection::NEG_Z) {
            mix4_light<0>(curFace, lr, lg, lb, ls, x-1,y-1,z-1, x,y-1,z-1, x-1,y,z-1);
            mix4_light<1>(curFace, lr, lg, lb, ls, x-1,y+1,z-1, x,y+1,z-1, x-1,y,z-1);
            mix4_light<2>(curFace, lr, lg, lb, ls, x+1,y+1,z-1, x,y+1,z-1, x+1,y,z-1);
            mix4_light<3>(curFace, lr, lg, lb, ls, x+1,y-1,z-1, x,y-1,z-1, x+1,y,z-1);
        }
        
        if constexpr (FD==FaceDirection::NEG_Y || FD==FaceDirection::POS_Y) {
            vector<VoxelFace> &faces = y_faces[static_cast<size_t>(FD)];
            bool added = false;
            if (!faces.empty()) {
                VoxelFace& last = faces.back();
                if (VoxelFace::canCombineZWise<FD>(last, curFace)) {
                    faces.back() = VoxelFace(last, curFace);
                    added = true;
                }
            }
            if (!added) faces.push_back(curFace);
        } else {
            unordered_map<size_t, vector<VoxelFace>> &mfaces = zx_faces[static_cast<size_t>(FD)];
            size_t key = (static_cast<size_t>(z) << 32) | x;
            vector<VoxelFace>& faces = mfaces[key];
            bool added = false;
            if(!faces.empty()) {
                VoxelFace& last = faces.back();
                if (VoxelFace::canCombineZWise<FD>(last, curFace)) {
                    faces.back() = VoxelFace(last, curFace);
                    added = true;
                }
            } 
            if(!added) faces.push_back(curFace);
        }
        
    }

    void addCube(int block, int lx, int ly, int lz);
public:
    void make();

    ChunkMesher(ChunkSnapshot* chunk, Mesh* mesh) : chunk(chunk), cubeModel(mesh) {
        for(int i = 0; i < 2; i++) y_faces[i].reserve(512);
    }
};

class Mesher {
    std::unique_ptr<GlController> glController;
    LogicSystem* world;

    std::mutex meshUploadMutex;
    std::queue<std::pair<ChunkPtr, std::shared_ptr<Mesh>>> meshUploadQueue;
    std::thread worker;
    bool stop = false;

    friend class BlockRenderer; 
public:
    Mesher(LogicSystem* world) : glController(new GlController), world(world) {
        worker = std::thread([this, world] { meshWorkerThread(); });
    }

    ~Mesher() {
        {
            std::lock_guard<std::mutex> lk(world->readyQueueMutex);
            stop = true;
        }
        world->readyCv.notify_all();
        if (worker.joinable())
            worker.join();
    }

    void meshWorkerThread();
};

#endif //CUBEMESHER_HPP

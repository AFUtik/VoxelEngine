
#include "ChunkMesher.hpp"
#include <memory>
#include <mutex>

// BLOCK MESHER
void BlockModelCube::addFaceXPlane(float x1, float y1, float z1, float x2, float y2, float z2,
                                   float u1, float v1, float u2, float v2, const VoxelFace::Light &l)
{
    unsigned int startIndex = mesh->buffer.size();

    mesh->buffer.push_back(Vertex{x1, y1, z1, u1, v1, l.r[0], l.g[0], l.b[0], l.s[0]});
    mesh->buffer.push_back(Vertex{x1, y2, z1, u1, v2, l.r[1], l.g[1], l.b[1], l.s[1]});
    mesh->buffer.push_back(Vertex{x1, y2, z2, u2, v2, l.r[2], l.g[2], l.b[2], l.s[2]});
    mesh->buffer.push_back(Vertex{x1, y1, z2, u2, v1, l.r[3], l.g[3], l.b[3], l.s[3]});

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 1);
    mesh->indices.push_back(startIndex + 2);

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 2);
    mesh->indices.push_back(startIndex + 3);
}

void BlockModelCube::addFaceNXPlane(float x1, float y1, float z1, float x2, float y2, float z2,
                                    float u1, float v1, float u2, float v2, const VoxelFace::Light &l)
{
    unsigned int startIndex = mesh->buffer.size();

    mesh->buffer.push_back(Vertex{x1, y1, z1, u1, v1, l.r[0], l.g[0], l.b[0], l.s[0]});
    mesh->buffer.push_back(Vertex{x1, y2, z1, u1, v2, l.r[1], l.g[1], l.b[1], l.s[1]});
    mesh->buffer.push_back(Vertex{x1, y2, z2, u2, v2, l.r[2], l.g[2], l.b[2], l.s[2]});
    mesh->buffer.push_back(Vertex{x1, y1, z2, u2, v1, l.r[3], l.g[3], l.b[3], l.s[3]});

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 2);
    mesh->indices.push_back(startIndex + 1);

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 3);
    mesh->indices.push_back(startIndex + 2);
}

void BlockModelCube::addFaceYPlane(float x1, float y1, float z1, float x2, float y2, float z2,
                                   float u1, float v1, float u2, float v2, const VoxelFace::Light &l)
{
    unsigned int startIndex = mesh->buffer.size();

    mesh->buffer.push_back(Vertex{x1, y2, z1, u1, v2, l.r[0], l.g[0], l.b[0], l.s[0]});
    mesh->buffer.push_back(Vertex{x1, y2, z2, u1, v1, l.r[1], l.g[1], l.b[1], l.s[1]});
    mesh->buffer.push_back(Vertex{x2, y2, z2, u2, v1, l.r[2], l.g[2], l.b[2], l.s[2]});
    mesh->buffer.push_back(Vertex{x2, y2, z1, u2, v2, l.r[3], l.g[3], l.b[3], l.s[3]});

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 1);
    mesh->indices.push_back(startIndex + 2);

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 2);
    mesh->indices.push_back(startIndex + 3);
}

void BlockModelCube::addFaceNYPlane(float x1, float y1, float z1, float x2, float y2, float z2,
                                    float u1, float v1, float u2, float v2, const VoxelFace::Light &l)
{
    unsigned int startIndex = mesh->buffer.size();

    mesh->buffer.push_back(Vertex{x1, y1, z1, u1, v1, l.r[0], l.g[0], l.b[0], l.s[0]});
    mesh->buffer.push_back(Vertex{x2, y1, z1, u2, v1, l.r[3], l.g[3], l.b[3], l.s[3]});
    mesh->buffer.push_back(Vertex{x2, y1, z2, u2, v2, l.r[2], l.g[2], l.b[2], l.s[2]});
    mesh->buffer.push_back(Vertex{x1, y1, z2, u1, v2, l.r[1], l.g[1], l.b[1], l.s[1]});

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 1);
    mesh->indices.push_back(startIndex + 2);

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 2);
    mesh->indices.push_back(startIndex + 3);
}

void BlockModelCube::addFaceZPlane(float x1, float y1, float z1, float x2, float y2, float z2,
                                   float u1, float v1, float u2, float v2, const VoxelFace::Light &l)
{
    unsigned int startIndex = mesh->buffer.size();

    mesh->buffer.push_back(Vertex{x1, y1, z2, u1, v1, l.r[0], l.g[0], l.b[0], l.s[0]});
    mesh->buffer.push_back(Vertex{x2, y1, z2, u2, v1, l.r[3], l.g[3], l.b[3], l.s[3]});
    mesh->buffer.push_back(Vertex{x2, y2, z2, u2, v2, l.r[2], l.g[2], l.b[2], l.s[2]});
    mesh->buffer.push_back(Vertex{x1, y2, z2, u1, v2, l.r[1], l.g[1], l.b[1], l.s[1]});

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 1);
    mesh->indices.push_back(startIndex + 2);

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 2);
    mesh->indices.push_back(startIndex + 3);
}

void BlockModelCube::addFaceNZPlane(float x1, float y1, float z1, float x2, float y2, float z2,
                                    float u1, float v1, float u2, float v2, const VoxelFace::Light &l)
{
    unsigned int startIndex = mesh->buffer.size();

    mesh->buffer.push_back(Vertex{x1, y1, z1, u1, v1, l.r[0], l.g[0], l.b[0], l.s[0]});
    mesh->buffer.push_back(Vertex{x2, y1, z1, u2, v1, l.r[3], l.g[3], l.b[3], l.s[3]});
    mesh->buffer.push_back(Vertex{x2, y2, z1, u2, v2, l.r[2], l.g[2], l.b[2], l.s[2]});
    mesh->buffer.push_back(Vertex{x1, y2, z1, u1, v2, l.r[1], l.g[1], l.b[1], l.s[1]});

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 2);
    mesh->indices.push_back(startIndex + 1);

    mesh->indices.push_back(startIndex);
    mesh->indices.push_back(startIndex + 3);
    mesh->indices.push_back(startIndex + 2);
}

// CHUNK MESHER
void ChunkMesher::addCube(int block, int lx, int ly, int lz) {
    if (!chunk->getBoundBlock(lx, ly+1, lz).isOpaque()) addFace<FaceDirection::POS_Y>(block, lx, ly, lz);
    if (!chunk->getBoundBlock(lx, ly-1, lz).isOpaque()) addFace<FaceDirection::NEG_Y>(block, lx, ly, lz);
    if (!chunk->getBoundBlock(lx+1, ly, lz).isOpaque()) addFace<FaceDirection::POS_X>(block, lx, ly, lz);
    if (!chunk->getBoundBlock(lx-1, ly, lz).isOpaque()) addFace<FaceDirection::NEG_X>(block, lx, ly, lz);
    if (!chunk->getBoundBlock(lx, ly, lz+1).isOpaque()) addFace<FaceDirection::POS_Z>(block, lx, ly, lz);
    if (!chunk->getBoundBlock(lx, ly, lz-1).isOpaque()) addFace<FaceDirection::NEG_Z>(block, lx, ly, lz);
}

void ChunkMesher::make() {
    for (int y = 0; y < ChunkInfo::HEIGHT; y++) {
        for (int z = 0; z < ChunkInfo::DEPTH; z++) {
            for (int x = 0; x < ChunkInfo::WIDTH; x++) {
                block vox = chunk->getBlock(x, y, z);
                unsigned int id = vox.id;

                if (!id) continue;
                addCube(vox.id, x, y, z);
            }
        }
    }
    const unordered_map<size_t, vector<VoxelFace>>& posx_map = zx_faces[static_cast<size_t>(FaceDirection::POS_X)];
    const unordered_map<size_t, vector<VoxelFace>>& negx_map = zx_faces[static_cast<size_t>(FaceDirection::NEG_X)];
    const unordered_map<size_t, vector<VoxelFace>>& posz_map = zx_faces[static_cast<size_t>(FaceDirection::POS_Z)];
    const unordered_map<size_t, vector<VoxelFace>>& negz_map = zx_faces[static_cast<size_t>(FaceDirection::NEG_Z)];

    

    constexpr float uvsize = 32.0f / 512.0f;
    float u = 0.0625f;
    float v = 0.9375f - uvsize;
    for(const auto& p : posx_map) 
    for(const auto& f : p.second) 
    {
        float y1 = f.minY;
        float z1 = f.minZ;
        float x2 = f.maxX+1;
        float y2 = f.maxY+1;
        float z2 = f.maxZ+1;

        float u1 = u + (f.maxZ - f.minZ);
        float v1 = v + (f.maxY - f.minY);
        float u2 = u1 + uvsize;
        float v2 = v1 + uvsize;
        cubeModel.addFaceXPlane(x2, y1, z1, x2, y2, z2, u1, v1, u2, v2, f.light);
    }
    for(const auto& p : negx_map) 
    for(const auto& f : p.second)  
    {
        float y1 = f.minY;
        float z1 = f.minZ;
        float x1 = f.minX;
        float y2 = f.maxY+1;
        float z2 = f.maxZ+1;

        float u1 = u + (f.maxZ - f.minZ);
        float v1 = v + (f.maxY - f.minY);
        float u2 = u1 + uvsize;
        float v2 = v1 + uvsize;
        cubeModel.addFaceNXPlane(x1, y1, z1, x1, y2, z2, u1, v1, u2, v2, f.light);
    }
    for(const VoxelFace& f : y_faces[static_cast<size_t>(FaceDirection::POS_Y)]) {
        float x1 = f.minX;
        float z1 = f.minZ;
        float y2 = f.maxY+1;
        float z2 = f.maxZ+1;
        float x2 = f.maxX+1;

        float u1 = u + (f.maxX - f.minX);
        float v1 = v + (f.maxZ - f.minZ);
        float u2 = u1 + uvsize;
        float v2 = v1 + uvsize;
        cubeModel.addFaceYPlane(x1, y2, z1, x2, y2, z2, u1, v1, u2, v2, f.light);
    }
    for(const VoxelFace& f : y_faces[static_cast<size_t>(FaceDirection::NEG_Y)]) {
        float x1 = f.minX;
        float z1 = f.minZ;
        float y1 = f.minY;
        float z2 = f.maxZ+1;
        float x2 = f.maxX+1;

        float u1 = u + (f.maxX - f.minX);
        float v1 = v + (f.maxZ - f.minZ);
        float u2 = u1 + uvsize;
        float v2 = v1 + uvsize;
        cubeModel.addFaceNYPlane(x1, y1, z1, x2, y1, z2, u1, v1, u2, v2, f.light);
    }
    for(const auto& p : posz_map) 
    for(const auto& f : p.second) {
        float x1 = f.minX;
        float y1 = f.minY;
        float z2 = f.maxZ+1;
        float x2 = f.maxX+1;
        float y2 = f.maxY+1;

        float u1 = u + (f.maxX - f.minX);
        float v1 = v + (f.maxY - f.minY);
        float u2 = u1 + uvsize;
        float v2 = v1 + uvsize;
        cubeModel.addFaceZPlane(x1, y1, z2, x2, y2, z2, u1, v1, u2, v2, f.light);
    }
    for(const auto& p : negz_map) 
    for(const auto& f : p.second) {
        float x1 = f.minX;
        float y1 = f.minY;
        float z1 = f.minZ;
        float x2 = f.maxX+1;
        float y2 = f.maxY+1;

        float u1 = u + (f.maxX - f.minX);
        float v1 = v + (f.maxY - f.minY);
        float u2 = u1 + uvsize;
        float v2 = v1 + uvsize;
        cubeModel.addFaceNZPlane(x1, y1, z1, x2, y2, z1, u1, v1, u2, v2, f.light);
    }
}

void Mesher::meshWorkerThread() {
    while (true) {
            std::unique_ptr<ChunkSnapshot> snap;
            {
                std::unique_lock lk(world->readyQueueMutex);
                world->readyCv.wait(lk, [&] {
                    return stop || !world->readyChunks.empty();
                });
                if (stop && world->readyChunks.empty())
                    break;

                snap = std::move(world->readyChunks.front());
                world->readyChunks.pop_front();
            }
            std::shared_ptr<Chunk> org = snap->source;

            if(!org) continue;
            if(org->checkState(ChunkState::Finished)) {
                auto mesh = make_shared<Mesh>(glController.get());
                ChunkMesher chunkMesher(snap.get(), mesh.get());
                chunkMesher.make(); 
                {
                    std::unique_lock<std::mutex> wlock(meshUploadMutex);
                    meshUploadQueue.push({org, mesh});
                }
            }
        }
    }
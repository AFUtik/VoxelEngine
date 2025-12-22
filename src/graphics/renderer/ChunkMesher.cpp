
#include "ChunkMesher.hpp"
#include <memory>
#include <mutex>

void ChunkMesher::meshWorkerThread() {
    while (true) {
            std::shared_ptr<Chunk> sp;
            {
                std::unique_lock lk(world->readyQueueMutex);
                world->readyCv.wait(lk, [&] {
                    return stop || !world->readyChunks.empty();
                });
                if (stop && world->readyChunks.empty())
                    break;

                sp = world->readyChunks.front();
                world->readyChunks.pop();
            }
            if(!sp) continue;

            DrawableObject& draw = sp->chunk_draw;

            if(sp->checkState(ChunkState::Finished)) {
                if(draw.getMesh()) {
                    updateChunk(sp.get());
                } else {
                    std::shared_ptr<Mesh> mesh;
                    {
                        std::shared_lock<std::shared_mutex> wl(sp->dataMutex);
                        mesh = makeChunk(sp.get());
                    }
                    
                    if(mesh==nullptr) continue;

                    {
                        std::unique_lock<std::shared_mutex> wlock(sp->dataMutex);
                        draw.loadMesh(mesh);
                    }

                    {
                        std::unique_lock<std::mutex> wlock(meshUploadMutex);
                        meshUploadQueue.push(sp);
                    }
                }
            }
            meshUploadCv.notify_one();
        }
    }
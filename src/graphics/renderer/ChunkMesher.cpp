#include "ChunkMesher.hpp"
#include <memory>

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

            if(sp->isModified()) {
                std::shared_ptr<Mesh> mesh;
                {
                    std::shared_lock<std::shared_mutex> wlock(sp->dataMutex);
                    mesh = makeChunk(sp.get());
                }
                {
                    std::unique_lock<std::shared_mutex> wlock(sp->dataMutex);
                    sp->chunk_draw.loadMesh(mesh);
                }            
                {
                    std::unique_lock<std::mutex> wlock(meshUploadMutex);
                    meshUploadQueue.push(sp);
                }
                sp->unmodify();
            } else if(sp->isLightModified()) {
                {
                    std::shared_lock<std::shared_mutex> wlock(sp->dataMutex);
                    updateChunk(sp.get());
                } 
                sp->unmodifyLight();
            }
            meshUploadCv.notify_one();
        }
    }
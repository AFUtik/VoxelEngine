#include "ChunkMesher.hpp"

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
                Mesh* mesh = nullptr;
                {
                    std::shared_lock<std::shared_mutex> wlock(sp->dataMutex);
                    mesh = makeChunk(sp.get());
                }
                {
                    std::lock_guard lk(meshUploadMutex);
                    meshUploadQueue.push({mesh, sp});
                }
                meshUploadCv.notify_one();   
        }
    }
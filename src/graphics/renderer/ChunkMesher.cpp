
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

            if(draw.isModified()) {
                std::shared_ptr<Mesh> mesh;
                {
                    std::shared_lock<std::shared_mutex> wlock(sp->dataMutex);
                    mesh = makeChunk(sp.get());
                }
                
                {
                    std::unique_lock<std::shared_mutex> wlock(sp->dataMutex);
                    draw.loadMesh(mesh);
                }

                {
                    std::unique_lock<std::mutex> wlock(meshUploadMutex);
                    meshUploadQueue.push(sp);
                }
                draw.unmodify();
            } 
            //else if(draw.isUpdated()) {
            //    {
            //        std::shared_lock<std::shared_mutex> wlock(sp->dataMutex);
            //        updateChunk(sp.get());
            //    }
            //    draw.unupdate();
            //}
            meshUploadCv.notify_one();
        }
    }
#ifndef CHUNKIO_HPP
#define CHUNKIO_HPP

class Chunk;

class ChunkIO {
public:
    void saveChunk(Chunk* chunk);
    Chunk* loadChunk(int x, int y, int z);
};

#endif
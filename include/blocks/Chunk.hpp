#ifndef CHUNK_HPP
#define CHUNK_HPP

#define CHUNK_W 256
#define CHUNK_H 256
#define CHUNK_D 256
#define CHUNK_VOL (CHUNK_W * CHUNK_H * CHUNK_D)

struct block;

class Chunk {
public:
	block* blocks;
	Chunk();
	~Chunk();
};

#endif // !CHUNK_HPP

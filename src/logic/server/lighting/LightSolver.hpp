#ifndef LIGHTSOLVER_HPP
#define LIGHTSOLVER_HPP

#include <memory>
#include <cstdint>

#include "LightInfo.hpp"
#include "../blocks/Chunk.hpp"
#include "../blocks/ChunkInfo.hpp"

enum class Channels {
	RED, GREEN, BLUE, LIGHT
};

class BasicLightSolver {
	void processBoundaryBlock(
		Chunk* A, Chunk* B,
		int ax, int ay, int az,
		int bx, int by, int bz);

	void syncBoundaryWithNeigbour(Chunk* chunk, Chunk* neighbor, int dir);
public:
	/*
	 * Adds light with specified emission right in chunk without chunk finding.
	 * @param x local
	 * @param y local
	 * @param z local
	 */
	void addLocally(int x, int y, int z, uint8_t emission, int32_t channel, Chunk* chunk);

	void solve(ChunkPtr chunk);

	void solveRecursive(ChunkPtr chunk);

	void calculateLight(ChunkPtr chunk);

	/*
	 * Propagates light sun top to bottom.
	 * @param chunk
	 */
	void propagateSunLight(ChunkPtr chunk);

	/*
	 * Propagates light sun top to bottom.
	 * @param chunk
	 */
	void propagateSunRay(int lx, int lz, ChunkPtr &chunk);
};


#endif /* LIGHTSOLVER_HPP */
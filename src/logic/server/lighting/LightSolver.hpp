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

class LightSolver {
	int channel;

	std::queue<LightEntry> addqueue;
	std::queue<LightEntry> remqueue;

	friend class BasicLightSolver;
public:
	LightSolver(int channel);

	/*
	 * Adds light right in chunk without chunk finding.
	 * @param x local
	 * @param y local
	 * @param z local
	 */
	void addLocally(int x, int y, int z, Chunk* chunk);

	/*
	 * Adds light with specified emission right in chunk without chunk finding.
	 * @param x local
	 * @param y local
	 * @param z local
	 */
	void addLocally(int x, int y, int z, uint8_t emission, Chunk* chunk);

	void remove(int x, int y, int z);

	void removeLocally(int lx, int ly, int lz, Chunk* chunk);

	void solve();
};

class BasicLightSolver {
	void processBoundaryBlock(
		Chunk* A, Chunk* B,
		int ax, int ay, int az,
		int bx, int by, int bz,
		std::array<bool, 4> &addedAny);

	void syncBoundaryWithNeigbour(
		Chunk* chunk, Chunk* neighbor,
		int dir, std::array<bool, 4> &addedAny);
		
	inline LightSolver* getSolver(int chan) {
		switch(chan) {
			case 0: return  solverR.get();
			case 1: return  solverG.get();
			case 2: return  solverB.get();
			case 3: return  solverS.get();
		}
	}
public:
	std::unique_ptr<LightSolver> solverR, solverG, solverB, solverS;
	BasicLightSolver();

	void calculateLight(ChunkPtr &chunk);

	/*
	 * Propagates light sun top to bottom.
	 * @param chunk
	 */
	void propagateSunLight(Chunk* chunk);

	/*
	 * Propagates light sun top to bottom.
	 * @param chunk
	 */
	void propagateSunRay(int lx, int lz, ChunkPtr &chunk);

	void removeLightLocally(int lx, int ly, int lz, ChunkPtr &chunk);
	
	void placeLightLocally(int lx, int ly, int lz, Emission emission, ChunkPtr &chunk);
};


#endif /* LIGHTSOLVER_HPP */
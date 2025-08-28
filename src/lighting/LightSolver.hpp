#ifndef LIGHTSOLVER_HPP_
#define LIGHTSOLVER_HPP_

#include <memory>
#include <queue>
#include <cstdint>
#include <structures/RingBuffer.hpp>

#include <shared_mutex>

class Chunks;
class Chunk;

struct LightEntry {
	/* Local Coords */
	int32_t lx;
	int32_t ly;
	int32_t lz;
	uint8_t light;
	std::weak_ptr<Chunk> chunk;
};

constexpr int MAX_LIGHT_UPDATES = 1 << 20;

class LightSolver {
	RingBuffer<LightEntry, MAX_LIGHT_UPDATES> addqueue;
	RingBuffer<LightEntry, MAX_LIGHT_UPDATES> remqueue;
	Chunks* chunks;
	int channel;

	friend class BasicLightSolver;
public:
	mutable std::shared_mutex lightMutex;

	LightSolver(Chunks* chunks, int channel);

	/*
	 * Adds light right in chunk without chunk finding.
	 * @param x local
	 * @param y local
	 * @param z local
	 */
	void addLocally(int x, int y, int z, const std::shared_ptr<Chunk>& chunk);

	/*
	 * Adds light with specified emission right in chunk without chunk finding.
	 * @param x local
	 * @param y local
	 * @param z local
	 */
	void addLocally(int x, int y, int z, uint8_t emission, const std::shared_ptr<Chunk>& chunk);

	void remove(int x, int y, int z);

	void removeLocally(int lx, int ly, int lz, const std::shared_ptr<Chunk> &chunk);

	void solve();
};

class BasicLightSolver {
	std::unique_ptr<LightSolver> solverR, solverG, solverB, solverS;
	
	void processBoundaryBlock(
		const std::shared_ptr<Chunk>& A, const std::shared_ptr<Chunk>& B,
		int ax, int ay, int az,
		int bx, int by, int bz,
		std::array<bool, 4> &addedAny);

	void syncBoundaryWithNeigbour(
		const std::shared_ptr<Chunk>& chunk, const std::shared_ptr<Chunk>& neighbor,
		int dir, std::array<bool, 4> &addedAny);
		
	inline LightSolver* getSolver(int chan) {
		switch(chan) {
			case 0: return  solverR.get();
			case 1: return  solverG.get();
			default: return solverB.get();
		}
	}
public:
	BasicLightSolver(Chunks* chunks);

	/*
	 * Propagates light sun top to bottom.
	 * @param chunk
	 */
	void propagateSunLight(const std::shared_ptr<Chunk>& chunk);

	/*
	 * Calculates light for chunks and neighbours around it.
	 */
	void calculateLight(const std::shared_ptr<Chunk>& chunk);


	void removeLightLocally(int lx, int ly, int lz, const std::shared_ptr<Chunk> &chunk);
};

#endif /* LIGHTSOLVER_HPP */
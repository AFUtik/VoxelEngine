#ifndef LIGHTSOLVER_HPP_
#define LIGHTSOLVER_HPP_

#include <memory>
#include <queue>
#include <cstdint>
#include <structures/RingBuffer.hpp>

class Chunks;
class Chunk;

struct LightEntry {
	/* Local Coords */
	int32_t lx;
	int32_t ly;
	int32_t lz;
	uint8_t light;

	Chunk* chunk;
};

constexpr int MAX_LIGHT_UPDATES = 1 << 20;

class LightSolver {
	RingBuffer<LightEntry, MAX_LIGHT_UPDATES> addqueue;
	RingBuffer<LightEntry, MAX_LIGHT_UPDATES> remqueue;
	Chunks* chunks;
	int channel;
public:
	LightSolver(Chunks* chunks, int channel);

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

	void solve();
};

#endif /* LIGHTSOLVER_HPP */
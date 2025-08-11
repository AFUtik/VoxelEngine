#ifndef LIGHTSOLVER_HPP_
#define LIGHTSOLVER_HPP_

#include <queue>
#include <cstdint>

class Chunks;
class Chunk;

struct LightEntry {
	/* Local Coords */
	int32_t lx;
	int32_t ly;
	int32_t lz;
	uint8_t light;

	Chunk *chunk = nullptr;
};

template <typename T, size_t UPDATES>
struct RingBuffer {
private:
	T buffer[UPDATES];
	int head = 0, tail = 0;
public:
	inline void write(LightEntry entry) {
		buffer[head] = entry;
		head = (head + 1) % UPDATES;
	}

	inline T& read() {
		T& entry = buffer[tail];
		tail = (tail + 1) % UPDATES;
		return entry;
	}

	inline bool empty() const {
		return head == tail;
	}
};

constexpr int MAX_LIGHT_UPDATES = 1 << 18; // 65536

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
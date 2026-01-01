#ifndef LIGHTSOLVER_HPP_
#define LIGHTSOLVER_HPP_

#include <memory>
#include <cstdint>

#include "LightInfo.hpp"
#include "../blocks/Chunk.hpp"
#include "../blocks/ChunkInfo.hpp"

template <typename T, size_t UPDATES>
struct RingBuffer {
private:
    T buffer[UPDATES];
    int head = 0, tail = 0;
public:
    inline void write(T entry) {
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

class LogicSystem;

class LightSolver {
	RingBuffer<LightEntry, ChunkInfo::VOLUME*2> addqueue;
	RingBuffer<LightEntry, ChunkInfo::VOLUME*2> remqueue;
	LogicSystem* chunks;
	int channel;

	friend class BasicLightSolver;
public:
	LightSolver(LogicSystem* chunks, int channel);

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
	

	void processBoundaryBlockSingle(
		Chunk* A, Chunk* B,
		int ax, int ay, int az,
		int face,
		std::array<bool,4> &addedAny);
	
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
			default: return solverB.get();
		}
	}
public:
	std::unique_ptr<LightSolver> solverR, solverG, solverB, solverS;
	BasicLightSolver(LogicSystem* chunks);

	/*
	 * Propagates light sun top to bottom.
	 * @param chunk
	 */
	void propagateSunLight(Chunk* chunk);

	/*
	 * Propagates light sun top to bottom.
	 * @param chunk
	 */
	void propagateSunRay(int lx, int lz, Chunk* chunk);

	/*
	 * Calculates light for chunks and neighbours around it.
	 */
	void calculateLight(Chunk* chunk);

	/*
	 * Calculates light for chunks and neighbours around it.
	 */
	void calculateLight(ChunkSnapshot* chunk);

	void removeLightLocally(int lx, int ly, int lz, Chunk* chunk);
	
	void placeLightLocally(int lx, int ly, int lz, Emission emission, Chunk* chunk);
};

#endif /* LIGHTSOLVER_HPP */
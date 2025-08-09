#ifndef LIGHTSOLVER_HPP_
#define LIGHTSOLVER_HPP_

#include <queue>
#include <cstdint>

class Chunks;

struct lightentry {
	int x;
	int y;
	int z;
	unsigned char light;
};

class LightSolver {
	std::queue<lightentry> addqueue;
	std::queue<lightentry> remqueue;
	Chunks* chunks;
	int channel;
public:
	LightSolver(Chunks* chunks, int channel);

	void add(int x, int y, int z);
	void add(int x, int y, int z, uint8_t emission);
	void remove(int x, int y, int z);
	void solve();
};

#endif /* LIGHTSOLVER_HPP */
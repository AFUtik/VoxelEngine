#ifndef BLOCKCONTAINER_HPP
#define BLOCKCONTAINER_HPP

#include "Block.hpp"
#include <memory>

class BlockContainerDynamic {
private:
    std::unique_ptr<block[]> blocks;
    int w, h, d;
public:
    BlockContainerDynamic(int w, int d, int h) : w(w), d(d), h(h), blocks(std::make_unique<block[]>(w*h*d)) {}

    inline block getBlock(int32_t lx, int32_t ly, int32_t lz) const {return blocks[(ly * d + lz) * w + lx];}
	inline void setBlock(int32_t lx, int32_t ly, int32_t lz, uint8_t id) {blocks[(ly * d + lz) * h + lx].id = id;}
};

template<size_t W, size_t D, size_t H>
class BlockContainer {
private:
	static constexpr size_t VOLUME = W*H*D;
	block blocks[VOLUME];
public:
	inline block getBlock(int32_t lx, int32_t ly, int32_t lz) const {return blocks[(ly * D + lz) * W + lx];}
	inline void setBlock(int32_t lx, int32_t ly, int32_t lz, uint8_t id) {blocks[(ly * D + lz) * H + lx].id = id;}
};

#endif
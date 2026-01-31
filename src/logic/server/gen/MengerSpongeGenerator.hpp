#ifndef MENGERSPONGEGENERATOR_HPP
#define MENGERSPONGEGENERATOR_HPP

#include "../blocks/Chunk.hpp"
#include "../blocks/BlockContainer.hpp"

class MengerSpongeGenerator {
    BlockContainer<54, 54, 54> fractal;
    int iterations;

    inline bool isMengerBlockFast(int x, int y, int z)
    {
        while (x || y || z)
        {
            if ((x % 2 == 1 && y % 2 == 1) ||
                (x % 2 == 1 && z % 2 == 1) ||
                (y % 2 == 1 && z % 2 == 1))
                return false;

            x /= 2; y /= 2; z /= 2;
        }
        return true;
    }
public:
    MengerSpongeGenerator(int size, int iterations);
    void generate(ChunkPtr ptr);
};

#endif
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
            if ((x % 3 == 1 && y % 3 == 1) ||
                (x % 3 == 1 && z % 3 == 1) ||
                (y % 3 == 1 && z % 3 == 1))
                return false;

            x /= 3; y /= 3; z /= 3;
        }
        return true;
    }
public:
    MengerSpongeGenerator(int size, int iterations);
    void generate(ChunkPtr ptr);
};

#endif
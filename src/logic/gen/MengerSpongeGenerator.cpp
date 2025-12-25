#include "MengerSpongeGenerator.hpp"
#include <memory>

MengerSpongeGenerator::MengerSpongeGenerator(int size, int iterations) {
    for(int x = 0; x < 54; x++)
    for(int z = 0; z < 54; z++)
    for(int y = 0; y < 54; y++) 
    {
        bool solid = isMengerBlockFast(x, y, z);
        if(solid) fractal.setBlock(x, y, z, solid);
    }
}

void MengerSpongeGenerator::generate(ChunkPtr ptr) {
    for(int x = 0; x < ChunkInfo::WIDTH;  x++)
    for(int z = 0; z < ChunkInfo::DEPTH;  z++)
    for(int y = 0; y < ChunkInfo::HEIGHT; y++) 
    {
        int gx = abs((x+ptr->x*ChunkInfo::WIDTH )%54);
        int gy = abs((y+ptr->y*ChunkInfo::HEIGHT)%54);
        int gz = abs((z+ptr->z*ChunkInfo::DEPTH )%54);
 
        bool solid = fractal.getBlock(gx, gy, gz).id;
        if(solid) ptr->setBlock(x, y, z, solid);
    }
}
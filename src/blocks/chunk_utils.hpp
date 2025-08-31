#ifndef CHUNK_UTILS_HPP
#define CHUNK_UTILS_HPP

#include <array>

#include "ChunkInfo.hpp"

const int OFFSETS[26][3] = {
	{-1,-1,-1}, {0,-1,-1}, {1,-1,-1},
	{-1,0,-1}, {0,0,-1}, {1,0,-1},
	{-1,1,-1}, {0,1,-1}, {1,1,-1},
	{-1,-1,0}, {0,-1,0}, {1,-1,0},
	{-1,0,0},           {1,0,0},
	{-1,1,0},  {0,1,0}, {1,1,0},
	{-1,-1,1}, {0,-1,1}, {1,-1,1},
	{-1,0,1},  {0,0,1},  {1,0,1},
	{-1,1,1},  {0,1,1},  {1,1,1}
};

const int FACE_DIRS[6][3] = {
	{0,0,1}, 
	{0,0,-1}, 
	{0,1,0}, 
	{0,-1,0}, 
	{1,0,0}, 
	{-1,0,0}
};

static constexpr int encode3(int dx, int dy, int dz) noexcept {
    return (dx + 1) + (dy + 1) * 3 + (dz + 1) * 9; // 0..26
}

static std::array<int, 27> buildNeiIndexByEncode() {
    std::array<int,27> t;
    t.fill(-1);
    for (int i = 0; i < 26; ++i) {
        int dx = OFFSETS[i][0];
        int dy = OFFSETS[i][1];
        int dz = OFFSETS[i][2];
        t[ encode3(dx, dy, dz) ] = i;
    }
    return t;
}

static const std::array<int,27> NEI_INDEX_BY_ENCODE = buildNeiIndexByEncode();

static std::array<int,26> makeOpposites() {
        std::array<int,26> arr{};
        for (int i = 0; i < 26; ++i) {
            arr[i] = -1;
            for (int j = 0; j < 26; ++j) {
                if (OFFSETS[j][0] == -OFFSETS[i][0] &&
                    OFFSETS[j][1] == -OFFSETS[i][1] &&
                    OFFSETS[j][2] == -OFFSETS[i][2]) {
                    arr[i] = j;
                    break;
                }
            }
        }
        return arr;
    }

static inline std::array<int,26> oppositeIdx = makeOpposites();

static constexpr inline int floorDiv(int a, int b) {
	if (b < 0) b = -b;
	int q = a / b;
	int r = a % b;
	if (r != 0 && ((r < 0) != (b < 0))) --q;
	return q;
}

static inline int dirFromDelta(int dx, int dy, int dz) {
    for (int d = 0; d < 6; ++d) {
        if (FACE_DIRS[d][0] == dx && FACE_DIRS[d][1] == dy && FACE_DIRS[d][2] == dz) return d;
    }
    return -1;
}


static inline int faceToIdx(int face) {
    int dx = FACE_DIRS[face][0];
    int dy = FACE_DIRS[face][1];
    int dz = FACE_DIRS[face][2];

    for (int i = 0; i < 26; ++i) {
        if (OFFSETS[i][0] == dx &&
            OFFSETS[i][1] == dy &&
            OFFSETS[i][2] == dz) {
            return i;
        }
    }
    return -1;
}

inline bool isOnFace(int lx, int ly, int lz, int face) {
    const int W = ChunkInfo::WIDTH;
    const int H = ChunkInfo::HEIGHT;
    const int D = ChunkInfo::DEPTH;
    switch(face) {
        case 0: return lz == D-1; // +Z
        case 1: return lz == 0;   // -Z
        case 2: return ly == H-1; // +Y
        case 3: return ly == 0;   // -Y
        case 4: return lx == W-1; // +X
        case 5: return lx == 0;   // -X
    }
    return false;
}

inline int coordsIntoNeighInd(int lx, int ly, int lz) {
    int dx = 0, dy = 0, dz = 0;

    if      (lx < 0)                  dx = -1;
    else if (lx >= ChunkInfo::WIDTH)  dx =  1;

    if      (ly < 0)                   dy = -1;
    else if (ly >= ChunkInfo::HEIGHT)  dy =  1;

    if      (lz < 0)                  dz = -1;
    else if (lz >= ChunkInfo::DEPTH)  dz =  1;

    return NEI_INDEX_BY_ENCODE[ encode3(dx, dy, dz)];
}

inline bool inBounds(int x, int y, int z) noexcept {
    return  (unsigned)x < ChunkInfo::WIDTH  &&
            (unsigned)y < ChunkInfo::HEIGHT &&
            (unsigned)z < ChunkInfo::DEPTH;
}

inline bool onBorders(int x, int y, int z) noexcept {
    return (unsigned)x == ChunkInfo::WIDTH  - 1  || 
           (unsigned)y == ChunkInfo::HEIGHT - 1  || 
           (unsigned)z == ChunkInfo::DEPTH  - 1;
}

inline int neighbourIndexFromDelta(int dx, int dy, int dz) noexcept {
    return NEI_INDEX_BY_ENCODE[encode3(dx, dy, dz)];
}

#endif 
#pragma once

#define COORD_SIZE 3
#define TEX_COORD_SIZE 2
#define LIGHT_SIZE 4

#define VERTEX_SIZE (COORD_SIZE+TEX_COORD_SIZE+LIGHT_SIZE)

#define TRIANGLE_INDEX_SIZE 3
#define QUAD_INDEX_SIZE 6
#define CUBE_INDEX_SIZE 36

#define QUAD_VERTEX_SIZE 4

static unsigned int ATTRIBUTES[] = {COORD_SIZE, TEX_COORD_SIZE, LIGHT_SIZE, 0};

struct Vertex {
    float x, y, z;
    float u, v;
    float lr, lg, lb, ls;
};
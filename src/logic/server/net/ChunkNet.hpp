#ifndef CHUNKNET_HPP
#define CHUNKNET_HPP

#include <vector>
#include <zip.hpp>

#include "../blocks/Chunk.hpp"

inline void append(std::vector<uint8_t>& v,
    const void* src,
    size_t size)
{
    const uint8_t* p = static_cast<const uint8_t*>(src);
    v.insert(v.end(), p, p + size);
}

inline std::vector<uint8_t> serializeChunk(const ChunkPtr& c)
{
    constexpr size_t totalSize =
        sizeof(Vector3I) +
        sizeof(uint32_t) +
        ChunkInfo::VOLUME * sizeof(block) +
        ChunkInfo::VOLUME * sizeof(uint16_t);

    std::vector<uint8_t> out;
    out.reserve(totalSize);

    auto writeInt = [&](int32_t v) {
        out.insert(out.end(),
            (uint8_t*)&v,
            (uint8_t*)&v + 4);
        };

    writeInt(c->pos.x);
    writeInt(c->pos.y);
    writeInt(c->pos.z);
    append(out, &c->version, sizeof(uint32_t));

    append(out,
        c->blocks.get(),
        ChunkInfo::VOLUME * sizeof(block));

    append(out,
        c->lightmap->map,
        ChunkInfo::VOLUME * sizeof(uint16_t));

    return out; // NRVO
}

inline ChunkPtr deserializeChunk(const std::vector<uint8_t>& data)
{
    constexpr size_t expected =
        sizeof(Vector3I) +
        sizeof(uint32_t) +
        ChunkInfo::VOLUME * sizeof(block) +
        ChunkInfo::VOLUME * sizeof(uint16_t);

    if (data.size() != expected)
        return nullptr;

    auto c = std::make_shared<Chunk>();

    size_t off = 0;

    auto readInt = [&](int32_t& v) {
        memcpy(&v, data.data() + off, 4);
        off += 4;
        };

    readInt(c->pos.x);
    readInt(c->pos.y);
    readInt(c->pos.z);

    memcpy(&c->version, data.data() + off, sizeof(uint32_t));
    off += sizeof(uint32_t);

    memcpy(c->blocks.get(), data.data() + off,
        ChunkInfo::VOLUME * sizeof(block));
    off += ChunkInfo::VOLUME * sizeof(block);

    memcpy(c->lightmap->map, data.data() + off,
        ChunkInfo::VOLUME * sizeof(uint16_t));

    return c;
}

#endif
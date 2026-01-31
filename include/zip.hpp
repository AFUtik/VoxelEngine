#ifndef ZIP_HPP
#define ZIP_HPP

#include <zstd.h>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

inline std::vector<uint8_t> CompressZstd(const void* data, size_t srcSize)
{
    size_t bound = ZSTD_compressBound(srcSize);

    std::vector<uint8_t> out(bound);

    size_t size = ZSTD_compress(
        out.data(), bound,
        data, srcSize,
        3
    );

    if (ZSTD_isError(size))
        throw std::runtime_error(ZSTD_getErrorName(size));

    out.resize(size);
    return out;
}

inline std::vector<uint8_t> DecompressZstd(const void* data, size_t compressedSize, size_t originalSize)
{
    std::vector<uint8_t> out(originalSize);

    size_t size = ZSTD_decompress(
        out.data(), originalSize,
        data, compressedSize
    );

    if (ZSTD_isError(size))
        throw std::runtime_error(ZSTD_getErrorName(size));

    return out;
}

#endif
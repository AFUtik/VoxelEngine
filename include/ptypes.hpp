#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <map>
#include <memory>
#include <atomic>
#include <queue>
#include <deque>
#include <mutex>
#include <shared_mutex>
#include <iostream>
#include <chrono>
#include <optional>

using namespace std;
using namespace glm;

using Vector3 = glm::dvec3;
using Vector2 = glm::dvec2;

template<typename T>
struct HashVector : public glm::vec<3, T, glm::packed_highp> {
    using Base = glm::vec<3, T, glm::packed_highp>;

    HashVector() : Base(0) {}

    HashVector(T x, T y, T z) : Base(x, y, z) {}

    constexpr bool operator==(const HashVector<T>& o) const noexcept {
        return x == o.x && y == o.y && z == o.z;
    }

    constexpr bool operator!=(const HashVector<T>& o) const noexcept {
        return x != o.x || y != o.y || z != o.z;
    }

    constexpr bool operator<(const HashVector<T>& o) const noexcept {
        if (x != o.x) return x < o.x;
        if (y != o.y) return y < o.y;
        return z < o.z;
    }

    constexpr bool operator>(const HashVector<T>& o) const noexcept {
        if (x != o.x) return x > o.x;
        if (y != o.y) return y > o.y;
        return z > o.z;
    }
};

namespace std
{
    template<typename T>
    struct hash<HashVector<T>>
    {
        size_t operator()(const HashVector<T>& p) const noexcept
        {
            size_t h = 0;
            auto hash_combine = [&](size_t v)
                {
                    h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
                };
            hash_combine(std::hash<T>{}(p.x));
            hash_combine(std::hash<T>{}(p.y));
            hash_combine(std::hash<T>{}(p.z));

            return h;
        }
    };
};

using Vector3I  = HashVector<int>;
using Vector3LL = HashVector<long long>;
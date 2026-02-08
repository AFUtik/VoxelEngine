#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <array>
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
struct HashVector : public glm::vec<3, T, glm::defaultp> {
    using Base = glm::vec<3, T, glm::defaultp>;

    HashVector() : Base(0) {}

    HashVector(T x, T y, T z) : Base(x, y, z) {}

    constexpr bool operator==(const HashVector<T>& o) const noexcept {
        return Base::x == o.x && Base::y == o.y && Base::z == o.z;
    }

    constexpr bool operator!=(const HashVector<T>& o) const noexcept {
        return Base::x != o.x || Base::y != o.y || Base::z != o.z;
    }

    constexpr bool operator<(const HashVector<T>& o) const noexcept {
        if (Base::x != o.x) return Base::x < o.x;
        if (Base::y != o.y) return Base::y < o.y;
        return Base::z < o.z;
    }

    constexpr bool operator>(const HashVector<T>& o) const noexcept {
        if (Base::x != o.x) return Base::x > o.x;
        if (Base::y != o.y) return Base::y > o.y;
        return Base::z > o.z;
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
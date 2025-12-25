#ifndef BLOCKREGISTER_HPP
#define BLOCKREGISTER_HPP

#include <optional>
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>

#include "../graphics/texture/TextureRegion.hpp"

using BlockTextureSet = std::array<uint32_t, 6>;
using BlockID = uint16_t;
using StateIndex = uint32_t;

struct Identifier {
    BlockID id = 0;
    std::string name;

    Identifier() = default;
    Identifier(BlockID id_, std::string name_) : id(id_), name(std::move(name_)) {}
};

struct BlockState {
    BlockID id = 0;
    std::string name;

    uint32_t model_id;
    BlockTextureSet textures{};

    BlockState() = default;
    BlockState(BlockID id_, std::string name_, const BlockTextureSet& tex)
        : id(id_), name(std::move(name_)), textures(tex) {}
};

struct BlockPrototype {
    BlockID id = 0;
    std::string name;

    std::vector<BlockState> states;
    std::unordered_map<std::string, StateIndex> stateMap;

    BlockPrototype() = default;
    BlockPrototype(BlockID id_, std::string name_) : id(id_), name(std::move(name_)) {}

    StateIndex addState(BlockState state) {
        StateIndex idx = static_cast<StateIndex>(states.size());
        stateMap.emplace(state.name, idx);
        states.emplace_back(std::move(state));
        return idx;
    }

    std::optional<StateIndex> findStateIndex(std::string_view s) const {
        auto it = stateMap.find(std::string(s));
        if (it == stateMap.end()) return std::nullopt;
        return it->second;
    }
};

class BlockRegistry {
    static inline std::vector<BlockPrototype> registry;
    static inline std::unordered_map<std::string, uint32_t> registryMap;

    static inline BlockID nextBlockID = 1;
    static inline BlockID nextStateID = 1;
public:
    static uint32_t registerBlock(std::string name, BlockPrototype proto) {
        //std::lock_guard<std::mutex> lk(registryMutex);

        if (registryMap.find(name) != registryMap.end())
            return registryMap[name];

        proto.id = nextBlockID++;
        proto.name = std::move(name);

        uint32_t idx = static_cast<uint32_t>(registry.size());
        registryMap.emplace(proto.name, idx);
        registry.emplace_back(std::move(proto));
        return idx;
    }

    static std::optional<StateIndex> registerState(std::string_view block_name, BlockState state) {
        //std::lock_guard<std::mutex> lk(registryMutex);

        auto it = registryMap.find(std::string(block_name));
        if (it == registryMap.end()) return std::nullopt;

        // присвоим уникальный id состоянию (если нужно)
        state.id = nextStateID++;
        BlockPrototype &proto = registry[it->second];
        StateIndex idx = proto.addState(std::move(state));
        return idx;
    }

    static const BlockPrototype* getBlockByName(std::string_view name) {
        //std::lock_guard<std::mutex> lk(registryMutex);

        auto it = registryMap.find(std::string(name));
        if (it == registryMap.end()) return nullptr;
        return &registry[it->second];
    }

    static const BlockPrototype* getBlockByIndex(uint32_t index) {
        //std::lock_guard<std::mutex> lk(registryMutex);

        if (index >= registry.size()) return nullptr;
        return &registry[index];
    }
};

#endif
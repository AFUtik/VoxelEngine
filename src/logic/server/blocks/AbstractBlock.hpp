#ifndef ABSTRACTBLOCK_HPP
#define ABSTRACTBLOCK_HPP

#include <cstdint>
#include <string>

#include "../lighting/LightInfo.hpp"

struct AbstractBlock {
	uint16_t id = 0;
    std::string literalId  = "";

    bool opaque;

	Emission emission;
public:    
	inline uint16_t    getId() {return id;};
    inline std::string getLiteralId() {return literalId;};
    inline Emission    getEmission() {return emission;};
};

#endif
#include "LightCompressor.hpp"

void LightCompressor::compress(Lightmap* lightmap, RGBS_compression &out) {
    if (ChunkInfo::VOLUME == 0) return;

    for(int channel = 0; channel < 4; channel++) {
        if(!lightmap->addedRGBS[channel]) continue;

        uint8_t  start  = (lightmap->map[0] >> (channel << 2)) & 0xF;
        uint8_t  end    = start;
        uint16_t length = 1;
        uint8_t  prev   = start;
        bool equal = false;

        for (int i = 1; i < ChunkInfo::VOLUME; ++i) {
            uint8_t light = (lightmap->map[i] >> (channel << 2)) & 0xF;
            int diff = int(light) - int(prev);
            
            if ((diff == 1 || diff == -1) && !equal) {
                end = light;
                ++length;
            } 
            else if (light == prev && (equal || length == 1)) {
                ++length;
                equal = true;
            } 
            else {
                out[channel].push_back({start, end, length});
                start = end = light;
                prev = light;
                length = 1;
                equal = false;
                continue;
            }
            prev = light;
            
        }
        out[channel].push_back({start, end, length});
    }
}

void LightCompressor::decompress(Lightmap* lightmap, const RGBS_compression &in) {
    uint32_t idx = 0;

    for(int channel = 0; channel < 4; channel++) {
        if(!lightmap->addedRGBS[channel]) continue;

        for (const LightCompression &c : in[channel]) {
            if (c.start == c.end) {
                if(c.start == 0) {
                    idx+=c.length;
                    continue;
                }
                for (int k = 0; k < c.length; ++k) {
                    lightmap->map[idx] = (lightmap->map[idx] & (0xFFFF & (~(0xF << (channel * 4))))) | (c.start << (channel << 2));
                    ++idx;
                }
            } else {
                int step = (c.end > c.start) ? 1 : -1;
                int v = c.start;
                for (int k = 0; k < c.length; ++k) {
                    lightmap->map[idx] = (lightmap->map[idx] & (0xFFFF & (~(0xF << (channel * 4))))) | (v << (channel << 2));
                    v += step;
                    ++idx;
                }
            }
        }
    }
    

    if (idx != ChunkInfo::VOLUME) {
        throw std::runtime_error("Decompressed size != volume");
    }
}
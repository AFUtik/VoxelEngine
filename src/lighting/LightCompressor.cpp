#include "LightCompressor.hpp"

void LightCompressor::compress(Lightmap* lightmap, RGBS_compression &out) {
    if (ChunkInfo::VOLUME == 0) return;

    for (int channel = 0; channel < 4; channel++) {
        if (!lightmap->addedRGBS[channel]) continue;
        out[channel].clear();

        uint8_t  start  = (lightmap->map[0] >> (channel << 2)) & 0xF;
        uint8_t  end    = start;
        uint16_t length = 1;
        uint8_t  prev   = start;

        enum Mode { NONE, EQUAL, ASC, DESC } mode = NONE;

        for (int i = 1; i < ChunkInfo::VOLUME; ++i) {
            uint8_t light = (lightmap->map[i] >> (channel << 2)) & 0xF;
            int diff = int(light) - int(prev);

            bool appended = false;
            switch (mode) {
                case NONE:
                    if (light == prev) {
                        mode = EQUAL;
                        ++length;
                        end = light;
                    } else if (diff == 1) {
                        mode = ASC;
                        ++length;
                        end = light;
                    } else if (diff == -1) {
                        mode = DESC;
                        ++length;
                        end = light;
                    } else {
                        out[channel].push_back({start, end, length});
                        start = end = light;
                        length = 1;
                        mode = NONE;
                    }
                    break;

                case EQUAL:
                    if (light == prev) {
                        ++length;
                        end = light;
                    } else {
                        out[channel].push_back({start, end, length});
                        start = end = light;
                        length = 1;
                        mode = NONE;
                    }
                    break;

                case ASC:
                    if (diff == 1) {
                        ++length;
                        end = light;
                    } else {
                        out[channel].push_back({start, end, length});
                        start = end = light;
                        length = 1;
                        mode = NONE;
                    }
                    break;

                case DESC:
                    if (diff == -1) {
                        ++length;
                        end = light;
                    } else {
                        out[channel].push_back({start, end, length});
                        start = end = light;
                        length = 1;
                        mode = NONE;
                    }
                    break;
            }

            prev = light;
        }

        // Финальный сброс
        out[channel].push_back({start, end, length});
    }
}

void LightCompressor::decompress(Lightmap* lightmap, const RGBS_compression &in) {
    for(int channel = 0; channel < 4; channel++) {
        if(in[channel].empty()) continue;
        lightmap->addedRGBS[channel] = true;
        uint32_t idx = 0;

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
            } 
            else {
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
}

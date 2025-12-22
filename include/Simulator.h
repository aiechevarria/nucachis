#pragma once

#include <stdint.h>

#include "Misc.h"
#include "Cache.h"
#include "MainMemory.h"
#include "PolicyReplacement.h"
#include "PolicyWrite.h"

typedef struct {
    // CPU configs
    int32_t cpuAddressWidth, cpuWordWidth, cpuRandSeed;

    // Memory configs
    int64_t memSize;
    double memAccessTimeSingle, memAccessTimeBurst;
    int64_t memPageSize, memPageBaseAddress;

    // Cache configs
    int64_t cacheSize[MAX_CACHE_LEVELS];
    int64_t cacheLineSize[MAX_CACHE_LEVELS];
    double cacheAccessTime[MAX_CACHE_LEVELS];
    uint8_t cacheAssoc[MAX_CACHE_LEVELS];
    PolicyWrite cachePolicyWrite[MAX_CACHE_LEVELS];
    PolicyReplacement cachePolicyReplacement[MAX_CACHE_LEVELS];
    bool cacheIsSplit[MAX_CACHE_LEVELS];

    // Other misc configs
    uint8_t miscCacheLevels;

} SimulatorConfig;


class Simulator {
private:
    // Pointers to elements of the memory hierarchy
    Cache* caches[MAX_CACHE_LEVELS];
    MainMemory* memory;

public:
    Simulator();
    ~Simulator();
};

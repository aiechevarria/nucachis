#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "MemoryElement.h"

// A cache line
typedef struct {
    uint64_t content;
    uint32_t tag, set, way;
    uint32_t firstAccess, lastAccess, numberAccesses;
    bool valid, dirty;
} CacheLine;


class Cache : public MemoryElement {
// Write policies
typedef enum {
    WRITE_BACK,
    WRITE_THROUGH,
    NUM_WRITE_POLICIES
} WritePolicy;

// Caches
typedef enum {
    DATA_CACHE,
    INST_CACHE,
    NUM_CACHE_TYPES
} CacheType;

private:
    // The actual cache structures
    CacheLine** caches[NUM_CACHE_TYPES];

    // Properties of the cache
    bool isSplit;
    WritePolicy writePolicy;
    uint32_t dataSets, dataWays, instSets, instWays;

public:
    Cache(uint32_t numSets, uint32_t numWays, WritePolicy policy, bool split);
    ~Cache();
    bool isCacheSplit();
    CacheLine** getDataCache();
    CacheLine** getInstCache();
};

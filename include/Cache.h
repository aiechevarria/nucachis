#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "Misc.h"
#include "MemoryElement.h"
#include "PolicyReplacement.h"
#include "PolicyWrite.h"

// A cache line
typedef struct {
    uint32_t* content;              // Pointer to an array of words
    uint32_t tag, set, way;
    uint32_t firstAccess, lastAccess, numberAccesses;
    bool valid, dirty;
} CacheLine;

class Cache : public MemoryElement {
// Caches
typedef enum {
    DATA_CACHE,
    INST_CACHE,
    NUM_CACHE_TYPES
} CacheType;

private:
    // The actual cache structures
    CacheLine* caches[NUM_CACHE_TYPES];

    // Properties of the cache
    uint64_t size, lineSize; 
    double accessTime;
    uint32_t sets, ways;
    bool isSplit;
    PolicyWrite policyWrite;
    PolicyReplacement policyReplacement;
    

public:
    Cache(SimulatorConfig* sc, uint8_t id);
    ~Cache();

    bool isCacheSplit();
    CacheLine* getDataCache();
    CacheLine* getInstCache();
    uint32_t getCacheLines();

    void flush();
};

#include <stdint.h>
#include <stdlib.h>

#include "Cache.h"

/**
 * Constructs a new Cache object.
 * 
 * @param sets The number of sets.
 * @param ways The number of ways.
 * @param policy The write policy used in the cache
 * @param split If the cache is split for instructions and data or not.
 */
Cache::Cache(uint32_t numSets, uint32_t numWays, WritePolicy policy, bool split) {
    uint8_t numCaches;
    writePolicy = policy;
    isSplit = split;

    // If the cache is split, split the ways and sets between both caches
    if (isSplit) {
        numCaches = 1;
        dataSets = numSets / 2;
        dataWays = numWays / 2;
        instSets = numSets / 2;
        instWays = numWays / 2;

        // Allocate the instruction cache
        caches[INST_CACHE] = (CacheLine**) malloc(instSets * instWays * sizeof(CacheLine));
    } else {
        numCaches = 2;
        dataSets = numSets;
        dataWays = numWays;

        caches[INST_CACHE] = nullptr;
    }

    // Allocate memory for the data cache
    caches[DATA_CACHE] = (CacheLine**) malloc(dataSets * dataWays * sizeof(CacheLine));

    // Populate the caches
    for (int i = 0; i < dataSets; i++) {
        for (int j = 0; j < dataWays; j++) {
            for (int k = 0 ; k < numCaches; k++) {
                caches[k][i][j].content = 0;
                caches[k][i][j].tag = 0;
                caches[k][i][j].set = i;
                caches[k][i][j].way = j;
                caches[k][i][j].firstAccess = 0;
                caches[k][i][j].lastAccess = 0;
                caches[k][i][j].numberAccesses = 0;
                caches[k][i][j].valid = false;
                caches[k][i][j].dirty = false;
            }
        }
    }

}

Cache::~Cache() {
    // Free the data cache regardless
    free(caches[DATA_CACHE]);

    // Only free the instruction one if the cache is split
    if (isSplit) {
        free(caches[INST_CACHE]);
    }
}

/**
 * Returns if the cache is split for vectors and instructions or not.
 * 
 * @return true The cache is split 
 * @return false The cache is unified
 */
bool Cache::isCacheSplit() {
    return isSplit;
}

/**
 * Returns the cache whole data cache. 
 * 
 * @return CacheLine** Pointer to the data cache
 */
CacheLine** Cache::getDataCache() {
    return caches[DATA_CACHE];
} 

/**
 * Returns the cache whole instruction cache. The caller MUST check if there is an instruction cache available.
 * 
 * @return CacheLine** Pointer to the instruction cache or nullptr if it does not exist.
 */
CacheLine** Cache::getInstCache() {
    return caches[INST_CACHE];
} 
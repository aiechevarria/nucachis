#include "Cache.h"

/**
 * Constructs a new Cache object.
 * 
 * @param sets The number of sets.
 * @param ways The number of ways.
 * @param policy The write policy used in the cache
 * @param split If the cache is split for instructions and data or not.
 */
Cache::Cache(SimulatorConfig* sc, uint8_t id) {
    size = sc->cacheSize[id];
    lineSize = sc->cacheLineSize[id];
    accessTime = sc->cacheAccessTime[id];
    ways = sc->cacheSize[id];                           // Ways per set
    isSplit = sc->cacheIsSplit[id];
    policyWrite = sc->cachePolicyWrite[id];
    policyReplacement = sc->cachePolicyReplacement[id];

    // Calculate the number of sets
    sets = size / lineSize / ways;

    if (isSplit) {
        // Allocate the caches
        caches[INST_CACHE] = (CacheLine**) malloc(sets / 2 * ways * sizeof(CacheLine));
        caches[DATA_CACHE] = (CacheLine**) malloc(sets / 2 * ways * sizeof(CacheLine));
    } else {
        caches[INST_CACHE] = nullptr;
        caches[DATA_CACHE] = (CacheLine**) malloc(sets * ways * sizeof(CacheLine));
    }

    // Init the cache
    flush();
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

/**
 * Resets the entire cache. 
 */
void Cache::flush() {
    uint32_t totalSets = sets;
    uint8_t numCaches = 1;

    if (isSplit) {
        numCaches++;
        totalSets = sets / 2;
    }

    for (int i = 0; i < totalSets; i++) {
        for (int j = 0; j < ways; j++) {
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
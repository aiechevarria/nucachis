#include "Cache.h"
#include "Misc.h"

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
    ways = sc->cacheAssoc[id];                           // Ways per set
    isSplit = sc->cacheIsSplit[id];
    policyWrite = sc->cachePolicyWrite[id];
    policyReplacement = sc->cachePolicyReplacement[id];

    // Precalculate some useful values
    sets = size / lineSize / ways;
    lineSizeWords = lineSize / (sc->cpuWordWidth / 8);     // Number of words in a line
    if (isSplit) {
        lines = sets * ways / 2;
    } else {
        lines = sets * ways;
    }

    if (isSplit) {
        // Allocate the caches
        caches[DATA_CACHE] = (CacheLine*) malloc(sets / 2 * ways * sizeof(CacheLine));
        caches[INST_CACHE] = (CacheLine*) malloc(sets / 2 * ways * sizeof(CacheLine));
    } else {
        caches[DATA_CACHE] = (CacheLine*) malloc(sets * ways * sizeof(CacheLine));
        caches[INST_CACHE] = nullptr;
    }

    // Allocate space for the content
    for (int i = 0; i < (isSplit ? 2 : 1); i++) {
        for (int j = 0; j < lines; j++) {
            // Allocate lineSizeWords slots for data
            caches[i][j].content = (uint64_t*) malloc(sizeof(uint64_t) * lineSizeWords);
        }
    }

    // Init all execution dependent stats
    flush();
}

Cache::~Cache() {
    // Deallocate the space for the content
    for (int i = 0; i < (isSplit ? 2 : 1); i++) {
        for (int j = 0; j < lines; j++) {
            free(caches[i][j].content);
        }
    }

    // Free the data cache
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
 * Returns the cache whole data or instruction cache. 
 * @param getInst 0 if the cache should be the data cache, 1 if it should be the instr cache. Returns the data cache by default
 * 
 * @return CacheLine** Pointer to the cache, nullptr if the cache does not exist.
 */
CacheLine* Cache::getCache(bool getInst) {
    if (getInst) {
        return caches[INST_CACHE];
    } else {
        return caches[DATA_CACHE];
    }
} 

/**
 * Gets the total number of lines in a cache. If the cache is split, it will return half the total lines.
 * 
 * @return uint32_t The number of lines in the cache
 */
uint32_t Cache::getLines() {
    return lines;
}

/**
 * Gets the total number of words stored in a cache line.
 * 
 * @return uint32_t The number of words.
 */
uint32_t Cache::getLineSizeWords() {
    return lineSizeWords;
}

/**
 * Gets the total number of accesses.
 * @return uint32_t The number of accesses
 */
uint32_t Cache::getAccesses() {
    return accesses;
}
    
/**
 * Gets the total number of hits.
 * @return uint32_t The number of hits
 */
uint32_t Cache::getHits() {
    return hits;
}

/**
 * Gets the total number of misses.
 * @return uint32_t The number of misses
 */
uint32_t Cache::getMisses() {
    return misses;
}

/**
 * Resets the entire cache. 
 */
void Cache::flush() {
    // Reset the stats
    accesses = 0;
    hits = 0;
    misses = 0;

    // Init the cache
    for (int i = 0; i < (isSplit ? 2 : 1); i++) {
        for (int j = 0; j < lines; j++) {
            // Init the content to 0
            for (int k = 0; k < lineSizeWords; k++) {
                caches[i][j].content[k] = 0;
            }

            // Init the rest of properties
            caches[i][j].tag = 0;
            caches[i][j].set = i;
            caches[i][j].way = j;
            caches[i][j].firstAccess = 0;
            caches[i][j].lastAccess = 0;
            caches[i][j].numberAccesses = 0;
            caches[i][j].valid = false;
            caches[i][j].dirty = false;
        }
    }
}

/**
 * Processes a memory operation that was sent from the upper level 
 * @param op The memory request that was made. 
 */
void Cache::processRequest(MemoryOperation* op, MemoryReply* rep) {

}
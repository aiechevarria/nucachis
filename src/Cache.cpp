#include "Cache.h"

/**
 * Constructs a new Cache object.
 * 
 * @param sets The number of sets.
 * @param ways The number of ways.
 * @param policy The write policy used in the cache
 * @param split If the cache is split for instructions and data or not.
 */
Cache::Cache(SimulatorConfig* sc, uint8_t identifier) {
    id = identifier;
    size = sc->cacheSize[id];
    lineSize = sc->cacheLineSize[id];                   // Size of the lines in Bytes
    accessTime = sc->cacheAccessTime[id];
    ways = sc->cacheAssoc[id];                          // Ways per set
    isSplit = sc->cacheIsSplit[id];
    policyWrite = sc->cachePolicyWrite[id];
    policyReplacement = sc->cachePolicyReplacement[id];
    wordWidth = sc->cpuWordWidth;

    // Precalculate some useful values
    lineSizeWords = lineSize / (wordWidth / 8);     // Number of words in a line
    sets = size / lineSize / ways;
    if (isSplit) {
        sets = sets / 2;
    }
    lines = sets * ways;

    if (isSplit) {
        // Allocate the caches
        caches[DATA_CACHE] = (CacheLine*) malloc(sets * ways * sizeof(CacheLine));
        caches[INST_CACHE] = (CacheLine*) malloc(sets * ways * sizeof(CacheLine));
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
 * @return uint32_t The number of lines in the cache
 */
uint32_t Cache::getLines() {
    return lines;
}

/**
 * Gets the total number of words stored in a cache line.
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
            caches[i][j].set = (uint32_t) j / ways;
            caches[i][j].way = j % ways;
            caches[i][j].firstAccess = -1;
            caches[i][j].lastAccess = -1;
            caches[i][j].numberAccesses = -1;
            caches[i][j].valid = false;
            caches[i][j].dirty = false;
            caches[i][j].lineColor = COLOR_NONE;
        }
    }
}

/*
 * ---------------------------
 * |  Tag  |   Set  | Offset |
 * ---------------------------
 */

 /**
  * Creates a mask with as many ones as numBits. 
  * @param numBits The number of bits to return.
  * @return uint64_t The mask.
  */
uint64_t Cache::getMask(uint64_t numBits) {
    // Creates a 1 of type Unsigned Long Long (64 bits at least)
    // Shift it numBits to the left and substract 1 so that it all the previous 0 bits become 1s.
    return ((1ULL << numBits) - 1);
}

/**
 * For a given address, gets it's tag.
 * @param address The address to calculate the tag
 * @return uint64_t The tag.
 */
uint64_t Cache::getTag(uint64_t address) {
    // Calculate the number of bits for the offset
    uint32_t offsetBits = log2(lineSize);
    
    // Calculate the number of bits for the set
    uint32_t setBits = log2(sets);

    // Remove the set and offset bits
    return address >> setBits >> offsetBits;
}

/**
 * For a given address, calculates the set in which it will be stored.
 * @param address The address to calculate the set
 * @return uint32_t The set.
 */
uint32_t Cache::getSet(uint64_t address) {
    // Calculate the number of bits for the offset
    uint32_t offsetBits = log2(lineSize);
    uint64_t addrWithoutOffset = address >> offsetBits;
    
    // Calculate the number of bits for the set
    uint32_t setBits = log2(sets);

    // And the address with a mask of setBits bits to remove the tag
    return addrWithoutOffset & getMask(setBits);
}

/**
 * For a given address, gets it's offset.
 * @param address The address to calculate the offset
 * @return uint32_t The offset.
 */
uint32_t Cache::getOffset(uint64_t address) {
    // Remove the bits that are not offset
    return address & getMask(log2(lineSize));
}

/**
 * Reconstructs an address without an offset from a tag and a set. 
 * @param tag The tag.
 * @param set The set.
 * @return uint64_t The address without the offset.
 */
uint64_t Cache::getAddressFromTagAndSet(uint64_t tag, uint32_t set) {
    // Calculate the number of bits for the offset and set
    uint32_t offsetBits = log2(lineSize);
    uint32_t setBits = log2(sets);
    return (tag << setBits << offsetBits) | (set << offsetBits);
}

/**
 * Searches if an address is present in the cache.
 * @param cache The cache to search
 * @param address The address to search. 
 * @return int32_t The cache line in which the data is present, or -1 if it is not present
 */
int32_t Cache::searchAddress(CacheLine* cache, uint64_t address) {
    uint64_t tag = getTag(address);
    uint32_t set = getSet(address);

    // Search in that set for the line
    for (int i = 0; i < ways; i++) {
        if (cache[set * ways + i].tag == tag) {
            return set * ways + i;
        }
    }

    // It it was not found, return -1
    return -1; 
}

/**
 * Extracts the specified number of words from the given cache line and puts them into the reply.
 * @param line The line 
 * @param op The operation with the data
 * @param rep The reply in which the data will be put
 */
void Cache::extractWordsFromLine(CacheLine line, MemoryOperation* op, MemoryReply* rep) {
    // Get the base index from which to extract the first word
    uint32_t baseIndex = getOffset(op->address) / (wordWidth / 8);

    assert (baseIndex + op->numWords <= lineSizeWords && "Multi-word requests that span two or more cache lines are unsupported");

    // Move all the requested words to the reply
    for (int i = 0; i < op->numWords; i++) {
        rep->data[i] = line.content[i + baseIndex];
    }
}

/**
 * Inserts the specified number of words from the given cache line. 
 * @param line The line 
 * @param op The operation with the data
 * @param rep The reply in which the data will be put
 */
void Cache::insertWordsInLine(CacheLine line, MemoryOperation* op) {
    // Get the base index from which to extract the first word
    uint32_t baseIndex = getOffset(op->address) / (wordWidth / 8);

    assert (baseIndex + op->numWords <= lineSizeWords && "Multi-word requests that span two or more cache lines are unsupported");

    // Move all the requested words to the reply
    for (int i = 0; i < op->numWords; i++) {
        line.content[i + baseIndex] = op->data[i];
    }
}

/**
 * Selects the most suitable line to be replaced on the cache for a given address. 
 * @param cache The cache to search in
 * @param address The address used to calculate the set.
 * @return uint32_t The line that was picked for eviction.
 */
uint32_t Cache::findReplacement(CacheLine* cache, uint64_t address) {
    uint32_t candidate;
    int32_t leastAccessed = -1;
    int32_t oldest = -1;
    uint32_t set = getSet(address);

    // If a line is invalid, return that instead of going through all policies.
    for (int i = 0; i < ways; i++) {
        if (!cache[set * ways + i].valid) {
            return set * ways + i;
        }
    }

    // Apply the corresponding replacement policy
    switch (policyReplacement) {
        case LRU:
            // If the policy is LRU, pick the one that has been referenced the longest ago
            for (int i = 0; i < ways; i++) {
                if (oldest == -1 || cache[set * ways + i].lastAccess < oldest) {
                    candidate = set * ways + i;
                    oldest = cache[set * ways + i].lastAccess;
                }
            }
            break;

        case LFU:
            // If the policy is LRU, pick the one that has been referenced the least
            for (int i = 0; i < ways; i++) {
                if (leastAccessed == -1 || cache[set * ways + i].numberAccesses < leastAccessed) {
                    candidate = set * ways + i;
                    leastAccessed = cache[set * ways + i].numberAccesses;
                }
            }
            break;

        case FIFO:
            // If the policy is FIFO, pick the one that was brought first (AKA, the oldest first access)
            for (int i = 0; i < ways; i++) {
                if (oldest == -1 || cache[set * ways + i].firstAccess < oldest) {
                    candidate = set * ways + i;
                    oldest = cache[set * ways + i].firstAccess;
                }
            }
            break;

        case RAND:
            // If the policy is RAND, pick a line randomly inside of that set
            candidate = set * ways + (rand() % ways);
            break;

        default:
            assert(0 && "Invalid replacement policy used");
            break;
    }

    return candidate;
}

/**
 * Fills an entire cache line with data from the lower level.
 * @param address The address to fetch.
 * @param isData If the address contains data or not.
 * @return double The total access time.
 */
double Cache::fetchFromLowerLevel(CacheLine* cache, uint64_t address, bool isData) {
    // Build a new request and reply for the lower level
    MemoryOperation newOp;
    MemoryReply newRep;
    double time = 0.0;

    newOp.address = address >> (uint8_t) log2(lineSize) << (uint8_t) log2(lineSize);    // Remove the offset to point to the base address to fetch
    newOp.numWords = lineSizeWords;
    newOp.operation = LOAD;
    newOp.isData = isData;

    newRep.data = (uint64_t*) malloc(sizeof(uint64_t) * lineSizeWords);
    newRep.totalTime = 0.0;

    // Throw the request to the lower level
    next->processRequest(&newOp, &newRep);

    // Update the stats
    time += newRep.totalTime;

    // Once the request is here, find a place to put it
    // Find replacement line function that uses the policy of the cache
    int32_t newLine = findReplacement(cache, address);
    printf("L%u%c: Picked line %d to be evicted\n", id + 1, (!isData && isSplit) ? 'I' : 'D', newLine);

    // Evict the data to the lower level
    if (cache[newLine].valid && cache[newLine].dirty) {
        // Prepare the eviction memory operation with all words in this line
        MemoryOperation evictOp;
        MemoryReply evictRep;

        evictOp.address = getAddressFromTagAndSet(cache[newLine].tag, cache[newLine].set);
        evictOp.numWords = lineSizeWords;
        evictOp.operation = STORE;
        evictOp.data = (uint64_t*) malloc(sizeof(uint64_t) * lineSizeWords);
        for (int i = 0; i < lineSizeWords; i++) {
            evictOp.data[i] = cache[newLine].content[i];
        }
        evictRep.totalTime = 0.0;

        // Send the eviction as a STORE to the lower level
        printf("L%u%c: Line %d is dirty and will be sent to the lower level\n", id + 1, (!isData && isSplit) ? 'I' : 'D', newLine);
        next->processRequest(&evictOp, &evictRep);

        // Update the stats
        time += evictRep.totalTime;

        // Free the mem
        free(evictOp.data);
    }

    // Put the data in the now free line.
    for (int i = 0; i < lineSizeWords; i++) {
        cache[newLine].content[i] = newRep.data[i];
    }

    cache[newLine].firstAccess = cycle;
    cache[newLine].numberAccesses = 0;
    cache[newLine].tag = getTag(address);
    cache[newLine].dirty = false;
    cache[newLine].valid = true;

    // Free the request
    free(newRep.data);

    return(time);
}

/**
 * Processes a memory operation that was sent from the upper level 
 * @param op The memory request that was made. 
 * @param rep The reply this cache provides. 
 */
void Cache::processRequest(MemoryOperation* op, MemoryReply* rep) {
    CacheLine* cache;

   if (debugLevel >= 1) printf("Debug: L%d, Address=%lu, Tag=%lu, Set=%u, Offset=%u\n", id + 1, op->address, getTag(op->address), getSet(op->address), getOffset(op->address));
    
    // Update the stats
    rep->totalTime += accessTime;
    accesses++;

    // Fetch the correct cache
    if (isSplit && !op->isData) {
        cache = caches[INST_CACHE];
    } else {
        cache = caches[DATA_CACHE];
    }
    
    // First, check if the data is present in the cache
    int32_t line = searchAddress(cache, op->address);

    // For loads
    if (op->operation == LOAD) {
        // If it is present 
        if (line != -1) {
            printf("L%u%c: Hit in line %d\n", id + 1, (!op->isData && isSplit) ? 'I' : 'D', line);
            hits++;
            cache[line].lineColor = COLOR_HIT;

            // Reply with that data
            extractWordsFromLine(cache[line], op, rep);
        } else {
            // If it is not present
            printf("L%u%c: Miss, fetching from lower level\n", id + 1, (!op->isData && isSplit) ? 'I' : 'D');
            misses++;

            // Query the lower level
            rep->totalTime += fetchFromLowerLevel(cache, op->address, op->isData);

            // Fetch the line again
            line = searchAddress(cache, op->address);
            assert(line != -1 && "The line should be found after being brought"); 
            cache[line].lineColor = COLOR_MISS;

            // Reply with that data
            extractWordsFromLine(cache[line], op, rep);
        }
    } else if (op->operation == STORE) {
        // For stores
        // If the cache is WT
        if (policyWrite == WRITE_THROUGH) {
            // Note it as a hit always
            hits++;
            
            // If the data is present in the cache, store it but do not flag it as dirty
            if (line != -1) {
                printf("L%u%c: Write-Through, updating already present data\n", id + 1, (!op->isData && isSplit) ? 'I' : 'D');
                insertWordsInLine(cache[line], op);
                cache[line].lineColor = COLOR_HIT;
            }

            printf("L%u%c: Write-Through, sending store to lower level\n", id + 1, (!op->isData && isSplit) ? 'I' : 'D');

            // Send it to the lower level (Reusing the reply, as no data will be stored on it)
            next->processRequest(op, rep);
        } else if (policyWrite == WRITE_BACK) {
            // If the cache is WB
            // If the line is not present
            if (line == -1) {
                misses++;

                // Query the lower level (Write-allocate)
                printf("L%u%c: Write-Back allocate miss, fetching from lower level\n", id + 1, (!op->isData && isSplit) ? 'I' : 'D');
                rep->totalTime += fetchFromLowerLevel(cache, op->address, op->data);

                // Search again for the address
                line = searchAddress(cache, op->address);
                assert(line != -1 && "The line should be found after being brought"); 
                cache[line].lineColor = COLOR_MISS;
            } else {
                hits++;
                cache[line].lineColor = COLOR_HIT;
            }

            printf("L%u%c: Storing in line %d\n", id + 1,  (!op->isData && isSplit) ? 'I' : 'D', line);

            // Store the data
            insertWordsInLine(cache[line], op);
            
            // Flag the line as dirty
            cache[line].dirty = true;
        } else {
            assert(0 && "Unsupported write policy type");
        }
    } else {
        assert(0 && "Unsupported operation type");
    }

    // Update the line stats
    cache[line].numberAccesses++;
    cache[line].lastAccess = cycle;
}

/**
 * Clears the style from all cache rows. 
 */
void Cache::clearStyle() {
    for (int i = 0; i < (isSplit ? 2 : 1); i++) {
        for (int j = 0; j < lines; j++) {
            caches[i][j].lineColor = COLOR_NONE;
        }
    }
}
#pragma once

#include <stdint.h>

#include "Misc.h"
#include "Cache.h"
#include "MainMemory.h"
#include "PolicyReplacement.h"
#include "PolicyWrite.h"

class Simulator {
private:
    // Private variables
    // Pointers to elements of the memory hierarchy
    Cache* caches[MAX_CACHE_LEVELS];
    MainMemory* memory;
    MemoryElement* hierarchyStart;  // The first element of the hierarchy. All messages will be sent to it

    // Instructions to execute
    MemoryOperation** operations;

    // CPU variables
    int32_t addressWidth, wordWidth;        // In Bytes
    uint32_t numOperations;
    uint8_t cacheLevels;

    // Stats
    double totalAccessTime;

public:
    Simulator(SimulatorConfig* sc, MemoryOperation** ops);
    ~Simulator();

    void singleStep();
    void stepAll(bool stopOnBreakpoint);
    void reset();

    // Object getters
    MemoryOperation** getOps();
    MainMemory* getMemory();
    Cache* getCache(uint8_t cache);

    // Other getters
    uint32_t getNumOps();
    uint8_t getNumCaches();
    uint32_t getAddressWidth();
    uint32_t getWordWidth();
    double getTotalAccessTime();

    void clearAllStyles();
};

#include "Simulator.h"

/**
 * Construct a new Simulator:: Simulator object
 * @param sc The simulator configs
 * @param ops The trace of operations to execute
 */
Simulator::Simulator(SimulatorConfig* sc, MemoryOperation** ops) {
    // Store the simulator and CPU configs
    wordWidth = sc->cpuWordWidth / 8;           // In Bytes
    addressWidth = sc->cpuAddressWidth;         // In bits
    numOperations = sc->miscNumOperations;
    cacheLevels = sc->miscCacheLevels;
    cycle = 0;

    // Set the rand seed for the simulation
    srand(sc->cpuRandSeed);

    // Store the trace
    operations = ops;

    // Init the stats
    totalAccessTime = 0.0f;

    // Create the memory hierarchy
    memory = new MainMemory(sc);
    for (int i = 0; i < cacheLevels; i++) {
        caches[i] = new Cache(sc, i);
    }

    /* TODO Is the previous pointer required ? 
       A memory element will get a function called and the reply is passed as a return of that fx */

    // Link all caches together
    for (int i = 1; i < cacheLevels; i++) {
        caches[i - 1]->setNext(caches[i]);
        caches[i]->setPrev(caches[i - 1]);
    }

    // If there is at least one cache, link the last cache to the main memory
    if (cacheLevels > 0) {
        caches[cacheLevels - 1]->setNext(memory);
        memory->setPrev(caches[cacheLevels - 1]);

        // Also, store a pointer to the first cache as the memory element that is closest to the CPU
        // All requests will be sent to this cache and it will have to take care of bringing the data by itself
        hierarchyStart = caches[0];
    } else {
        // If there is no cache, wire everything straight to the CPU
        hierarchyStart = memory;
    }
}

Simulator::~Simulator() {
    // Free the data in the memory operations loaded from the trace
    for (int i = 0; i < numOperations; i++) {
        free(operations[i]->data);
    }
}

/**
 * Runs a single instruction. 
 */
void Simulator::singleStep() {
    MemoryReply rep;

    // Check that the cycle is not greater than the number of ops
    if (cycle < numOperations) {
        // Clear previous styles
        clearAllStyles();

        // Set up the reply
        rep.totalTime = 0.0;
        rep.data = (uint64_t*) malloc(sizeof(uint64_t));

        // Display information on console
        printf("\n\n------ Cycle %d ------\n\n", cycle);
        if (operations[cycle]->operation == LOAD)  printf("CPU: Requested data on 0x%lX\n", operations[cycle]->address);
        if (operations[cycle]->operation == STORE) printf("CPU: Storing %lu on 0x%lX\n", operations[cycle]->data[0], operations[cycle]->address);

        // Throw the request to the first level of the memory hierarchy
        hierarchyStart->processRequest(operations[cycle], &rep);

        // Unpack the reply and free the data
        if (operations[cycle]->operation == LOAD)  printf("CPU: Finished load, got %lu in %.2f\n", rep.data[0], rep.totalTime);
        if (operations[cycle]->operation == STORE)  printf("CPU: Finished store in %.2f\n", rep.totalTime);
        totalAccessTime += rep.totalTime;
        free(rep.data);

        // Enter a new cycle
        cycle++;
    }
}

/**
 * Runs all instructions.
 * @param stopOnBreakpoint If true, it will stop on the first breakpoint it reaches, if false, it will run until the trace ends.
 */
void Simulator::stepAll(bool stopOnBreakpoint) {
    for (int i = cycle; i < numOperations; i++) {
        // Check if there was a breakpoint prior to executing the operation
        bool hasBreakPoint = operations[i]->hasBreakPoint;

        // Run the cycle and then stop afterwards if it had a breakpoint
        singleStep();
        if (hasBreakPoint && stopOnBreakpoint) break;
    }
}

/**
 * Sets the state to a default and starts the simulation from the beginning.
 */
void Simulator::reset() {
    // Reset the cycles
    cycle = 0;

    // Reset the stats
    totalAccessTime = 0.0;

    // Init the mem hierarchy
    memory->flush();
    for (int i = 0; i < cacheLevels; i++) {
        caches[i]->flush();
    }
}

/**
 * Returns the entire parsed trace.
 * @return MemoryOperation* Pointer to an array of memory operations that represent the trace.
 */
MemoryOperation** Simulator::getOps() {
    return operations;
}

/**
 * Returns the memory.
 * @return MainMemory* Pointer to the memory.
 */
MainMemory* Simulator::getMemory() {
    return memory;
}

/**
 * Returns a pointer to one of the caches
 * @param uint8_t The cache index
 * @return Cache** Pointer to an array of cache pointers.
 */
Cache* Simulator::getCache(uint8_t cache) {
    return caches[cache];
}

/**
 * Returns the number of operations in the trace.
 * @return uint32_t number of operations.
 */
uint32_t Simulator::getNumOps() {
    return numOperations;
}

/**
 * Returns the number of caches in the hierarchy.
 * @return uint8_t number of caches.
 */
uint8_t Simulator::getNumCaches() {
    return cacheLevels;
}

/**
 * Returns the address width in bits.
 * @return uint32_t The address width in Bytes 
 */
uint32_t Simulator::getAddressWidth() {
    return addressWidth;
}

/**
 * Returns the word width in Bytes.
 * @return uint32_t The word width in Bytes 
 */
uint32_t Simulator::getWordWidth() {
    return wordWidth;
}

/**
 * Returns the total access time.
 * @return uint32_t The total access time.
 */
double Simulator::getTotalAccessTime() {
    return totalAccessTime;
}

/**
 * Clears the styles from all data structures.
 */
void Simulator::clearAllStyles() {
    memory->clearStyle();
    for (int i = 0; i < cacheLevels; i++) {
        caches[i]->clearStyle();
    }
}

/**
 * Prints the current execution statistics to stdout.
 */
void Simulator::printStatistics() {
    printf("\n\n------ Statistics ------\n\n");
    printf("CPU:\n");
    printf("\tTotal access time (s): %.4f\n", totalAccessTime);
    printf("\tAverage memory access time (s): %.4f\n", totalAccessTime / (double) cycle);
    
    for (int i = 0; i < cacheLevels; i++) {
        Cache* cache = getCache(i);
        printf("\nCache L%d:\n", i + 1);
        printf("\tTotal accesses: %d\n", cache->getAccesses());
        printf("\tHits: %d\n", cache->getHits());
        printf("\tMisses: %d \n", cache->getMisses());
        printf("\tHit rate: %.1f%%\n", cache->getHits() / (double) cycle * 100);
        printf("\tMiss rate: %.1f%%\n", cache->getMisses() / (double) cycle * 100);
    }

    printf("\nMemory:\n");
    printf("\tTotal accesses: %ld\n", memory->getAccessesBurst() + memory->getAccessesSingle());
    printf("\tFirst word accesses: %ld\n", memory->getAccessesSingle());
    printf("\tBurst accesses: %ld\n", memory->getAccessesBurst());
}
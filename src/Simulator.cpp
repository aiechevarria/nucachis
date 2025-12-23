#include "Simulator.h"
#include "MainMemory.h"
#include "Misc.h"

/**
 * Construct a new Simulator:: Simulator object
 * 
 * @param sc The simulator configs
 */
Simulator::Simulator(SimulatorConfig* sc, MemoryOperation* ops) {
    // Store the simulator and CPU configs
    addressWidth = sc->cpuAddressWidth;
    wordWidth = sc->cpuWordWidth;
    numOperations = sc->miscNumOperations;
    cacheLevels = sc->miscCacheLevels;

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

    // Init everything to the default
    reset();
}

Simulator::~Simulator() {
}

/**
 * Runs a single instruction. 
 */
void Simulator::singleStep() {

}

/**
 * Runs all instructions until a breakpoint is reached or the trace ends.
 */
void Simulator::stepAll(bool stopOnBreakpoint) {

}

/**
 * Sets the state to a default and starts the simulation from the beginning.
 */
void Simulator::reset() {
    // Reset the program counter
    pc = 0;

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
MemoryOperation* Simulator::getOps() {
    return ops;
}

/**
 * Returns the memory.
 * @return MainMemory* Pointer to the memory.
 */
MainMemory* Simulator::getMemory() {
    return memory;
}

/**
 * Returns the caches.
 * @return Cache** Pointer to an array of cache pointers.
 */
Cache** Simulator::getCaches() {
    return caches;
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
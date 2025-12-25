#include "MainMemory.h"
#include "Misc.h"

MainMemory::MainMemory(SimulatorConfig* sc) {
    // Address size (This could be moved to the MemoryElement constructor in the future, perhaps)
    wordWidth = sc->cpuWordWidth / 4;
    addressWidth = sc->cpuAddressWidth / 4;

    // Memory geometry
    size = sc->memSize;
    pageSize = sc->memPageSize;
    pageBaseAddress = sc->memPageBaseAddress;

    // Memory timing
    accessTimeSingle = sc->memAccessTimeSingle;
    accessTimeBurst = sc->memAccessTimeBurst;

    // Allocate memory for the memory
    // The size is given in bytes, but the data is only addressable/displayed in words
    memory = (MemoryLine*) malloc(sizeof(MemoryLine) * (size / wordWidth));

    // Init all execution dependent stats
    flush();
}

MainMemory::~MainMemory() {
    free(memory);
}

/**
 * Gets the array of main memory elements.
 * @return MemoryLine* Pointer to the main memory
 */
MemoryLine* MainMemory::getMemory() {
    return memory;
}

/**
 * Gets the size of a page in Bytes.
 * @return uint64_t The size of the page in Bytes;
 */
uint64_t MainMemory::getPageSize() {
    return pageSize;
}

/**
 * Gets the base address in which the memory starts.
 * @return uint64_t The base address of the memory
 */
uint64_t MainMemory::getPageBaseAddress() {
    return pageBaseAddress;
}

/**
 * Resets the entire main memory.
 */
void MainMemory::flush() {
    // Calculate the maximum number of array items to cover up a page
    uint64_t pageLimit = pageSize / wordWidth;

    // Fill the memory with increasing numbers
    for (int i = 0; i < pageLimit; i++) {
        memory[i].address = i * wordWidth + pageBaseAddress;
        memory[i].content = i;
    }
}

/**
 * Processes a memory operation that was sent from the upper level 
 * @param op The memory request that was made. 
 */
void MainMemory::processRequest(MemoryOperation* op, MemoryReply* rep) {

}
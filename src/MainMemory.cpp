#include "MainMemory.h"

MainMemory::MainMemory(SimulatorConfig* sc) {
    // Memory geometry
    size = sc->memSize;
    pageSize = sc->memPageSize;
    pageBaseAddress = sc->memPageBaseAddress;

    // Memory timing
    accessTimeSingle = sc->memAccessTimeSingle;
    accessTimeBurst = sc->memAccessTimeBurst;

    // Allocate memory for the memory
    // The size is given in bytes, but the data is only addressable/displayed in 32-bit words
    memory = (MemoryLine*) malloc(sizeof(MemoryLine) * (size / 4));
}

MainMemory::~MainMemory() {
    free(memory);
}

uint32_t MainMemory::getWord(uint64_t address) {
    return memory[(address - pageBaseAddress) / 4].content;
}

void MainMemory::setWord(uint64_t address, int value) {
    memory[(address - pageBaseAddress) / 4].content = value;
}

/**
 * Resets the entire main memory.
 */
void MainMemory::flush() {
    // Calculate the maximum number of array items to cover up a page
    uint64_t pageLimit = pageSize / 4;

    // Fill the memory with increasing numbers
    for (int i = 0; i < pageLimit; i++) {
        memory[i].address = i * 4 + pageBaseAddress;
        memory[i].content = i;
    }
}
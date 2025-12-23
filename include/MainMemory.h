#pragma once

#include <unordered_map>
#include <vector>

#include "Misc.h"
#include "MemoryElement.h"

// A memory line
typedef struct {
    uint64_t address;
    uint32_t content;
} MemoryLine;

class MainMemory : public MemoryElement {
private:
    // Private variables
    // The actual main memory
    MemoryLine* memory;

    int64_t size, pageSize, pageBaseAddress;
    double accessTimeSingle, accessTimeBurst;

public:
    MainMemory(SimulatorConfig* sc);
    ~MainMemory();

    uint32_t getWord(uint64_t address);
    void setWord(uint64_t address, int value);

    void flush();
};

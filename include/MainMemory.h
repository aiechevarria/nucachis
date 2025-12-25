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

    int32_t addressWidth, wordWidth;        // In Bytes
    int64_t size, pageSize, pageBaseAddress;
    double accessTimeSingle, accessTimeBurst;

public:
    MainMemory(SimulatorConfig* sc);
    ~MainMemory();

    MemoryLine* getMemory();
    uint64_t getPageSize();
    uint64_t getPageBaseAddress();

    virtual void processRequest(MemoryOperation* op, MemoryReply* rep) override;

    void flush();
};

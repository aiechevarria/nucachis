#pragma once

#include <unordered_map>
#include <vector>

#include "Misc.h"
#include "MemoryElement.h"

// A memory line
typedef struct {
    uint64_t address;
    uint32_t content;
    ColorNames lineColor;
} MemoryLine;

class MainMemory : public MemoryElement {
private:
    // Private variables
    // The actual main memory
    MemoryLine* memory;

    int32_t addressWidth, wordWidth;
    int64_t size, pageSize, pageBaseAddress;
    double accessTimeSingle, accessTimeBurst;

    // Stats
    uint64_t accessesSingle, accessesBurst;

public:
    MainMemory(SimulatorConfig* sc);
    ~MainMemory();

    MemoryLine* getMemory();
    uint64_t getPageSize();
    uint64_t getPageBaseAddress();
    uint64_t getAccessesSingle();
    uint64_t getAccessesBurst();

    virtual void processRequest(MemoryOperation* op, MemoryReply* rep) override;
    virtual void clearStyle() override;

    void flush();
};

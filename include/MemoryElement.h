#pragma once

#include "Misc.h"

class MemoryElement {
protected:
    // Pointers to the next and previous elements in the memory hierarchy
    MemoryElement* next;
    MemoryElement* prev;

public:
    // Constructor
    MemoryElement();

    MemoryElement* getNext();
    MemoryElement* getPrev();

    void setNext(MemoryElement* nextElement);
    void setPrev(MemoryElement* prevElement);

    // Process request from another level
    virtual void processRequest(MemoryOperation* op, MemoryReply* rep) = 0;

    // Clear the style of whatever structure stores the code.
    virtual void clearStyle() = 0;
};

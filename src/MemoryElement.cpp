#include "MemoryElement.h"

// Constructor
MemoryElement::MemoryElement() : next(nullptr), prev(nullptr) {}

MemoryElement* MemoryElement::getNext() {
    return next;
}

MemoryElement* MemoryElement::getPrev() {
    return prev;
}

void MemoryElement::setNext(MemoryElement* nextElement) {
    next = nextElement;
}

void MemoryElement::setPrev(MemoryElement* prevElement) {
    prev = prevElement;
}

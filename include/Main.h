#pragma once

#include <string.h>

#include "CLI11.hpp"
#include "Misc.h"
#include "GUI.h"
#include "ParserConfig.h"
#include "ParserTrace.h"
#include "Simulator.h"

typedef struct {
    std::string configFile;
    std::string traceFile;
    int debug;
    bool noGui = false;     // Gui is on by default
} AppArgs;
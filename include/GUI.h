#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <GL/gl.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "Misc.h"
#include "Logo.h"
#include "Simulator.h"
#include "MainMemory.h"

// Proportions and placement of each window respective to the workspace
// Picker window
#define PICKER_WINDOW_WIDTH  0.50
#define PICKER_WINDOW_HEIGHT 0.45

// Instruction window
#define INSTR_WINDOW_WIDTH  0.25
#define INSTR_WINDOW_HEIGHT 0.65

// Memory window
#define MEM_WINDOW_WIDTH  0.15
#define MEM_WINDOW_HEIGHT 1

// Cache window
#define CACHE_WINDOW_WIDTH  (1 - (INSTR_WINDOW_WIDTH + MEM_WINDOW_WIDTH))   // Take up all the free remaining space
#define CACHE_WINDOW_HEIGHT 1
#define MIN_CACHE_TABLE_WIDTH 300.0f

// Stats window
#define STATS_WINDOW_WIDTH  INSTR_WINDOW_WIDTH
#define STATS_WINDOW_HEIGHT (1.0 - INSTR_WINDOW_HEIGHT) 

// Error window
#define ERROR_WINDOW_WIDTH  0.40
#define ERROR_WINDOW_HEIGHT 0.20

// GUI Colors
typedef enum {
    COLOR_RED,
    COLOR_ORANGE,
    COLOR_YELLOW,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_GREY,
    NUM_COLOR_NAMES    
} ColorNames;

class GUI {
private:
    SDL_Window* window;
    SDL_GLContext gl_context;
    
    // Images
    GLuint logo;

    // Window sizes
    int windowHeight, windowWidth; 

    // Draw functions
    GLuint LoadImageFromHeader(char* data, int width, int height, bool setTaskbarIcon);
    void centerNextItem(float itemWidth);
    void drawCacheTable(CacheLine* cache, uint32_t lineSizeWords, uint32_t numLines, char* label);

    // Main section renderers
    void renderInstructionWindow(Simulator* sim);
    void renderStatsWindow(Simulator* sim);
    void renderCacheWindow(Simulator* sim);
    void renderMemoryWindow(Simulator* sim);

public:
    GUI();
    ~GUI();
    SDL_Window* getWindow();
    void renderPicker(char configPath[MAX_PATH_LENGTH], char tracePath[MAX_PATH_LENGTH], bool freshLaunch, bool* clickedLaunch);
    void renderWorkspace(Simulator* sim);
    void renderError(char* message, bool* toggle);
};

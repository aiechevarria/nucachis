#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <GL/gl.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "Font.h"
#include "Misc.h"
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


class GUI {
private:
    SDL_Window* window;
    SDL_GLContext gl_context;
    
    // Images
    GLuint logo;

    // Table scroll
    // Used to scroll the first time a cycle passes but not hold the scroll in a certain position
    bool scrolledInstructions;
    bool scrolledCache[MAX_CACHE_LEVELS];
    bool scrolledMemory;

    // Window sizes
    int windowHeight, windowWidth; 

    // Draw functions
    GLuint LoadImageFromCSource(const unsigned char* rawData, int width, int height, bool setTaskbarIcon);
    void centerNextItem(float itemWidth);
    void drawCacheTable(CacheLine* cache, uint8_t id, uint32_t lineSizeWords, uint32_t numLines, char* label);

    // Main section renderers
    void renderInstructionWindow(Simulator* sim);
    void renderStatsWindow(Simulator* sim);
    void renderCacheWindow(Simulator* sim);
    void renderMemoryWindow(Simulator* sim);
    void resetScroll();

public:
    GUI();
    ~GUI();
    SDL_Window* getWindow();
    void renderPicker(char configPath[MAX_PATH_LENGTH], char tracePath[MAX_PATH_LENGTH], bool freshLaunch, bool* clickedLaunch);
    void renderWorkspace(Simulator* sim);
    void renderError(char* message, bool* toggle);
    
};

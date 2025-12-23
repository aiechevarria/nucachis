#pragma once

#include <SDL.h>
#include <SDL_opengl.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "Misc.h"
#include "Simulator.h"

// Proportions and placement of each window respective to the workspace
// Picker window
#define PICKER_WINDOW_WIDTH  0.50
#define PICKER_WINDOW_HEIGHT 0.40

// Instruction window
#define INSTR_WINDOW_WIDTH  0.25
#define INSTR_WINDOW_HEIGHT 0.65

// Memory window
#define MEM_WINDOW_WIDTH  0.2
#define MEM_WINDOW_HEIGHT 1

// Cache window
#define CACHE_WINDOW_WIDTH  (1 - (INSTR_WINDOW_WIDTH + MEM_WINDOW_WIDTH))   // Take up all the free remaining space
#define CACHE_WINDOW_HEIGHT 1

class GUI {
private:
    SDL_Window* window;
    SDL_GLContext gl_context;

    // Window sizes
    int windowHeight, windowWidth; 

    void renderInstructionWindow(Simulator* sim);
    void renderStatsWindow(Simulator* sim);
    void renderCacheWindow(Simulator* sim);
    void renderMemoryWindow(Simulator* sim);

public:
    GUI();
    ~GUI();
    SDL_Window* getWindow();
    void renderPicker(char configPath[MAX_PATH_LENGTH], char tracePath[MAX_PATH_LENGTH], bool* clickedLaunch);
    void renderWorkspace(Simulator* sim);
};

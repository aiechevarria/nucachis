#include "Misc.h"
#include "GUI.h"
#include "ParserConfig.h"
#include "ParserTrace.h"
#include "Simulator.h"

int main(int, char**) {
    // File paths for the trace and config
    char configPath[MAX_PATH_LENGTH] = "\0";
    char tracePath[MAX_PATH_LENGTH] = "\0";
    bool filesProvided = false;
    bool filesValidated = false;

    // Config and trace
    SimulatorConfig sc;
    MemoryOperation* ops;

    // Structures

    // Create a new GUI
    GUI* gui = new GUI();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    SDL_Window* window = gui->getWindow();

    // Main loop
    bool running = true;

    while (running) {
        // Mandatory SDL polling on each frame
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
        }

        // Start a new rendering frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Render the main window (workspace) on each frame
        if (!filesProvided) {
            gui->renderPicker(configPath, tracePath, &filesProvided);
        } else {
            // Parse the files the first time they are provided
            if (!filesValidated) {
                parseConfiguration(configPath, &sc);
                parseTrace(tracePath, ops);
                filesValidated = true;

                // TODO Build the simulator and memory hierarchy
            }

            // TODO check that both the config and trace are correct
            gui->renderWorkspace();
        }

        // Render the frame afterwards
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    return 0;
}

#include "Main.h"

/**
 * Parses the CLI arguments.
 * @param argc 
 * @param argv 
 * @return AppArgs The arguments packed in an AppArgs struct
 */
AppArgs parseArguments(int argc, char** argv) {
    AppArgs args;

    CLI::App app{APP_DESC};
    argv = app.ensure_utf8(argv);

    app.add_option("-c,--config", args.configFile, "Path to the configuration file")
       ->check(CLI::ExistingFile);
    app.add_option("-t,--trace", args.traceFile, "Path to the trace file")
       ->check(CLI::ExistingFile);
    app.add_flag("-g,--nogui", args.noGui, "Disable the GUI");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        std::exit(app.exit(e));
    }

    return args;
}

int main(int argc, char** argv) {
    // File paths for the trace and config
    char configPath[MAX_PATH_LENGTH] = "\0";
    char tracePath[MAX_PATH_LENGTH] = "\0";
    bool filesProvided = false;
    bool filesValidated = false;
    bool filesParsingError = false;

    // Config and trace
    SimulatorConfig sc;
    MemoryOperation* ops;

    // Structures
    Simulator* sim;
    AppArgs args = parseArguments(argc, argv);

    // Copy the config and trace files if they were provided as an argument
    if (!args.configFile.empty()) {
        strncpy(configPath, args.configFile.c_str(), MAX_PATH_LENGTH);
    }
    if (!args.traceFile.empty()) {
        strncpy(tracePath, args.traceFile.c_str(), MAX_PATH_LENGTH);
    }

    if (args.noGui) {
        // TODO Add headless run
    } else {
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


            // Display the file picker if no files have been provided
            if (!filesProvided) {
                gui->renderPicker(configPath, tracePath, &filesProvided);
                
                // If there was an error in the parsing operation, show the error
                if (filesParsingError) {
                    gui->renderError((char*) "Error parsing configuration or trace.\nCheck the console for more info.", &filesParsingError);
                }
            } else {
                // Parse the files the first time they are provided
                if (!filesValidated) {
                    // Parse the trace and make sure there are no fatal errors
                    if (parseConfiguration(configPath, &sc) != -2 &&
                        parseTrace(tracePath, ops, &sc.miscNumOperations) != -2) {
                        filesValidated = true;

                        sim = new Simulator(&sc, ops);
                    } else {
                        // If there were fatal errors, signal that an error should be shown and that the files are not ready
                        filesParsingError = true;
                        filesProvided = false;
                    }
                } else {
                    // Render the main window (workspace) on each frame once everything has been setup
                    gui->renderWorkspace(sim);
                }
            }

            // Render the frame afterwards
            ImGui::Render();
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(window);
        }
    }
    
    return 0;
}

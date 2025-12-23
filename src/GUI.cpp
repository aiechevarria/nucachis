#include "GUI.h"

GUI::GUI () {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    }

    // GL context setup
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window
    window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

GUI::~GUI () {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

}

SDL_Window* GUI::getWindow() {
    return window;
}

/**
 * Renders the instruction window.
 */
void GUI::renderInstructionWindow(Simulator* sim) {
    // Set a size and position based on the current workspace dimms
    ImVec2 windowSize(windowWidth * INSTR_WINDOW_WIDTH, windowHeight * INSTR_WINDOW_HEIGHT);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImVec2 windowPos(0, 0);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

    // Start the window disabling collapse
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Instruction Window", nullptr, window_flags);

    // Buttons
    if (ImGui::Button("Single Step")) {
        sim->singleStep();
    }
    ImGui::SameLine();
    if (ImGui::Button("Step All")) {
        sim->stepAll(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        sim->reset();
    }

    ImGui::End();
}


/**
 * Renders the cache window.
 */
void GUI::renderCacheWindow(Simulator* sim) {
    // Set a size and position based on the current workspace dimms
    ImVec2 windowSize(windowWidth * CACHE_WINDOW_WIDTH, windowHeight * CACHE_WINDOW_HEIGHT);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImVec2 windowPos((windowWidth * INSTR_WINDOW_WIDTH), 0);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

    // Start the window disabling collapse
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Cache Hierarchy", nullptr, window_flags);

    ImGui::Text("L1I");
    if (ImGui::BeginTable("L1I", 2, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Data");
        ImGui::TableHeadersRow();

        for (int row = 0; row < 5; ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("0x%X", 0x10 + row);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", row * 10);
        }

        ImGui::EndTable();
    }

    ImGui::Text("L1D");
    if (ImGui::BeginTable("L1D", 2, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Data");
        ImGui::TableHeadersRow();

        for (int row = 0; row < 5; ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("0x%X", 0x10 + row);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", row * 10);
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

/**
 * Renders the memory window.
 */
void GUI::renderMemoryWindow(Simulator* sim) {
    // Set a size and position based on the current workspace dimms
    ImVec2 windowSize(windowWidth * MEM_WINDOW_WIDTH, windowHeight * MEM_WINDOW_HEIGHT);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImVec2 windowPos(windowWidth * INSTR_WINDOW_WIDTH + windowWidth * CACHE_WINDOW_WIDTH, 0);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

    // Start the window disabling collapse
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Main Memory", nullptr, window_flags);

    if (ImGui::BeginTable("Memory table", 2, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Data");
        ImGui::TableHeadersRow();

        for (int row = 0; row < 5; ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("0x%X", 0x10 + row);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", row * 10);
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

/**
 * Renders the file picker.
 * @param configPath Pointer to a sufficiently large array of characters for the config path
 * @param tracePath Pointer to a sufficiently large array of characters for the trace path
 * @param clickedOk Pointer to a boolean. Toggled true when the user clicks the launch button
 */
void GUI::renderPicker(char configPath[MAX_PATH_LENGTH], char tracePath[MAX_PATH_LENGTH], bool* clickedLaunch) {
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // Set a size and position based on the current workspace dimms
    ImVec2 windowSize(windowWidth * PICKER_WINDOW_WIDTH, windowHeight * PICKER_WINDOW_HEIGHT);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImVec2 windowPos((windowWidth / 2 - windowWidth * PICKER_WINDOW_WIDTH / 2), (windowHeight / 2 - windowHeight * PICKER_WINDOW_HEIGHT / 2));
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::Begin("Welcome to NuCachis");
    ImGui::Text("Please, pick a config and trace file to start the simulation");

    ImGui::InputText("##ConfigPicker", configPath, MAX_PATH_LENGTH);
    ImGui::SameLine();

    if (ImGui::Button("Pick config")) {
        IGFD::FileDialogConfig config;
        config.path = '.';
		ImGuiFileDialog::Instance()->OpenDialog("ChooseConfigFile", "Choose Config File", ".ini", config);
    }

    ImGui::InputText("##TracePicker", tracePath, MAX_PATH_LENGTH);
    ImGui::SameLine();
    if (ImGui::Button("Pick trace")) {
        IGFD::FileDialogConfig trace;
        trace.path = '.';
		ImGuiFileDialog::Instance()->OpenDialog("ChooseTraceFile", "Choose Trace File", ".vca", trace);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Launch simulator")) {
        *clickedLaunch = true;
    }

    // File pickers
    if (ImGuiFileDialog::Instance()->Display("ChooseConfigFile")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            strncpy(configPath, ImGuiFileDialog::Instance()->GetFilePathName().c_str(), MAX_PATH_LENGTH);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseTraceFile")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            // std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            strncpy(tracePath, ImGuiFileDialog::Instance()->GetFilePathName().c_str(), MAX_PATH_LENGTH);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::End();
}

/**
 * Renders the main window (workspace) with all it's menus.
 * The trace and config should have previously been processed.
 */
void GUI::renderWorkspace(Simulator* sim) {
    // Always fetch the window size prior to re rendering it
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // Render all different parts of the GUI
    renderInstructionWindow(sim);
    renderCacheWindow(sim);
    renderMemoryWindow(sim);
}
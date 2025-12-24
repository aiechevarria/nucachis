#include "GUI.h"
#include "imgui.h"
#include <cstdio>

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

void GUI::drawCacheTable(CacheLine* cache, uint32_t numLines, char* label) {
    ImGui::Text("%s\n", label);

    // Display the instruction cache 
    if (ImGui::BeginTable(label, 9, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit, ImVec2(0.0f, 0.0f))) {
        ImGui::TableSetupColumn("L");
        ImGui::TableSetupColumn("S");
        ImGui::TableSetupColumn("W");
        ImGui::TableSetupColumn("D");
        ImGui::TableSetupColumn("V");
        ImGui::TableSetupColumn("1st Acc.");
        ImGui::TableSetupColumn("Last Acc.");
        ImGui::TableSetupColumn("# Acc.");
        ImGui::TableSetupColumn("Content");
        ImGui::TableHeadersRow();

        for (int i = 0; i < numLines; i++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%d", i);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%u", cache[i].set);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%u", cache[i].way);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%u", cache[i].dirty);
            ImGui::TableSetColumnIndex(4); ImGui::Text("%u", cache[i].valid);
            ImGui::TableSetColumnIndex(5); ImGui::Text("%u", cache[i].firstAccess);
            ImGui::TableSetColumnIndex(6); ImGui::Text("%u", cache[i].lastAccess);
            ImGui::TableSetColumnIndex(7); ImGui::Text("%u", cache[i].numberAccesses);
            ImGui::TableSetColumnIndex(8); ImGui::Text("%u %u %u %u", 0, 0,0,0);
        }

        ImGui::EndTable();
    }
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
 * @param sim Pointer to the simulator
 */
void GUI::renderCacheWindow(Simulator* sim) {
    char label[5] = "\0";               // Label for the columns

    // Set a size and position based on the current workspace dimms
    ImVec2 windowSize(windowWidth * CACHE_WINDOW_WIDTH, windowHeight * CACHE_WINDOW_HEIGHT);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImVec2 windowPos((windowWidth * INSTR_WINDOW_WIDTH), 0);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

    // Create the cache window
    if (ImGui::Begin("Cache Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse)) {
        // Get the number of caches
        uint8_t numCaches = sim->getNumCaches();

        // Create a table with as many columns as caches
        ImGuiTableFlags table_flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;
        if (ImGui::BeginTable("HierarchyTable", numCaches, table_flags, ImVec2(0.0f, 0.0f))) {
            // Store the table height for later
            float tableHeight = ImGui::GetItemRectSize().y;

            // Setup as many columns as caches with a label and a min width
            for (int i = 0; i < numCaches; i++) {
                char label[32];
                sprintf(label, "L%d Cache", i + 1);
                ImGui::TableSetupColumn(label, ImGuiTableColumnFlags_WidthFixed, MIN_CACHE_TABLE_WIDTH);
            }
            ImGui::TableHeadersRow();

            // Since this is a table, make a single row that has everything
            ImGui::TableNextRow();

            // Create a table for each cache and draw it's contents
            for (int i = 0; i < numCaches; i++) {
                Cache* cache = sim->getCache(i);

                ImGui::TableSetColumnIndex(i);

                // Generate a unique ID for the child window
                char childLabel[32];
                sprintf(childLabel, "Child_L%d", i);

                // ImVec2(0, 0) tells it to fill the width and height of the cell.
                ImGui::BeginChild(childLabel, ImVec2(0.0f, tableHeight - ImGui::GetStyle().ScrollbarSize), true);

                // Draw the content of the caches inside of the talbe
                if (cache->isCacheSplit()) {
                    drawCacheTable(cache->getInstCache(), cache->getCacheLines() / 2, (char*) "Instructions");
                    ImGui::Separator(); // Add a visual separator between split caches
                    drawCacheTable(cache->getDataCache(), cache->getCacheLines() / 2, (char*) "Data");
                } else {
                    drawCacheTable(cache->getDataCache(), cache->getCacheLines(), (char*) "Data");
                }

                ImGui::EndChild();
            }

            ImGui::EndTable();
        }
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

/**
 * Displays an error box with the provided message.
 * 
 * @param message The message to display
 * @param toggle Boolean variable to toggle when the close button is pressed
 */
void GUI::renderError(char* message, bool* toggle) {
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // Set a size and position based on the current workspace dimms
    ImVec2 windowSize(windowWidth * ERROR_WINDOW_WIDTH, windowHeight * ERROR_WINDOW_HEIGHT);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImVec2 windowPos((windowWidth / 2 - windowWidth * ERROR_WINDOW_WIDTH / 2), (windowHeight / 2 - windowHeight * ERROR_WINDOW_HEIGHT / 2));
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

    ImGui::Begin("Error");
    ImGui::Text(message);

    if (ImGui::Button("Ok")) {
        *toggle = !*toggle;
    }

    ImGui::End();
}
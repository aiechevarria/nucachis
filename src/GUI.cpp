#include "GUI.h"

ImVec4 colorVec[NUM_COLOR_NAMES] = {
    ImVec4(0.03f, 1.0f, 0.5f, 1.0f),    // COLOR_HIT
    ImVec4(0.9f, 0.05f, 0.25f, 1.0f),   // COLOR_MISS
    ImVec4(0.05f, 0.5f, 1.0f, 1.0f),    // COLOR_LOAD_FIRST
    ImVec4(0.05f, 0.8f, 1.0f, 1.0f),     // COLOR_LOAD_BURST
    ImVec4(1.0f, 0.65f, 0.0f, 1.0f),    // COLOR_WRITE_FIRST
    ImVec4(1.0f, 0.8f, 0.0f, 1.0f),     // COLOR_WRITE_BURST
    ImVec4(0.5f, 0.5f, 0.5f, 0.5f),     // COLOR_EXECUTE
    ImVec4(0.0f, 0.0f, 0.0f, 0.0f)      // COLOR_NONE
};

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

    // Set the style to dark
    ImGui::StyleColorsDark();

    // Load the images to the GPU
    logo = LoadImageFromHeader(logoData, logoWidth, logoHeight, true);

    // Init the scroll variables
    scrolledInstructions = false;
    scrolledMemory = false;
    for (int i = 0; i < MAX_CACHE_LEVELS; i++) {
        scrolledCache[i] = false;
    }

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
 * Decodes and loads the given image data to the GPU
 * @param data The data to decode 
 * @param width The width of the image
 * @param height The height of the image
 * @param setTaskbarIcon If true, also sets the image as the icon of the application in the OS' taskbar.
 * @return unsigned char* Pointer to the RGBA buffer
 */
GLuint GUI::LoadImageFromHeader(char* data, int width, int height, bool setTaskbarIcon) {
    unsigned char* rgbaData = (unsigned char*)malloc(width * height * 4);
    
    unsigned char* dest = rgbaData;
    char* readPointer = data;

    for (unsigned int i = 0; i < width * height; i++) {
        unsigned char pixel[3];
        HEADER_PIXEL(readPointer, pixel); // Macro from the H file. This is not very portable if there are multiple images
        
        dest[0] = pixel[0]; // R
        dest[1] = pixel[1]; // G
        dest[2] = pixel[2]; // B
        dest[3] = 255;      // A (Opaque)
        dest += 4;
    }

    // Set up the app's taskbar icon if chosen
    if (setTaskbarIcon) {
        // Create an SDL surface with the raw pixels
        SDL_Surface* iconSurface = SDL_CreateRGBSurfaceFrom(
            rgbaData, width, height, 32, width * 4,
            0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
        );

        // Set and free the icon
        SDL_SetWindowIcon(window, iconSurface);
        SDL_FreeSurface(iconSurface);
    }

    // Setup the texture
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering so the image doesn't look blurry
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload the pixels
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);

    // After loading the image to the GPU, the data can be freed from the CPU
    free(rgbaData);
    return image_texture;
}

/**
 * Centers the next ImGui element that gets rendered. 
 * @param itemWidth The width of the itme that will be rendered
 */
void GUI::centerNextItem(float itemWidth) {
    float availX = ImGui::GetContentRegionAvail().x;
    float pos = (availX - itemWidth) * 0.5f;
    if (pos > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + pos);
}

void GUI::drawCacheTable(CacheLine* cache, uint8_t id, uint32_t lineSizeWords, uint32_t numLines, char* label) {
    ImGui::Text("%s\n", label);

    // Display the instruction cache 
    if (ImGui::BeginTable(label, 9, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit, ImVec2(0.0f, 0.0f))) {
        ImGui::TableSetupColumn("L");
        ImGui::TableSetupColumn("S");
        ImGui::TableSetupColumn("W");
        ImGui::TableSetupColumn("D");
        ImGui::TableSetupColumn("V");
        ImGui::TableSetupColumn("1st Acc");
        ImGui::TableSetupColumn("Last Acc");
        ImGui::TableSetupColumn("# Acc");
        ImGui::TableSetupColumn("Content");
        ImGui::TableHeadersRow();

        for (int i = 0; i < numLines; i++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%d", i);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%u", cache[i].set);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%u", cache[i].way);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%u", cache[i].dirty);
            ImGui::TableSetColumnIndex(4); ImGui::Text("%u", cache[i].valid);
            ImGui::TableSetColumnIndex(5); (cache[i].firstAccess == -1) ? ImGui::Text("-") : ImGui::Text("%u", cache[i].firstAccess);
            ImGui::TableSetColumnIndex(6); (cache[i].lastAccess == -1) ? ImGui::Text("-") : ImGui::Text("%u", cache[i].lastAccess);
            ImGui::TableSetColumnIndex(7); (cache[i].numberAccesses == -1) ? ImGui::Text("-") : ImGui::Text("%u", cache[i].numberAccesses);
            ImGui::TableSetColumnIndex(8);

            for (int j = 0; j < lineSizeWords; j++) {
                ImGui::Text("%lu ", cache[i].content[j]);
                ImGui::SameLine();
            }

            // Apply color to the row if it has some style
            if (cache[i].lineColor != COLOR_NONE) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(colorVec[cache[i].lineColor]));

                // Scroll to that row once per cycle
                if (!scrolledCache[id]) {
                    ImGui::SetScrollHereY(0.5f);
                    scrolledCache[id] = true;
                }
            }
        }

        ImGui::EndTable();
    }
}

/**
 * Renders the instruction window.
 */
void GUI::renderInstructionWindow(Simulator* sim) {
    // Get the operations
    MemoryOperation** ops = sim->getOps();
    uint32_t numOps = sim->getNumOps();

    // Set a size and position based on the current workspace dimms
    ImVec2 windowSize(windowWidth * INSTR_WINDOW_WIDTH, windowHeight * INSTR_WINDOW_HEIGHT);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImVec2 windowPos(0, 0);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

    // Start the window disabling collapse
    ImGui::Begin("Instruction Window", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    // Buttons
    if (ImGui::Button("Single Step")) {
        resetScroll();
        sim->singleStep();
    }
    ImGui::SameLine();
    if (ImGui::Button("Step All")) {
        resetScroll();
        sim->stepAll(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        resetScroll();
        sim->reset();
    }

    ImGui::Separator();

    // Current cycle
    ImGui::Text("Current cycle: %u", cycle);

    ImGui::Separator();

    // Operation table
    if (ImGui::BeginTable("Operations", 5, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit, ImVec2(0.0f, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupColumn("B");
        ImGui::TableSetupColumn("Op");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Data");
        ImGui::TableHeadersRow();

        for (int i = 0; i < numOps; i++) {
            // Create an ID for the checkboxes
            char checkboxId[6];
            sprintf(checkboxId, "##C%d", i);

            // Highlight the current operation
            if (cycle != 0 && cycle == i) {
                ImU32 rowColor = ImGui::GetColorU32(colorVec[COLOR_EXECUTE]);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, rowColor);

                // Scroll to that row once per cycle
                if (!scrolledInstructions) {
                    ImGui::SetScrollHereY(0.5f);
                    scrolledInstructions = true;
                }
            }

            // Draw the table
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Checkbox(checkboxId, &ops[i]->hasBreakPoint);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%c", ops[i]->operation == LOAD ? 'L' : 'S');
            ImGui::TableSetColumnIndex(2); ImGui::Text("%c", ops[i]->isData ? 'D' : 'I');
            ImGui::TableSetColumnIndex(3); ImGui::Text("0x%lX", ops[i]->address);
            ImGui::TableSetColumnIndex(4); ops[i]->operation == STORE ? ImGui::Text("%lu", ops[i]->data[0]) : ImGui::Text("-");
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

/**
 * Renders the stats window.
 * @param sim Pointer to the simulator
 */
void GUI::renderStatsWindow(Simulator* sim) {
    ImVec2 windowSize(windowWidth * STATS_WINDOW_WIDTH, windowHeight * STATS_WINDOW_HEIGHT);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImVec2 windowPos(0, windowHeight * INSTR_WINDOW_HEIGHT);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    
    // Start the window disabling collapse
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Statistics", nullptr, window_flags);

    ImGui::Text("CPU:");
    ImGui::Text("\tTotal access time (s): %.4f", sim->getTotalAccessTime());
    cycle != 0 ? ImGui::Text("\tAverage memory access time (s): %.4f", sim->getTotalAccessTime() / (double) cycle) : ImGui::Text("\tAverage memory access time (ms): -");
    
    for (int i = 0; i < sim->getNumCaches(); i++) {
        Cache* cache = sim ->getCache(i);
        ImGui::Text("\nCache L%d:", i + 1);
        ImGui::Text("\tTotal accesses: %d", cache->getAccesses());
        ImGui::Text("\tHits: %d", cache->getHits());
        ImGui::Text("\tMisses: %d", cache->getMisses());
        cycle != 0 ? ImGui::Text("\tHit rate: %.1f%%", cache->getHits() / (double) cycle * 100) : ImGui::Text("\tHit rate: -");
        cycle != 0 ? ImGui::Text("\tMiss rate: %.1f%%", cache->getMisses() / (double) cycle * 100) : ImGui::Text("\tMiss rate: ");
    }

    ImGui::Text("\nMemory:");
    ImGui::Text("\tTotal accesses: %ld", sim->getMemory()->getAccessesBurst() + sim->getMemory()->getAccessesSingle());
    ImGui::Text("\tFirst word accesses: %ld", sim->getMemory()->getAccessesSingle());
    ImGui::Text("\tBurst accesses: %ld", sim->getMemory()->getAccessesBurst());

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
                    drawCacheTable(cache->getCache(true), i, cache->getLineSizeWords(),cache->getLines(), (char*) "Instructions");
                    ImGui::Separator(); // Visual separator line
                    drawCacheTable(cache->getCache(), i, cache->getLineSizeWords(), cache->getLines(), (char*) "Data");
                } else {
                    drawCacheTable(cache->getCache(), i, cache->getLineSizeWords(), cache->getLines(), (char*) "Data");
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
    MemoryLine* memory = sim->getMemory()->getMemory();
    uint64_t pageSize = sim->getMemory()->getPageSize();

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

        for (int i = 0; i < pageSize / sim->getWordWidth(); i++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("0x%lX", memory[i].address);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", memory[i].content);

            // Apply color to the row if it has some style
            if (memory[i].lineColor != COLOR_NONE) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(colorVec[memory[i].lineColor]));

                // Scroll to that row once per cycle
                if (!scrolledMemory) {
                    ImGui::SetScrollHereY(0.5f);
                    scrolledMemory = true;
                }
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

/**
 * Resets the scroll variables.
 */
void GUI::resetScroll() {
    scrolledInstructions = false;
    scrolledMemory = false;
    for (int i = 0; i < MAX_CACHE_LEVELS; i++) {
        scrolledCache[i] = false;
    }
}

/**
 * Renders the file picker.
 * @param configPath Pointer to a sufficiently large array of characters for the config path
 * @param tracePath Pointer to a sufficiently large array of characters for the trace path
 * @param freshLaunch If true, displays the app's logo and welcome screen
 * @param clickedLaunch Pointer to a boolean. Toggled true when the user clicks the launch button
 */
void GUI::renderPicker(char configPath[MAX_PATH_LENGTH], char tracePath[MAX_PATH_LENGTH], bool freshLaunch, bool* clickedLaunch) {
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // Set a size and position based on the current workspace dimms
    ImVec2 windowSize(windowWidth * PICKER_WINDOW_WIDTH, windowHeight * PICKER_WINDOW_HEIGHT);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
    ImVec2 windowPos((windowWidth / 2 - windowWidth * PICKER_WINDOW_WIDTH / 2), (windowHeight / 2 - windowHeight * PICKER_WINDOW_HEIGHT / 2));
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
    
    // Render the window
    ImGui::Begin("Welcome to NuCachis", nullptr, ImGuiWindowFlags_NoCollapse);

    if (freshLaunch) {
        // Center and draw the logo
        centerNextItem(logoWidth / 4);
        ImGui::Image((ImTextureID)(intptr_t)logo, ImVec2(logoWidth / 4, logoHeight / 4));

        centerNextItem(ImGui::CalcTextSize(APP_DESC).x);
        ImGui::Text(APP_DESC);
    }

    // Leave y pixels of vertical separation
    ImGui::Dummy(ImVec2(0.0f, 25.0f));
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

    ImGui::Separator();

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
    renderStatsWindow(sim);
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
    // Draw a red, 5 times larger than usual exclamation and reset the style after doing so 
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); 
    ImGui::SetWindowFontScale(5.0f); 
    ImGui::Text("!");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::Text(message);

    ImGui::Separator();
    if (ImGui::Button("Ok")) {
        *toggle = !*toggle;
    }

    ImGui::End();
}
// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "editor.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <cstdio>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS
// compilers. To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of
// Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "imgui/examples/libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char *description) { fprintf(stderr, "GLFW Error %d: %s\n", error, description); }

editor ed{};

extern "C" {
void setFile(const char *buf, size_t n, const char *name) {
    printf("Loaded file of size %zu bytes, name %s\n", n, name);
    ed.name = name;
    if (ed.masterVolume.FromBytes(buf, n) == 0) {
        ed.curveYLimit = 255;
        ed.curveElementCount = 121;
        ed.tableType = ETableType_VOLUME;
        ed.MasterToDouble();
        memcpy(
            ed.valuesyCopy,
            ed.masterVolumeValues[(int)ed.soundEffectOn][ed.MasterVolumeTableType][ed.MasterVolumeValueType],
            sizeof(ed.masterVolumeValues[(int)ed.soundEffectOn][ed.MasterVolumeTableType][ed.MasterVolumeValueType])
        );
        ed.cond = ImPlotCond_Always;
        ed.legendName = "Master Volume, " + std::string(name);

        return;
    }
    if (ed.masterVolumeDsd.FromBytes(buf, n) == 0) {
        ed.tableType = ETableType_DSD;
        ed.curveYLimit = (1 << 15) - 1; // 32767
        ed.curveElementCount = 121;
        ed.DSDToDouble();
        memcpy(
            ed.valuesyCopy,
            ed.masterVolumeDSDValues[ed.MasterVolumeDSDTableType],
            sizeof(ed.masterVolumeDSDValues[ed.MasterVolumeDSDTableType])
        );
        ed.cond = ImPlotCond_Always;
        ed.legendName = "DSD, " + std::string(name);
        return;
    }
    if (ed.toneControl.FromBytes(buf, n) == 0) {
        ed.tableType = ETableType_TONE;
        ed.curveYLimit = 255;
        ed.curveElementCount = 320;
        ed.ToneToDouble();

        memcpy(ed.valuesyCopy, ed.toneControlValues[ed.toneControlTableType], sizeof(ed.toneControlValues[ed.toneControlTableType]));
        ed.cond = ImPlotCond_Always;
        ed.legendName = "Tone Control, " + std::string(name);
        return;
    }
    emscripten_run_script("alert('unrecognized file format');");
}

const size_t dataSize = 1024 * 1024 * 90;
const unsigned char data[dataSize]{0};

unsigned char *saveFile(size_t *size) {
    switch (ed.tableType) {
    case ETableType_UNKNOWN:
        emscripten_run_script("alert('nothing to save');");
        break;
    case ETableType_VOLUME:
        memset((void *)&data, 0, dataSize);
        ed.masterVolume.ToBytes((void *)data, size);
        return (unsigned char *)&data;
        break;
    case ETableType_DSD:
        *size = sizeof ed.masterVolumeDsd.v;
        return (unsigned char *)&ed.masterVolumeDsd.v;
        break;
    case ETableType_TONE:
        *size = sizeof ed.toneControl.v;
        return (unsigned char *)&ed.toneControl.v;
        break;
    }

    return nullptr;
}

void setHeight(int v) { ed.height = v; }
void setWidth(int v) { ed.width = v; }
}

// Main code
int main(int, char **) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char *glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char *glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of
        // the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy
        // of the keyboard data. Generally you may always pass all inputs to dear imgui, and hide them from your application based on those
        // two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));

        //        ImGui::ShowDemoWindow();
        //        ImPlot::ShowDemoWindow();

        ImGui::SetNextWindowSize(ImVec2(1920, 1080));

        ImGui::Begin(
            "1",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoSavedSettings
        );

        ed.CurveEditor();

        ImGui::End();
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

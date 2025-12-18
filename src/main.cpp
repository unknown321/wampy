#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include <cstdio>

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <glad/glad.h>

#include <chrono>
#include <csignal>

#include "glm/glm/ext/matrix_transform.hpp"
#include "glm/glm/trigonometric.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "skin.h"
#include "util/util.h"

#include "config.h"
#include <unistd.h>
#include <vector>

#include "Version.h"
#include "shader.h"

#include "connector/hagoromo.h"
#include "mkpath.h"
#include "wampy.h"

#define IMGUI_WIDTH 800.0f
#define IMGUI_HEIGHT 480.0f

int GLFW_WIDTH = 800.0f;
int GLFW_HEIGHT = 800.0f;
ImVec2 screenMode = ImVec2((float)GLFW_WIDTH, (float)GLFW_HEIGHT);

#ifdef DESKTOP

#include "connector/mpd.h"

std::string dumpDir = "/tmp";
#else
std::string dumpDir = "/contents/wampy/log";

#endif

#define TARGET_FPS 24

static void glfw_error_callback(int error, const char *description) { DLOG("Glfw Error %d: %s\n", error, description); }

void drawCallback(const ImDrawList *dl, const ImDrawCmd *cmd) {
    //    printf("cliprect %f %f %f %f\n", cmd->ClipRect.x, cmd->ClipRect.y, cmd->ClipRect.z, cmd->ClipRect.w);
    auto newCmd = (ImDrawCmd *)cmd;

    ImVec2 clip_min((newCmd->ClipRect.x - 0) * 1, (newCmd->ClipRect.y - 0) * 1);
    ImVec2 clip_max((newCmd->ClipRect.z - 0) * 1, (newCmd->ClipRect.w - 0) * 1);

    auto sinr = -1;
    auto cosr = 0;
    auto x1 = (int)clip_min.x;
    auto y1 = (int)((float)IMGUI_WIDTH - clip_max.y);
    auto width = (int)(clip_max.x - clip_min.x);
    auto height = (int)(clip_max.y - clip_min.y);
    auto originX = 0;
    auto originY = 0;

    auto newX1 = ((x1 - originX) * cosr - (y1 - originY) * sinr) + originX;
    //    auto newX2 = ((width - originX) * cosr - (height - originY) * sinr) + originX;
    auto newY1 = ((x1 - originX) * sinr + (-y1 - originY) * cosr) + originY;
    //    auto newY2 = ((x2 - originX) * sinr + (-y2 - originY) * cosr) + originY;

    newCmd->ClipRect = ImVec4((float)newX1, (float)newY1, (float)height, (float)width);
    //    printf("cliprect %f %f %f %f\n", cmd->ClipRect.x, cmd->ClipRect.y, cmd->ClipRect.z, cmd->ClipRect.w);
    glScissor(newX1, newY1, height, width);

    auto sp = (GLuint)(long)cmd->UserCallbackData;
    glUseProgram(sp);

    glUniform1i(glGetUniformLocation(sp, "Texture"), 0);

    glm::mat4 Projection = glm::ortho((float)-GLFW_WIDTH, 0.0f, (float)GLFW_HEIGHT, 0.0f);
    glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    //    glm::mat4 ViewRotateX = glm::rotate( ViewTranslate, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f) );
    //    glm::mat4 ViewRotateY = glm::rotate( ViewRotateX, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f) );
    glm::mat4 ViewRotateZ = glm::rotate(ViewTranslate, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 View = ViewRotateZ;

    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

    glm::mat4 MVP = Projection * View * Model;
    glm::mat4 tMVP = glm::translate(MVP, glm::vec3(0, (float)GLFW_WIDTH - IMGUI_HEIGHT, 0));
    //    glm::mat4 tMVP = glm::translate(MVP, glm::vec3(0, 0, 0));
    GLint uniTrans = glGetUniformLocation(sp, "ProjMtx");
    glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(tMVP));

    //    auto color = (GLuint) glGetUniformLocation(sp, "Color");
    //    glUniform4f((GLint) color, 1.0f, 1.0f, 0.0f, 1.0f);
    //
    //    auto uv = (GLuint) glGetUniformLocation(sp, "UV");
    //    glUniform2f((GLint) uv, 0.01f, 0.5f);
}

GLFWwindow *CreateWindow() {
    glfwSetErrorCallback(glfw_error_callback);
    const char *title;
#ifdef DESKTOP
    DLOG("running on desktop\n");
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
    title = "Arandr";
#else
    DLOG("running on mobile\n");
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_NULL);
    title = "";
#endif

    if (!glfwInit()) {
        DLOG("glfw init failed\n");
        return nullptr;
    }

    // Decide GL+GLSL versions
    // const char *glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); // OpenGL ES 2.0 is the minimal version with shader support
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#ifdef DESKTOP
    GLFWmonitor *monitor = nullptr;
#else
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
#endif

    DLOG("monitor is %p\n", monitor);
    if (monitor != nullptr) {
        auto vmode = glfwGetVideoMode(monitor);
        DLOG("mode: %dx%d\n", vmode->width, vmode->height);
        GLFW_WIDTH = vmode->width;
        GLFW_HEIGHT = vmode->width;
        screenMode.x = (float)vmode->width;
        screenMode.y = (float)vmode->height;
    }

    GLFWwindow *window = glfwCreateWindow(GLFW_WIDTH, GLFW_HEIGHT, title, monitor, nullptr);
    if (window == nullptr) {
        DLOG("failed to create window\n");
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    int version = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (version == 0) {
        DLOG("glad failed\n");
        return nullptr;
    }
    DLOG("version: %d\n", version);

    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *glVersion = glGetString(GL_VERSION);
    const GLubyte *extensions = glGetString(GL_EXTENSIONS);
    DLOG("renderer %s; vendor %s; version %s\n", renderer, vendor, glVersion);
    DLOG("GL extensions: %s\n", extensions);
    int textureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &textureSize); // 4096
    DLOG("texture size %d\n", textureSize);

    return window;
}

#define ACTION_RELEASE 0
#define ACTION_PRESS 1

#ifdef DESKTOP
bool render = true;
#else
bool render = false;
#endif
bool hold_toggled = false;
bool power_pressed = false;
int hold_value = ACTION_RELEASE;
struct timespec tw = {0, 300000000};

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_HOLD) {
        hold_toggled = true;
        hold_value = action;
        if (action == ACTION_PRESS) {
            render = true;
        } else {
            render = false;
        }
    }

    if (key == GLFW_KEY_POWER) {
        if (action == ACTION_RELEASE) {
            DLOG("power press\n");
            power_pressed = true;
        }
    }

    if (render) {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    // touchscreen events are not written if file is not in use (read)
    // if there are no applications reading from it, there will be no events
    // you can `cat /dev/input/event1` if hagoromo is off

    //    DLOG("%f %f\n", xpos, ypos);
    if (render) {
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    //    DLOG("%d %d\n", action, mods);
    if (render) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    }
}

// used for profiling
void my_handler(int s) {
    render = false;
    sleep(1);
    DLOG("signal %d, exiting\n", s);
    exit(0);
}

void setupProfiling() {
    struct sigaction sigUSR1Handler{};

    sigUSR1Handler.sa_handler = my_handler;
    sigemptyset(&sigUSR1Handler.sa_mask);
    sigUSR1Handler.sa_flags = 0;

    sigaction(SIGUSR1, &sigUSR1Handler, nullptr);
    DLOG("pid: %d\n", getpid());
}

int main(int, char **) {
    DLOG("starting, commit %s\n", SOFTWARE_VERSION);
    DLOG("\n    wampy  Copyright (C) 2024-2025  unknown321\n"
         "    This program comes with ABSOLUTELY NO WARRANTY; for details see Settings->Misc->License.\n"
         "    This is free software, and you are welcome to redistribute it\n"
         "    under certain conditions; see Settings->Misc->License for details.\n");

    std::string model = "unknown model";
    bool isWalkmanOne{};
    getModel(&model, &isWalkmanOne);
    DLOG("device model: %s, walkmanOne: %d\n", model.c_str(), isWalkmanOne);

    recoverDumps(dumpDir);
    auto config = AppConfig::AppConfig();
    config.Load();
    if (config.badBoots >= 3) {
        DLOG("3 reboots in a row, exiting\n");
        exit(1);
    }

    config.badBoots += 1;
    config.Save();

    setupProfiling();

    std::string socket{};
    Connector *connector;
#ifdef DESKTOP
    if (config.forceConnector == "hagoromo") {
        connector = new Hagoromo::HagoromoConnector();
        socket = WAMPY_SOCKET;
        hold_toggled = true;
        hold_value = 1;
        auto v = (Hagoromo::HagoromoConnector *)connector;
        v->touchscreenStaysOFF = &config.features.touchscreenStaysOFF;
        v->featureBigCover = &config.features.bigCover;
        v->featureShowTime = &config.features.showTime;
        v->featureLimitVolume = &config.features.limitVolume;
        v->featureEqPerSong = &config.features.eqPerSong;
        v->visualizerWinampBands = &config.winamp.visualizerWinampBands;
        v->visualizerEnabled = &config.winamp.visualizerEnable;
        v->masterVolumePath = config.volumeTables.MasterVolumeTable;
        v->masterVolumeDSDPath = config.volumeTables.MasterVolumeTableDSD;
        v->toneControlPath = config.volumeTables.ToneControl;
        v->filters = &config.filters;
        v->controlFilters = &config.controlFilters;
    } else {
        connector = new MPD::MPDConnector();
        connector->address = config.MPDSocketPath.c_str();
        socket = config.MPDSocketPath;
    }
#else
    connector = new Hagoromo::HagoromoConnector();
    auto v = (Hagoromo::HagoromoConnector *)connector;
    v->touchscreenStaysOFF = &config.features.touchscreenStaysOFF;
    v->featureBigCover = &config.features.bigCover;
    v->featureShowTime = &config.features.showTime;
    v->featureLimitVolume = &config.features.limitVolume;
    v->featureEqPerSong = &config.features.eqPerSong;
    v->visualizerWinampBands = &config.winamp.visualizerWinampBands;
    v->visualizerEnabled = &config.winamp.visualizerEnable;
    v->masterVolumePath = config.volumeTables.MasterVolumeTable;
    v->masterVolumeDSDPath = config.volumeTables.MasterVolumeTableDSD;
    v->toneControlPath = config.volumeTables.ToneControl;
    v->filters = &config.filters;
    v->controlFilters = &config.controlFilters;

    socket = WAMPY_SOCKET;

#endif
    connector->render = &render;

    GLFWwindow *window = CreateWindow();
    assert(window != nullptr);

    if (config.debug) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(GlMessageCallback, 0);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    GLuint shaderProgram = ShaderProgram();

    ImVec2 imguiWindowPos = ImVec2(0, 0);
    ImVec2 imguiWindowSize = ImVec2(IMGUI_WIDTH, IMGUI_HEIGHT);

    MyMagick::InitMagick();

    auto skin = Skin();
    skin.isWalkmanOne = isWalkmanOne;
    skin.render = &render;
#ifdef DESKTOP
    skin.WithWinampSkinDir("../skins/");
    skin.WithCassetteReelDir("../cassette/cassetteunpacker/res/reel/");
    skin.WithCassetteTapeDir("../cassette/cassetteunpacker/res/tape/");

    skin.WithMasterVolumeTableDirs("../ss/system/master_volume/");
    skin.WithMasterVolumeTableDirs("../ss/user/master_volume/");
    skin.WithMasterVolumeTableDSDDirs("../ss/system/master_volume_dsd/");
    skin.WithMasterVolumeTableDSDDirs("../ss/user/master_volume_dsd/");
    skin.WithToneControlTableDirs("../ss/system/tone_control/");
    skin.WithToneControlTableDirs("../ss/user/tone_control/");

    skin.radioAvailable = true;
#else
    skin.WithWinampSkinDir("/system/vendor/unknown321/usr/share/wampy/skins/winamp/");
    skin.WithWinampSkinDir("/contents/wampy/skins/winamp/");

    skin.WithCassetteReelDir("/system/vendor/unknown321/usr/share/wampy/skins/cassette/reel/");
    skin.WithCassetteReelDir("/contents/wampy/skins/cassette/reel/");

    skin.WithCassetteTapeDir("/system/vendor/unknown321/usr/share/wampy/skins/cassette/tape/");
    skin.WithCassetteTapeDir("/contents/wampy/skins/cassette/tape/");

    skin.WithMasterVolumeTableDSDDirs("/system/vendor/unknown321/usr/share/wampy/sound_settings/master_volume_dsd/");
    skin.WithMasterVolumeTableDSDDirs("/contents/wampy/sound_settings/master_volume_dsd/");

    skin.WithMasterVolumeTableDirs("/system/vendor/unknown321/usr/share/wampy/sound_settings/master_volume/");
    skin.WithMasterVolumeTableDirs("/contents/wampy/sound_settings/master_volume/");

    skin.WithToneControlTableDirs("/system/vendor/unknown321/usr/share/wampy/sound_settings/tone_control/");
    skin.WithToneControlTableDirs("/contents/wampy/sound_settings/tone_control/");

    skin.radioAvailable = exists("/dev/radio0");
#endif

    skin.hold_toggled = &hold_toggled;
    skin.power_pressed = &power_pressed;
    skin.hold_value = &hold_value;
    skin.config = &config;
    skin.windowOffset = &imguiWindowPos;
    skin.screenMode = screenMode;
    skin.windowSize = imguiWindowSize;

    auto opt = W1::WalkmanOneOptions{};
    W1::ParseSettings(&opt);
    skin.walkmanOneOptions = opt;

    skin.connector = connector;
    skin.winamp.connector = connector;
    skin.cassette.connector = connector;

    skin.winamp.WithConfig(&config.winamp);
    skin.cassette.WithConfig(&config.cassette);
    skin.digitalClock.WithConfig(&config.digitalClock);

    connector->clients.push_back(&skin.winamp);
    connector->clients.push_back(&skin.cassette);

    skin.RefreshWinampSkinList();
    skin.RefreshCassetteTapeReelLists();
    skin.RefreshMasterVolumeTableFiles();
    skin.RefreshMasterVolumeTableDSDFiles();
    skin.RefreshToneControlFiles();
    skin.PreprocessTableFilenames();

    skin.ReadLicense();
    skin.ReadQR();
    skin.GetLogsDirSize();
    connector->soundSettings.Start();
    connector->soundSettingsFw.Start();
    skin.Load();

    if (socket.empty()) {
        DLOG("no socket path provided\n");
        exit(1);
    }

    auto now = std::time(nullptr);
    while (true) {
        struct stat st{};
        if (stat(socket.c_str(), &st) == 0) {
            break;
        } else {
            nanosleep(&tw, nullptr);
            DLOG("waiting for socket %s\n", socket.c_str());
        }

        if (std::time(nullptr) - now > WAMPY_TIMEOUT_SECONDS) {
            DLOG("wampy socket timeout, %s\n", socket.c_str());
            createDump();
            exit(1);
        }
    }

    connector->Start();
    // NOLINTBEGIN
    std::srand(std::time(nullptr)); // cassette rand initialization
    // NOLINTEND

    DLOG("start\n");

    config.badBoots = 0;
    config.Save();

#ifndef DESKTOP
    auto events = []() { glfwPollEvents(); };
    std::thread f(events);
    f.detach();

    mkpath("/contents/wampy/fonts/", 0777);
    mkpath("/contents/wampy/skins/cassette", 0777);
    mkpath("/contents/wampy/skins/winamp/", 0777);
    mkpath("/contents/wampy/skins/cassette/reel/", 0777);
    mkpath("/contents/wampy/skins/cassette/tape/", 0777);
    mkpath("/contents/wampy/sound_settings/master_volume_dsd/", 0777);
    mkpath("/contents/wampy/sound_settings/master_volume/", 0777);
    mkpath("/contents/wampy/sound_settings/tone_control/", 0777);
#endif

    int display_w, display_h;
    auto lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
#ifdef DESKTOP
        glfwPollEvents();
#endif
        skin.KeyHandler();

        if (!render) {
            nanosleep(&tw, nullptr);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(imguiWindowPos);

        //        ImGui::ShowDemoWindow();
        //        ImPlot::ShowDemoWindow();

        ImGui::SetNextWindowSize(imguiWindowSize);

        ImGui::Begin("1", nullptr, window_flags);

#ifndef DESKTOP
        ImGui::GetBackgroundDrawList()->AddCallback(drawCallback, (void *)shaderProgram);
#endif
        skin.Draw();

        //        DrawWindowRects();

        ImGui::End();

        // Rendering
        ImGui::Render();

        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        skin.LoadUpdatedSkin();

        if (config.limitFPS) {
            // https://github.com/glfw/glfw/issues/1308
            while (glfwGetTime() < lastTime + (1.0 / TARGET_FPS)) {
            }
            lastTime += 1.0 / TARGET_FPS;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
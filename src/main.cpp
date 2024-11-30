#include <cstdio>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions

#include <chrono>
#include <csignal>

#include "glm/glm/trigonometric.hpp"
#include "glm/glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui_internal.h"

#include "skin.h"
#include "util/util.h"

#include "config.h"
#include <vector>
#include <unistd.h>
#include <thread>

#include "shader.h"

#ifdef DESKTOP

#include "connector/mpd.h"

#define WIDTH 800.0f
#define HEIGHT 480.0f
#else

#include "connector/hagoromo.h"

#define WIDTH 480.0f
#define HEIGHT 800.0f
#endif

#define TARGET_FPS 24

static void glfw_error_callback(int error, const char *description) {
    DLOG("Glfw Error %d: %s\n", error, description);
}

void drawCallback(const ImDrawList *dl, const ImDrawCmd *cmd) {
    auto sp = (GLuint) (long) cmd->UserCallbackData;
    glUseProgram(sp);

    glUniform1i(glGetUniformLocation(sp, "Texture"), 0);

    glm::mat4 Projection = glm::ortho(-WIDTH, 0.0f, HEIGHT, 0.0f);
    glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

//    glm::mat4 ViewRotateX = glm::rotate( ViewTranslate, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f) );
//    glm::mat4 ViewRotateY = glm::rotate( ViewRotateX, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f) );
    glm::mat4 ViewRotateZ = glm::rotate(ViewTranslate, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 View = ViewRotateZ;

    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

    glm::mat4 MVP = Projection * View * Model;
    glm::mat4 tMVP = glm::translate(MVP, glm::vec3(0, 0, 0));
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#ifndef DESKTOP
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
#else
    GLFWmonitor* monitor = nullptr;
#endif

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, title, monitor, nullptr);
    if (window == nullptr) {
        DLOG("failed to create window\n");
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    int version = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    if (version == 0) {
        DLOG("glad failed\n");
        return nullptr;
    }

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

    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
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

SkinList skinList{};
SkinList reelList{};
SkinList tapeList{};

int main(int, char **) {
    auto config = AppConfig::AppConfig();
    config.Load();
    setupProfiling();

#ifdef DESKTOP
    auto connector = Player::MPDConnector();
    connector.address = config.MPDSocketPath.c_str();
    listdir("../skins/", &skinList, ".wsz");
    listdirs("/media/ssd/dev/nw/cassetteunpacker/res/reel/", &reelList);
    listdirs("/media/ssd/dev/nw/cassetteunpacker/res/tape/", &tapeList);
#else
    auto connector = Player::HagoromoConnector();
    listdir("/system/vendor/unknown321/usr/share/skins/winamp/", &skinList, ".wsz");
    listdir("/contents/skins/winamp/", &skinList, ".wsz");

    listdirs("/system/vendor/unknown321/usr/share/skins/cassette/reel/", &reelList);
    listdirs("/contents/skins/cassette/reel/", &reelList);

    listdirs("/system/vendor/unknown321/usr/share/skins/cassette/tape/", &tapeList);
    listdirs("/contents/skins/cassette/tape/", &tapeList);
#endif
    connector.render = &render;

    GLFWwindow *window = CreateWindow();
    assert(window != nullptr);

    if (config.debug.enabled) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(GlMessageCallback, 0);
    }

    const GLubyte *vendor = glGetString(GL_VENDOR); // Returns the vendor
    const GLubyte *renderer = glGetString(GL_RENDERER); // Returns a hint to the model
    DLOG("running on %s; %s\n", renderer, vendor);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollWithMouse |
                                    ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    GLuint shaderProgram = ShaderProgram();

    ImVec2 pos = ImVec2(0, 0);
    ImVec2 size = ImVec2(WIDTH, HEIGHT);
#ifndef DESKTOP
    size = ImVec2(HEIGHT, HEIGHT);
#endif

    MyMagick::InitMagick();

    auto skin = Skin();
    skin.render = &render;
    skin.skinList = &skinList;
    skin.reelList = &reelList;
    skin.tapeList = &tapeList;
    skin.hold_toggled = &hold_toggled;
    skin.power_pressed = &power_pressed;
    skin.hold_value = &hold_value;
    skin.config = &config;

    skin.connector = &connector;
    skin.winamp.connector = &connector;
    skin.cassette.connector = &connector;

    skin.winamp.fontRanges = &config.fontRanges;
    skin.cassette.fontRanges = &config.fontRanges;


    if (config.features.bigCover) {
        connector.FeatureBigCover(config.features.bigCover);
    }

    if (config.features.showTime) {
        connector.FeatureShowTime(config.features.showTime);
    }

    connector.Start();
    skin.Load();

    DLOG("start\n");


#ifndef DESKTOP
    auto events = []() {
        glfwPollEvents();
    };
    std::thread f(events);
    f.detach();
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

        ImGui::SetNextWindowPos(pos);

//        ImGui::ShowDemoWindow();

        ImGui::SetNextWindowSize(size);

        ImGui::Begin("1", nullptr, window_flags);

#ifndef DESKTOP
        ImGui::GetBackgroundDrawList()->AddCallback(drawCallback, (void *) shaderProgram);
        auto w = ImGui::GetCurrentWindow();
        w->OuterRectClipped = ImRect(ImVec2(0, 0), ImVec2(HEIGHT, WIDTH));
        ImGui::PushClipRect(ImVec2(0, 0), ImVec2(HEIGHT, HEIGHT), false);
#endif
        skin.Draw();

//        DrawWindowRects();

        ImGui::End();

        // Rendering
        ImGui::Render();

        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        skin.Load();

//        https://github.com/glfw/glfw/issues/1308
        while (glfwGetTime() < lastTime + (1.0 / TARGET_FPS)) {}
        lastTime += 1.0 / TARGET_FPS;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
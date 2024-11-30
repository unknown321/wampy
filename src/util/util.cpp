#include <sys/stat.h>
#include "util.h"
#include "../wstring.h"
#include "imgui_internal.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "GLFW/glfw3.h"
#include "imgui_impl_glfw.h"


void
listdir(const char *dirname, std::vector<directoryEntry> *list, const std::string &extension) {
    DIR *d;
    struct dirent *dir;
    d = opendir(dirname);
    if (d) {
        while ((dir = readdir(d)) != nullptr) {
            if (dir->d_type != DT_REG) {
                continue;
            }

            const char *ext = strrchr(dir->d_name, '.');
            if ((!ext) || (ext == dir->d_name))
                continue;
            else {
                if (strcmp(ext, extension.c_str()) == 0) {
                    auto a = directoryEntry{};
                    a.fullPath = std::string(dirname) + std::string(dir->d_name);
                    a.name = std::string(dir->d_name);
                    list->emplace_back(a);
                }
            }
        }
        closedir(d);
    }
}


void listdirs(const char *dirname, std::vector<directoryEntry> *list) {
    DIR *d;
    struct dirent *dir;
    d = opendir(dirname);
    if (d) {
        while ((dir = readdir(d)) != nullptr) {
            if (dir->d_type != DT_DIR) {
                continue;
            }

            if (dir->d_name[0] == '.') {
                continue;
            }

            auto a = directoryEntry{};
            a.fullPath = std::string(dirname) + std::string(dir->d_name);
            a.name = std::string(dir->d_name);
            list->emplace_back(a);
        }
        closedir(d);
    }
}

void UnloadTexture(ImTextureID textureID) {
//    DLOG("unload %p\n", textureID);
    if (textureID != nullptr) {
        auto g = (GLuint) *(unsigned int *) &textureID;
        glDeleteTextures(1, &g);
    }
}

void UnloadTexture(GLuint textureID) {
//    DLOG("unload %d\n", textureID);
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}

// breaks in thread
bool LoadTextureFromMagic(unsigned char *data, GLuint *out_texture, int dstWidth, int dstHeight) {
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 dstWidth, dstHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    *out_texture = image_texture;

    return true;
}


ImU32 colorToImCol32(const std::string &color) {
    unsigned int i[4]{};
    sscanf(color.c_str(), "#%02X%02X%02X", &i[0], &i[1], &i[2]);
    i[3] = 255;
    return IM_COL32(i[0], i[1], i[2], i[3]);
}

std::string parseColor(const std::string &in) {
    std::string color = in;

    if (color.substr(0, 1) != "#") {
        color = "#" + color;
    }

    if (color[color.length() - 1] == '\n') {
        color.erase(color.length() - 1);
    }

    if (color[color.length() - 1] == '\r') {
        color.erase(color.length() - 1);
    }

    while (color.length() < 4) {
        auto c = color.substr(color.length() - 1, 1);
        color.append(c);
    }

    return color;
}

std::vector<std::string> split(const std::string &s, const std::string &delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    auto v = s.substr(pos_start);
    res.push_back(v);
    return res;
}


std::string join(const std::vector<std::string> &v, int start) {
    std::string res;
    for (int i = start; i < v.size(); i++) {
        res += v[i];
    }

    return res;
}

void printFPS() {
    auto io = ImGui::GetIO();
//    ImGui::SetCursorPos(ImVec2(0, 460));
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
}

std::string CalculateTextWidth(std::string text, float maxWidth) {
    if (text.empty()) {
        return "";
    }

    if (ImGui::CalcTextSize(text.c_str()).x > maxWidth) {
        while (ImGui::CalcTextSize(text.c_str()).x > maxWidth) {
            int l = utfLen(text);
            char res[text.length() - 1];
            utfCut(text, l - 1, res);
            text = std::string(res);
        }

        text += "..";
    }

    return text;
}

enum {
    WRT_OuterRect,
    WRT_OuterRectClipped,
    WRT_InnerRect,
    WRT_InnerClipRect,
    WRT_WorkRect,
    WRT_Content,
    WRT_ContentIdeal,
    WRT_ContentRegionRect,
    WRT_Count
}; // Windows Rect Type
const char *wrt_rects_names[WRT_Count] = {"OuterRect", "OuterRectClipped", "InnerRect", "InnerClipRect", "WorkRect",
                                          "Content", "ContentIdeal", "ContentRegionRect"};

ImRect GetWindowRect(ImGuiWindow *window, int rect_type) {
    if (rect_type == WRT_OuterRect) { return window->Rect(); }
    else if (rect_type == WRT_OuterRectClipped) { return window->OuterRectClipped; }
    else if (rect_type == WRT_InnerRect) { return window->InnerRect; }
    else if (rect_type == WRT_InnerClipRect) { return window->InnerClipRect; }
    else if (rect_type == WRT_WorkRect) { return window->WorkRect; }
    else if (rect_type == WRT_Content) {
        ImVec2 min = window->InnerRect.Min - window->Scroll + window->WindowPadding;
        return ImRect(min, min + window->ContentSize);
    } else if (rect_type == WRT_ContentIdeal) {
        ImVec2 min = window->InnerRect.Min - window->Scroll + window->WindowPadding;
        return ImRect(min, min + window->ContentSizeIdeal);
    } else if (rect_type == WRT_ContentRegionRect) { return window->ContentRegionRect; }
    IM_ASSERT(0);
    return ImRect();
}

void DrawWindowRects() {
    auto g = ImGui::GetCurrentContext();
    for (ImGuiWindow *window: g->Windows) {
        if (!window->WasActive)
            continue;
        ImDrawList *draw_list = ImGui::GetForegroundDrawList(window);
        ImRect r = GetWindowRect(window, 1);
        draw_list->AddRect(r.Min, r.Max, IM_COL32(255, 0, 128, 255));
        if (!(window->Flags & ImGuiWindowFlags_ChildWindow)) {
            char buf[32];
            ImFormatString(buf, IM_ARRAYSIZE(buf), "%d", window->BeginOrderWithinContext);
            float font_size = 12;
            draw_list->AddRectFilled(window->Pos, window->Pos + ImVec2(font_size, font_size),
                                     IM_COL32(200, 100, 100, 255));
            draw_list->AddText(window->Pos, IM_COL32(255, 255, 255, 255), buf);
        }
    }
}

glm::mat4 rotAroundPoint(float deg, const glm::vec3 &origin, const glm::vec3 &axis) {
    glm::mat4 t1 = glm::translate(glm::mat4(1), -origin);
    glm::mat4 rot = glm::rotate(glm::mat4(1), glm::radians(deg), axis);
    glm::mat4 t2 = glm::translate(glm::mat4(1), origin);
    return t2 * rot * t1;
}

bool IsMounted() {
#ifdef DESKTOP
    return false;
#endif
    struct stat sb{};
    return (stat("/contents/MUSIC/", &sb) != 0);
}
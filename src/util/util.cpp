#include "util.h"
#include "../wstring.h"
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"
#include <algorithm>
#include <fstream>
#include <sys/stat.h>

bool dirComp(const directoryEntry &a, const directoryEntry &b) { return b.name >= a.name; }

void listdir(const char *dirname, std::vector<directoryEntry> *list, const std::string &extension) {
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

    std::sort(list->begin(), list->end(), dirComp);
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

    std::sort(list->begin(), list->end(), dirComp);
}

void UnloadTexture(ImTextureID textureID) {
    //    DLOG("unload %d\n", (int *)textureID);
    if (textureID != nullptr) {
        auto g = (GLuint) * (unsigned int *)&textureID;
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
                    GL_CLAMP_TO_EDGE);                                   // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dstWidth, dstHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

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
    ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
}

void CropTextToWidth(char *text, ImFont *font, float fontSize, float maxWidth) {
    const char *remaining;
    font->CalcTextSizeA(fontSize, maxWidth, -1.0f, text, nullptr, &remaining);
    if (strlen(remaining) > 0) {
        auto r = (char *)remaining;
        r[0] = '.';
        r[1] = '.';
        r[2] = '\0';
    }
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
const char *wrt_rects_names[WRT_Count] = {
    "OuterRect", "OuterRectClipped", "InnerRect", "InnerClipRect", "WorkRect", "Content", "ContentIdeal", "ContentRegionRect"};

ImRect GetWindowRect(ImGuiWindow *window, int rect_type) {
    if (rect_type == WRT_OuterRect) {
        return window->Rect();
    } else if (rect_type == WRT_OuterRectClipped) {
        return window->OuterRectClipped;
    } else if (rect_type == WRT_InnerRect) {
        return window->InnerRect;
    } else if (rect_type == WRT_InnerClipRect) {
        return window->InnerClipRect;
    } else if (rect_type == WRT_WorkRect) {
        return window->WorkRect;
    } else if (rect_type == WRT_Content) {
        ImVec2 min = window->InnerRect.Min - window->Scroll + window->WindowPadding;
        return ImRect(min, min + window->ContentSize);
    } else if (rect_type == WRT_ContentIdeal) {
        ImVec2 min = window->InnerRect.Min - window->Scroll + window->WindowPadding;
        return ImRect(min, min + window->ContentSizeIdeal);
    } else if (rect_type == WRT_ContentRegionRect) {
        return window->ContentRegionRect;
    }
    IM_ASSERT(0);
    return ImRect();
}

void DrawWindowRects() {
    auto g = ImGui::GetCurrentContext();
    for (ImGuiWindow *window : g->Windows) {
        if (!window->WasActive)
            continue;
        ImDrawList *draw_list = ImGui::GetForegroundDrawList(window);
        ImRect r = GetWindowRect(window, 1);
        draw_list->AddRect(r.Min, r.Max, IM_COL32(255, 0, 128, 255));
        if (!(window->Flags & ImGuiWindowFlags_ChildWindow)) {
            char buf[32];
            ImFormatString(buf, IM_ARRAYSIZE(buf), "%d", window->BeginOrderWithinContext);
            float font_size = 12;
            draw_list->AddRectFilled(window->Pos, window->Pos + ImVec2(font_size, font_size), IM_COL32(200, 100, 100, 255));
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
    struct stat sb {};
    return (stat("/contents/MUSIC/", &sb) != 0);
}

static int do_mkdir(const char *path, mode_t mode) {
    struct stat st {};
    int status = 0;

    if (stat(path, &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        status = -1;
    }

    return (status);
}

int mkpath(const char *path, mode_t mode) {
    char *pp;
    char *sp;
    int status;
    char *copypath = strdup(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != nullptr) {
        if (sp != pp) {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir(path, mode);
    free(copypath);
    return (status);
}

void recoverDumps(const std::string &outdir) {
    struct stat sb {};
    std::string corePath = "/var/log/core......gz";
    std::string hdumpstatePath = "/var/log/hdumpstate......log";
    std::string out;

    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), ".%Y-%m-%d_%H.%M.%S", timeinfo);
    auto t = std::string(buffer);

    if (stat(corePath.c_str(), &sb) == 0) {
        mkpath(outdir.c_str(), 0755);
        out = outdir + "/core......gz" + t;
        std::ifstream src(corePath, std::ios::binary);
        std::ofstream dst(out, std::ios::binary);

        dst << src.rdbuf();
        dst.close();
        src.close();

        DLOG("recovered core file to %s\n", out.c_str());

        std::remove(corePath.c_str());
    }

    if (stat(hdumpstatePath.c_str(), &sb) == 0) {
        mkpath(outdir.c_str(), 0755);
        out = outdir + "/hdumpstate......log" + t;
        std::ifstream src(corePath, std::ios::binary);
        std::ofstream dst(out, std::ios::binary);

        dst << src.rdbuf();
        dst.close();
        src.close();

        DLOG("recovered hdumpstate log to %s\n", out.c_str());

        std::remove(hdumpstatePath.c_str());
    }
}

void createDump() {
#ifdef DESKTOP
    return;
#endif
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), ".%Y-%m-%d_%H.%M.%S", timeinfo);

    mkpath("/contents/wampy/log/", 0755);
    char comm[100]{};
    sprintf(comm, "/system/vendor/sony/bin/hdumpstate -o /contents/wampy/log/log.%s", buffer);
    system(comm);
}

void startADB() { system("/system/vendor/sony/bin/AdbEnabler"); }

void getModel(std::string *model, bool *isWalkmanOne) {
#ifdef DESKTOP
    *model = "desktop";
    return;
#endif
    std::ifstream f;
    f.open("/dev/icx_nvp/033");
    std::string m((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    *model = m;
    f.close();

    struct stat info {};

    if (stat("/etc/.mod", &info) == 0) {
        if (info.st_mode & S_IFDIR) {
            *isWalkmanOne = true;
        }
    }
}

// restore core dump string
// usually it's set to /bin/true on my device
void restoreCoredumpPattern() {
    std::string input = "|/bin/sh /system/vendor/sony/etc/hcoredump.sh /var/log";
    std::ofstream out("/proc/sys/kernel/core_pattern");
    out << input;
    out.close();
}

void DisplayKeys() {
    // We iterate both legacy native range and named ImGuiKey ranges. This is a little unusual/odd but this allows
    // displaying the data for old/new backends.
    // User code should never have to go through such hoops!
    // You can generally iterate between ImGuiKey_NamedKey_BEGIN and ImGuiKey_NamedKey_END.
#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
    struct funcs {
        static bool IsLegacyNativeDupe(ImGuiKey) { return false; }
    };
    ImGuiKey start_key = ImGuiKey_NamedKey_BEGIN;
#else
    struct funcs {
        static bool IsLegacyNativeDupe(ImGuiKey key) { return key >= 0 && key < 512 && ImGui::GetIO().KeyMap[key] != -1; }
    }; // Hide Native<>ImGuiKey duplicates when both exists in the array
    auto start_key = (ImGuiKey)0;
#endif
    ImGui::SetCursorPos({0, 310});
    ImGuiIO &io = ImGui::GetIO();
    ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
    ImGui::SameLine();
    ImGui::Text("Keys down:");
    for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) {
        if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key))
            continue;
        ImGui::SameLine();
        ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key);
    }
    //        ImGui::Text(
    //            "Keys mods: %s%s%s%s",
    //            io.KeyCtrl ? "CTRL " : "",
    //            io.KeyShift ? "SHIFT " : "",
    //            io.KeyAlt ? "ALT " : "",
    //            io.KeySuper ? "SUPER " : ""
    //        );
    //        ImGui::Text("Chars queue:");
    //        for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
    //            ImWchar c = io.InputQueueCharacters[i];
    //            ImGui::SameLine();
    //            ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c);
    //        }
}

#ifndef IMGUITEST_UTIL_H
#define IMGUITEST_UTIL_H

#include "GLFW/glfw3.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <dirent.h>
#include <glad/glad.h>
#include <map>
#include <string>
#include <vector>

#define DLOG(fmt, ...) fprintf(stderr, "[wampy] %s %s:%d " fmt, __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
// #define DLOG(fmt, ...) printf("[wampy] " fmt, ##__VA_ARGS__)

struct directoryEntry {
    std::string fullPath{};
    std::string name{};
    bool valid = true;
};

struct TextureMapEntry {
    char *data;
    size_t len;
};

typedef std::map<std::string, TextureMapEntry> TextureMap;

void listdir(const char *dirname, std::vector<directoryEntry> *list, const std::string &extension);

void listdirs(const char *dirname, std::vector<directoryEntry> *list);

void UnloadTexture(ImTextureID textureID);

void UnloadTexture(GLuint textureID);

bool LoadTextureFromMagic(unsigned char *data, GLuint *out_texture, int dstWidth, int dstHeight);

constexpr unsigned int hash(const char *s, int off = 0) { return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ s[off]; }

ImU32 colorToImCol32(const std::string &color);

std::string parseColor(const std::string &in);

std::vector<std::string> split(const std::string &s, const std::string &delimiter);

std::string join(const std::vector<std::string> &v, int start);

void printFPS();

void CropTextToWidth(char *text, ImFont *font, float fontSize, float maxWidth);

std::string CalculateTextWidth(std::string text, float maxWidth = 600.0f);

static ImRect GetWindowRect(ImGuiWindow *window, int rect_type);

glm::mat4 rotAroundPoint(float deg, const glm::vec3 &origin, const glm::vec3 &axis);

__attribute__((unused)) static void mouse_click_callback(GLFWwindow *window, int button, int action, int mods);

void DrawWindowRects();

bool IsMounted();

static int do_mkdir(const char *path, mode_t mode);

int mkpath(const char *path, mode_t mode);

void recoverDumps(const std::string &outdir);

void createDump();

void startADB();

void getModel(std::string *model, bool *isWalkmanOne);

void restoreCoredumpPattern();

void DisplayKeys();

#define ETC_PKM_HEADER_SIZE 16
#define ETC_MAX_SIZE 4096

struct PKMHeader {
    char sig[4]{};        // "PKM "
    char ver[2]{};        // "10 or "20"
    uint16_t imageType{}; // e.g. 0 - ETC1_RGB_NO_MIPMAPS
    uint16_t width{};
    uint16_t height{};
    uint16_t origWidth{};
    uint16_t origHeight{};

    explicit PKMHeader(const std::string &path);
    explicit PKMHeader(std::fstream *s);
};

GLuint LoadCompressedTexture(int width, int height, ulong size, const char *data);

struct AtlasImage {
    uint16_t width;
    uint16_t height;
    uint16_t x;
    uint16_t y;
    float u0;
    float v0;
    float u1;
    float v1;

    void ToUV(float w, float h);
};

struct Atlas {
    GLuint textureID;
    uint16_t width;
    uint16_t height;
    std::vector<AtlasImage> images;
};

Atlas LoadAtlas(const std::string &texturePath, const std::string &coordsPath);

std::vector<AtlasImage> LoadCoords(const std::string &path);

void ConvertCoordsToUVAtlasImage(std::vector<ImVec4> *in, std::vector<AtlasImage> *images, int width, int height);

void swapbyte(uint16_t t);

bool exists(const std::string &s);

#endif // IMGUITEST_UTIL_H

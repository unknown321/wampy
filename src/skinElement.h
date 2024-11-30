#ifndef WAMPY_SKINELEMENT_H
#define WAMPY_SKINELEMENT_H

#include "imgui.h"
#include <unistd.h>

#include "glad/glad.h"
#include <utility>

#include "imgui_widgets.h"
#include "magick/magick.h"

#include "util/util.h"

typedef struct {
    size_t width, height;
} Size;

typedef struct {
    ssize_t x, y;
} Point;

typedef struct {
    float x, y;
} FPoint;

static ImVec2 uv0 = ImVec2(0.0f, 0.0f);                  // UV coordinates for lower-left
static ImVec2 uv1 = ImVec2(1.0f, 1.0f);                  // UV coordinates for (32,32) in our texture
static ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);   // Black background
static ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // No tint

const float upscaleRatioWalkman = 800.0f / 275.0f;
const float upscaleRatioCassette = 1.0f;

struct FlatTexture {
    explicit FlatTexture(FlatTexture *pTexture) {}

    FlatTexture() = default;

    Magick::RectangleInfo crop{};
    Size upscaled{};
    bool fillUpscaled = true;

    GLuint textureID{};
    ImVec2 position{};

    Magick::Image *image{};

    // variables for drawing solid color block over current song title in MAIN.BMP
    Magick::RectangleInfo fillPos{};
    Magick::Color fillColor{};

    std::string magick = "BMP";
    float ratio = upscaleRatioWalkman;
    bool valid{};

    FlatTexture *WithRatio(float r) {
        this->ratio = r;
        return this;
    }

    FlatTexture *WithMagick(std::string m) {
        this->magick = std::move(m);
        return this;
    }

    FlatTexture *FromPair(TextureMapEntry p) {
        this->FromData(p.data, p.len);
        return this;
    }

    Magick::Color GetColor(ssize_t x, ssize_t y) const { return image->pixelColor(x, y); }

    FlatTexture *FromData(char *data, size_t length) {
        this->image = new Magick::Image();
        Magick::Blob tmp(data, length);
        this->image->depth(8);
        this->image->magick(this->magick);
        try {
            this->image->read(tmp);
        } catch (...) {
            DLOG("cannot read image\n");
        }

        this->image->magick("RGBA");

        return this;
    }

    ImTextureID FromColor(const Magick::Geometry &g, const Magick::Color &color) {
        this->image = new Magick::Image(g, color);
        this->image->magick("RGBA");
        this->image->fillColor(color);
        this->image->draw(Magick::DrawableRectangle(0, 0, g.width(), g.height()));
        this->upscaled = {g.width(), g.height()};
        this->LoadTexture();
        this->Release();
//        DLOG("tid %d\n", this->textureID);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
        return (ImTextureID)this->textureID;
#pragma GCC diagnostic pop
    }

    void Crop() const {
        if (crop.height < 0 || crop.width < 0) {
            return;
        }

        MyMagick::Crop(image, crop);
    }

    void Upscale() {
        if (upscaled.width < 1) {
            upscaled.width = (unsigned int)((float)image->size().width() * ratio);
        }

        if (upscaled.height < 1) {
            upscaled.height = (unsigned int)((float)image->size().height() * ratio);
        }

        auto geo = Magick::Geometry{upscaled.width, upscaled.height, 0, 0};

        MyMagick::Upscale(image, geo, fillUpscaled);
    }

    void fillRectangle() const {
        if (fillPos.height > 0 && fillPos.width > 0 && fillPos.x > 0 && fillPos.y > 0) {
            MyMagick::FillRectangle(image, fillPos, fillColor);
        }
    }

    FlatTexture *WithCrop(Magick::RectangleInfo c) {
        this->crop = c;
        return this;
    }

    FlatTexture *WithFilledRectangle(Magick::RectangleInfo pos, const Magick::Color &c) {
        this->fillColor = c;
        this->fillPos = pos;
        return this;
    }

    FlatTexture *WithScale(const Size &g, bool fill) {
        this->upscaled = g;
        this->fillUpscaled = fill;
        return this;
    }

    FlatTexture *WithPosition(ImVec2 pos) {
        this->position = pos;
        return this;
    }

    void Unload() const { UnloadTexture(textureID); }

    // breaks in thread
    void LoadTexture() {
        if (textureID != 0) {
            glDeleteTextures(1, &textureID);
        }
        Magick::Blob blob;
        this->image->write(&blob);
        bool ret = LoadTextureFromMagic(
            (unsigned char *)blob.data(), &textureID, (int)this->image->size().width(), (int)this->image->size().height()
        );
        //        DLOG("tid %d\n", textureID);
        IM_ASSERT(ret);
    }

    void load() {
        this->Crop();
        this->Upscale();
        this->fillRectangle();
        this->LoadTexture();
        this->Release();
    }

    ImTextureID Load() {
        this->load();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
        return (ImTextureID)this->textureID;
#pragma GCC diagnostic pop
    }

    // returns size after upscaling
    ImVec2 GetSize() const { return {(float)upscaled.width, (float)upscaled.height}; }

    void Draw() const {
        ImGui::SetCursorPos(position);
        ImGui::Image((void *)(intptr_t)textureID, ImVec2(float(upscaled.width), float(upscaled.height)));
    }

    void DrawAt(float x, float y) const {
        ImGui::SetCursorPos(ImVec2(x, y));
        auto size = ImVec2(float(upscaled.width), float(upscaled.height));
        ImGui::Image((void *)(intptr_t)textureID, size);
    }

    void DrawAt(ImVec2 pos) const {
        ImGui::SetCursorPos(pos);
        auto size = ImVec2(float(upscaled.width), float(upscaled.height));
        ImGui::Image((void *)(intptr_t)textureID, size);
    }

    void Release() const { delete this->image; }

    FlatTexture *Reset() {
        this->crop = Magick::RectangleInfo{};
        this->upscaled = Size{};
        this->textureID = 0;
        this->fillUpscaled = false;
        this->position = ImVec2(0, 0);
        return this;
    }
};

struct Button {
    char iid[40]{};
    ImVec2 position{};
    ImGui::ButtonTextures textures;

    void (*callbackFun)(void *, void *){};

    void *callbackParam1;
    void *callbackParam2;

    Button *WithTextures(ImGui::ButtonTextures bt) {
        this->textures = std::move(bt);
        return this;
    }

    Button *WithPosition(float x, float y) {
        this->position.x = x;
        this->position.y = y;
        return this;
    }

    Button *WithID(const char *id) {
        sprintf(iid, "##%s", id);
        return this;
    }

    Button *WithCallback(void f(void *, void *), void *param1, void *param2) {
        this->callbackFun = f;
        this->callbackParam1 = param1;
        this->callbackParam2 = param2;
        return this;
    }

    void Unload() {
        for (auto &v : this->textures) {
            UnloadTexture(v.second.active);
            UnloadTexture(v.second.pressed);
        }
        this->textures.clear();
    }

    void Draw(int key = 0) {
        if (this->textures.empty()) {
            DLOG("no textures\n");
            return;
        }

        if (this->textures.find(key) == this->textures.end()) {
            DLOG("no key %d found\n", key);
            return;
        }

        auto v = this->textures.at(key);
        ImGui::SetCursorPos(position);
        if (ImGui::MyImageButtonEx(iid, v.active, v.pressed, v.size, uv0, uv1, bg_col, tint_col, 0)) {

            DLOG("clicked button %s\n", iid);

            if (callbackFun != nullptr) {
                callbackFun(callbackParam1, callbackParam2);
            }
        }
    }

    void DrawAt(float x, float y, int key = 0) const {
        if (this->textures.empty()) {
            DLOG("no textures\n");
            return;
        }

        if (this->textures.find(key) == this->textures.end()) {
            DLOG("no key %d found\n", key);
            return;
        }

        auto v = this->textures.at(key);
        ImGui::SetCursorPos(ImVec2(x, y));
        if (ImGui::MyImageButtonEx(iid, v.active, v.pressed, v.size, uv0, uv1, bg_col, tint_col, 0)) {

            DLOG("clicked button %s\n", iid);

            if (callbackFun != nullptr) {
                callbackFun(callbackParam1, callbackParam2);
            }
        }
    }
};

struct Slider {
    char iid[40]{};
    FPoint position{};
    ImGui::SliderBarTextures textures{};
    ImGui::ButtonTextures button{};
    int min{};
    int max{};
    bool wasMousePressed{};
    int prevValue;

    void (*callbackReleased)(void *, void *){};

    void (*callbackPressed)(void *, void *){};

    void *callbackPressedArg1;
    void *callbackPressedArg2;

    void *callbackReleasedArg1;
    void *callbackReleasedArg2;

    Slider *WithID(const char *id) {
        sprintf(iid, "##%s", id);
        return this;
    }

    Slider *WithBarTextures(ImGui::SliderBarTextures barTextures) {
        this->textures = std::move(barTextures);
        return this;
    }

    Slider *WithButtonTextures(ImGui::ButtonTextures buttonTextures) {
        this->button = std::move(buttonTextures);
        return this;
    }

    Slider *WithLimits(int Min, int Max) {
        this->min = Min;
        this->max = Max;
        return this;
    }

    Slider *WithPosition(float x, float y) {
        this->position.x = x;
        this->position.y = y;
        return this;
    }

    Slider *WithCallbackPressed(void c(void *, void *), void *arg1, void *arg2 = nullptr) {
        this->callbackPressed = c;
        this->callbackPressedArg1 = arg1;
        this->callbackPressedArg2 = arg2;
        return this;
    }

    Slider *WithCallbackReleased(void c(void *, void *), void *arg1, void *arg2 = nullptr) {
        this->callbackReleased = c;
        this->callbackReleasedArg1 = arg1;
        this->callbackReleasedArg2 = arg2;
        return this;
    }

    void Unload() {
        for (auto &v : textures) {
            UnloadTexture(v.second.textureId);
        }

        textures.clear();

        for (auto &v : button) {
            UnloadTexture(v.second.active);
            UnloadTexture(v.second.pressed);
        }

        button.clear();
    }

    void Draw(void *data) {
        ImGui::SetCursorPos({position.x, position.y});
        int mouseDown = 0;
        auto format = ""; // %d
        if (ImGui::MySliderScalar(iid, ImGuiDataType_S8, data, &min, &max, format, 0, textures, button, uv0, uv1, tint_col, &mouseDown)) {

            if (callbackPressed != nullptr) {
                callbackPressed(callbackPressedArg1, data);
            }

            if (mouseDown == 1) {
                wasMousePressed = true;
            }
        }

        if ((mouseDown == 0) && wasMousePressed) {
            wasMousePressed = false;
            // ehh
            auto val = *(int *)data;
            if (val != prevValue) {
                if (callbackReleased != nullptr) {
                    callbackReleased(callbackReleasedArg1, data);
                    prevValue = val;
                    mouseDown = 0;
                }
            }
        }
    }
};

#endif // WAMPY_SKINELEMENT_H

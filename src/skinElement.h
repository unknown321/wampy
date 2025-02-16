#ifndef WAMPY_SKINELEMENT_H
#define WAMPY_SKINELEMENT_H

#include "imgui.h"
#include <unistd.h>

#include "glad/glad.h"
#include <fstream>
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

static const float upscaleRatioWalkman = 800.0f / 275.0f;
static const float upscaleRatioCassette = 1.0f;

struct FlatTexture {
    explicit FlatTexture(FlatTexture *pTexture) {}

    FlatTexture() = default;

    std::vector<int> pointList{};
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

    int FromPKM(const std::string &texturePath) {
        std::fstream s;
        s.open(texturePath, std::fstream::in | std::fstream::binary);

        s.seekg(0, std::fstream::end);
        int length = s.tellg();
        s.seekg(0, std::fstream::beg);

        auto header = PKMHeader(&s);
        if (header.width < 1 || header.height < 1 || header.width > ETC_MAX_SIZE || header.height > ETC_MAX_SIZE) {
            DLOG("texture resolution %dx%d invalid\n", header.width, header.height);
            s.close();
            return -1;
        }

        if (strcmp(header.ver, "10") != 0) {
            DLOG("unexpected etc version\n");
            s.close();
            return -1;
        }

        std::string contents;

        char *b = new char[length - ETC_PKM_HEADER_SIZE];
        s.read(b, length - ETC_PKM_HEADER_SIZE);

        textureID = LoadCompressedTexture(header.width, header.height, length - ETC_PKM_HEADER_SIZE, b);
        s.close();
        delete[] b;

        if (textureID > 0) {
            this->upscaled.width = header.width;
            this->upscaled.height = header.height;
        }

        return 0;
    }

    // used only for winamp main window (for now), uses hardcoded dimensions and coords
    // corrupted texture on NVIDIA desktop after changing to another skin with mask
    void FromPointList(const std::vector<int> &pl) {
        Magick::CoordinateList coords{};
        for (int i = 0; i < pl.size(); i = i + 2) {
            Magick::Coordinate c;
            c.x((double)pl.at(i));
            // dirty fix for masks starting at 0
            if (c.x() == 0) {
                c.x(-1);
            }

            if (c.x() == 275) {
                c.x(276);
            }

            c.y((double)pl.at(i + 1));
            coords.push_back(c);
        }

        if (coords.empty()) {
            Reset();
            return;
        }

        auto tmpimage = Magick::Image({275, 165}, "#000000");
        tmpimage.magick("RGBA");
        tmpimage.depth(8);
        tmpimage.alpha(true);

        auto mask = Magick::Image({275, 165}, "#000000");
        mask.magick("RGBA");
        mask.strokeColor("#000000");
        mask.fillColor("#ffffff");
        mask.strokeAntiAlias(true);
        mask.strokeWidth(-1);
        auto d = Magick::DrawablePolygon(coords);
        mask.draw(d);
        mask.negate();

        tmpimage.writeMask(mask);
        tmpimage.composite(tmpimage, "+0+0", MagickCore::ClearCompositeOp);

        auto v = Magick::Geometry{800, 480, 0, 0};

        tmpimage.compressType(MagickCore::NoCompression);
        tmpimage.filterType(MagickCore::PointFilter);
        v.aspect(true);
        v.fillArea(false);
        tmpimage.resize(v);

        // playlist zone
        auto mask2 = Magick::Image({tmpimage.size().width(), 480}, "#000000");
        mask2.magick("RGBA");
        mask2.strokeColor("#000000");
        mask2.fillColor("#ffffff");
        mask2.strokeAntiAlias(true);
        mask2.strokeWidth(1);
        auto r = Magick::DrawableRectangle({-3, -3, 800, 337});
        mask2.draw(r);

        tmpimage.writeMask(mask2);
        tmpimage.composite(tmpimage, "+0+0", MagickCore::ClearCompositeOp);

        this->image = new Magick::Image(tmpimage.size(), "#000000");
        this->image->depth(8);
        this->image->magick("RGBA");
        this->image->composite(tmpimage, "+0+0", MagickCore::CopyCompositeOp);

        this->LoadTexture();
        this->upscaled.width = 800;
        this->upscaled.height = 480;
        this->position = ImVec2(0, 0);

        this->Release();
    }

    Magick::Color GetColor(ssize_t x, ssize_t y) const { return image->pixelColor(x, y); }

    FlatTexture *FromData(char *data, size_t length) {
        this->image = new Magick::Image();
        Magick::Blob tmp(data, length);
        this->image->magick(this->magick);
        try {
            this->image->read(tmp);
        } catch (...) {
            DLOG("cannot read image\n");
        }

        this->image->magick("RGBA");

        return this;
    }

    ImTextureID BarFromColors(const Magick::Geometry &g, const std::vector<std::string> &colors) {
        Magick::Color color;
        if (colors.empty()) {
            color = "#ffffff";
        }
        this->image = new Magick::Image(g, color);
        this->image->magick("RGBA");
        this->image->fillColor(color);
        this->image->draw(Magick::DrawableRectangle(0, 0, g.width(), g.height()));
        this->upscaled = {g.width(), g.height()};
        auto yPos = 0;
        auto barHeight = 3;
        int i = 0;
        for (const auto &c : colors) {
            i++;
            if (i < 3) { // skip two first colors
                continue;
            }
            this->image->fillColor(c);
            if (yPos == 21) {
                barHeight = 2;
            } else {
                barHeight = 3;
            }
            //            DLOG("%s %d %d\n", c.c_str(), yPos, barHeight);
            this->image->draw(Magick::DrawableRectangle(0, yPos, g.width(), yPos + barHeight));
            yPos += barHeight;
            if (yPos > g.height()) {
                break;
            }
        }
        this->LoadTexture();
        this->Release();
//        DLOG("tid %d\n", this->textureID);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
        return (ImTextureID)this->textureID;
#pragma GCC diagnostic pop
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
        if (crop.height == 0 && crop.width == 0 && crop.y == 0 && crop.x == 0) {
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

    void Unload() {
        UnloadTexture(textureID);
        textureID = 0;
    }

    // breaks in thread
    void LoadTexture() {
        if (textureID != 0) {
            glDeleteTextures(1, &textureID);
        }

        Magick::Blob blob;
        this->image->write(&blob);
        //        DLOG(
        //            "loading %zu bytes, size %zux%zu (%zu), magick %s\n",
        //            blob.length(),
        //            image->size().width(),
        //            image->size().height(),
        //            image->size().width() * image->size().height() * 4,
        //            image->magick().c_str()
        //        );
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
        if (textureID == 0) {
            return;
        }
        ImGui::SetCursorPos(position);
        ImGui::Image((ImTextureID)(intptr_t)textureID, ImVec2(float(upscaled.width), float(upscaled.height)));
    }

    void DrawAt(float x, float y) const {
        if (textureID == 0) {
            return;
        }
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
        memset(this->iid, 0, 40);
        this->callbackFun = nullptr;
        this->position = ImVec2();
        this->callbackParam2 = nullptr;
        this->callbackParam1 = nullptr;
    }

    void Draw(int key = 0) {
        if (this->textures.empty()) {
            DLOG("%s no textures\n", iid);
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

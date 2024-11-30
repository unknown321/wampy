#ifndef WAMPY_IMGUI_WIDGETS_H
#define WAMPY_IMGUI_WIDGETS_H

#include "imgui_internal.h"
#include <map>

namespace ImGui {
    typedef std::map<int, ImTextureID> ValueTexture;

    struct SliderBarTexture {
        ImTextureID textureId{};
        ImVec2 size;
    };

    typedef std::map<int, SliderBarTexture> SliderBarTextures;

    // see https://m2.material.io/design/interaction/states.html for state names
    struct ButtonTexture {
        ImTextureID active{};
        ImTextureID pressed{};
        ImVec2 size;
    };

    typedef std::map<int, ButtonTexture> ButtonTextures;

    bool MyImageButtonEx(
        const char *id, ImTextureID texture_id, ImTextureID pressed_tid, const ImVec2 &image_size, const ImVec2 &uv0, const ImVec2 &uv1,
        const ImVec4 &bg_col, const ImVec4 &tint_col, ImGuiButtonFlags flags
    );

    template <typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
    bool MySliderBehaviorT(
        const ImRect &bb, ImGuiID id, ImGuiDataType data_type, TYPE *v, TYPE v_min, TYPE v_max, const char *format, ImGuiSliderFlags flags,
        ImRect *out_grab_bb, ImVec2 grabSize
    );

    bool MySliderScalar(
        const char *label, ImGuiDataType data_type, void *p_data, const void *p_min, const void *p_max, const char *format,
        ImGuiSliderFlags flags, const SliderBarTextures &background, ButtonTextures button, ImVec2 &uv0, ImVec2 &uv1,
        const ImVec4 &tint_col, int *mouseDown
    );
} // namespace ImGui

#endif
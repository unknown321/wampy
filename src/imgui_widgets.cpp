#include "imgui_widgets.h"
#include <stdexcept>

namespace ImGui {
    bool MyImageButtonEx(
        const char *id, ImTextureID texture_id, ImTextureID pressed_tid, const ImVec2 &image_size, const ImVec2 &uv0, const ImVec2 &uv1,
        const ImVec4 &bg_col, const ImVec4 &tint_col, ImGuiButtonFlags flags
    ) {
        ImGuiContext &g = *GImGui;
        ImGuiWindow *window = GetCurrentWindow();

        ImGuiID iid = window->GetID(id);
        if (window->SkipItems)
            return false;

        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + image_size);
        ItemSize(bb);
        if (!ItemAdd(bb, iid))
            return false;

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, iid, &hovered, &held, flags);

        ImTextureID tid = texture_id;

        // Render hovered/held blue rectangle
        //        const ImU32 col = GetColorU32(
        //                (held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        //        RenderFrame(bb.Min, bb.Max, col, true,
        //                    ImClamp(0.0f, 0.0f, g.Style.FrameRounding));

        if (held) {
            window->DrawList->AddImage(pressed_tid, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
        } else {
            window->DrawList->AddImage(texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
        }

        return pressed;
    }

    template <typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
    bool MySliderBehaviorT(
        const ImRect &bb, ImGuiID id, ImGuiDataType data_type, TYPE *v, const TYPE v_min, const TYPE v_max, const char *format,
        ImGuiSliderFlags flags, ImRect *out_grab_bb, ImVec2 grabSize
    ) {
        ImGuiContext &g = *GImGui;
        const ImGuiStyle &style = g.Style;

        const ImGuiAxis axis = (flags & ImGuiSliderFlags_Vertical) ? ImGuiAxis_Y : ImGuiAxis_X;
        const bool is_logarithmic = (flags & ImGuiSliderFlags_Logarithmic) != 0;
        const bool is_floating_point = (data_type == ImGuiDataType_Float) || (data_type == ImGuiDataType_Double);
        const auto v_range_f =
            (float)(v_min < v_max ? v_max - v_min : v_min - v_max); // We don't need high precision for what we do with it.

        // Calculate bounds
        const float grab_padding = 2.0f;
        const float slider_sz = (bb.Max[axis] - bb.Min[axis]) - grab_padding * 2.0f;
        float grab_sz = style.GrabMinSize;
        //        if (!is_floating_point &&
        //            v_range_f >= 0.0f)                         // v_range_f < 0 may happen on integer overflows
        //            grab_sz = ImMax(slider_sz / (v_range_f + 1),
        //                            style.GrabMinSize); // For integer sliders: if possible have the grab size represent 1 unit
        //        grab_sz = ImMin(grab_sz, slider_sz);
        grab_sz = grabSize.x;
        const float slider_usable_sz = slider_sz - grab_sz;
        const float slider_usable_pos_min = bb.Min[axis] + grab_padding + grab_sz * 0.5f;
        const float slider_usable_pos_max = bb.Max[axis] - grab_padding - grab_sz * 0.5f;
        //        printf("usable max %f, %f\n", slider_usable_pos_max, grab_sz);

        float logarithmic_zero_epsilon = 0.0f; // Only valid when is_logarithmic is true
        float zero_deadzone_halfsize = 0.0f;   // Only valid when is_logarithmic is true
        if (is_logarithmic) {
            // When using logarithmic sliders, we need to clamp to avoid hitting zero, but our choice of clamp value greatly affects slider
            // precision. We attempt to use the specified precision to estimate a good lower bound.
            const int decimal_precision = is_floating_point ? ImParseFormatPrecision(format, 3) : 1;
            logarithmic_zero_epsilon = ImPow(0.1f, (float)decimal_precision);
            zero_deadzone_halfsize = (style.LogSliderDeadzone * 0.5f) / ImMax(slider_usable_sz, 1.0f);
        }

        // Process interacting with the slider
        bool value_changed = false;
        if (g.ActiveId == id) {
            bool set_new_value = false;
            float clicked_t = 0.0f;
            if (g.ActiveIdSource == ImGuiInputSource_Mouse) {
                if (!g.IO.MouseDown[0]) {
                    ClearActiveID();
                } else {
                    const float mouse_abs_pos = g.IO.MousePos[axis];
                    if (g.ActiveIdIsJustActivated) {
                        float grab_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
                            data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize
                        );
                        if (axis == ImGuiAxis_Y)
                            grab_t = 1.0f - grab_t;
                        const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
                        const bool clicked_around_grab =
                            (mouse_abs_pos >= grab_pos - grab_sz * 0.5f - 1.0f) &&
                            (mouse_abs_pos <= grab_pos + grab_sz * 0.5f + 1.0f); // No harm being extra generous here.
                        g.SliderGrabClickOffset = (clicked_around_grab && is_floating_point) ? mouse_abs_pos - grab_pos : 0.0f;
                    }
                    if (slider_usable_sz > 0.0f)
                        clicked_t = ImSaturate((mouse_abs_pos - g.SliderGrabClickOffset - slider_usable_pos_min) / slider_usable_sz);
                    if (axis == ImGuiAxis_Y)
                        clicked_t = 1.0f - clicked_t;
                    set_new_value = true;
                }
            } else if (g.ActiveIdSource == ImGuiInputSource_Keyboard || g.ActiveIdSource == ImGuiInputSource_Gamepad) {
                if (g.ActiveIdIsJustActivated) {
                    g.SliderCurrentAccum = 0.0f; // Reset any stored nav delta upon activation
                    g.SliderCurrentAccumDirty = false;
                }

                float input_delta = (axis == ImGuiAxis_X) ? GetNavTweakPressedAmount(axis) : -GetNavTweakPressedAmount(axis);
                if (input_delta != 0.0f) {
                    const bool tweak_slow = IsKeyDown(
                        (g.NavInputSource == ImGuiInputSource_Gamepad) ? ImGuiKey_NavGamepadTweakSlow : ImGuiKey_NavKeyboardTweakSlow
                    );
                    const bool tweak_fast = IsKeyDown(
                        (g.NavInputSource == ImGuiInputSource_Gamepad) ? ImGuiKey_NavGamepadTweakFast : ImGuiKey_NavKeyboardTweakFast
                    );
                    const int decimal_precision = is_floating_point ? ImParseFormatPrecision(format, 3) : 0;
                    if (decimal_precision > 0) {
                        input_delta /= 100.0f; // Gamepad/keyboard tweak speeds in % of slider bounds
                        if (tweak_slow)
                            input_delta /= 10.0f;
                    } else {
                        if ((v_range_f >= -100.0f && v_range_f <= 100.0f && v_range_f != 0.0f) || tweak_slow)
                            input_delta =
                                ((input_delta < 0.0f) ? -1.0f : +1.0f) / v_range_f; // Gamepad/keyboard tweak speeds in integer steps
                        else
                            input_delta /= 100.0f;
                    }
                    if (tweak_fast)
                        input_delta *= 10.0f;

                    g.SliderCurrentAccum += input_delta;
                    g.SliderCurrentAccumDirty = true;
                }

                float delta = g.SliderCurrentAccum;
                if (g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated) {
                    ClearActiveID();
                } else if (g.SliderCurrentAccumDirty) {
                    clicked_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
                        data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize
                    );

                    if ((clicked_t >= 1.0f && delta > 0.0f) ||
                        (clicked_t <= 0.0f && delta < 0.0f)) // This is to avoid applying the saturation when already past the limits
                    {
                        set_new_value = false;
                        g.SliderCurrentAccum = 0.0f; // If pushing up against the limits, don't continue to accumulate
                    } else {
                        set_new_value = true;
                        float old_clicked_t = clicked_t;
                        clicked_t = ImSaturate(clicked_t + delta);

                        // Calculate what our "new" clicked_t will be, and thus how far we actually moved the slider, and subtract this from
                        // the accumulator
                        TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(
                            data_type, clicked_t, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize
                        );
                        if (is_floating_point && !(flags & ImGuiSliderFlags_NoRoundToFormat))
                            v_new = RoundScalarWithFormatT<TYPE>(format, data_type, v_new);
                        float new_clicked_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
                            data_type, v_new, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize
                        );

                        if (delta > 0)
                            g.SliderCurrentAccum -= ImMin(new_clicked_t - old_clicked_t, delta);
                        else
                            g.SliderCurrentAccum -= ImMax(new_clicked_t - old_clicked_t, delta);
                    }

                    g.SliderCurrentAccumDirty = false;
                }
            }

            if (set_new_value)
                if ((g.LastItemData.InFlags & ImGuiItemFlags_ReadOnly) || (flags & ImGuiSliderFlags_ReadOnly))
                    set_new_value = false;

            if (set_new_value) {
                TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(
                    data_type, clicked_t, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize
                );

                // Round to user desired precision based on format string
                if (is_floating_point && !(flags & ImGuiSliderFlags_NoRoundToFormat))
                    v_new = RoundScalarWithFormatT<TYPE>(format, data_type, v_new);

                // Apply result
                if (*v != v_new) {
                    *v = v_new;
                    value_changed = true;
                }
            }
        }

        if (slider_sz < 1.0f) {
            *out_grab_bb = ImRect(bb.Min, bb.Min);
        } else {
            // Output grab position, so it can be displayed by the caller
            float grab_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
                data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize
            );
            if (axis == ImGuiAxis_Y)
                grab_t = 1.0f - grab_t;
            const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
            if (axis == ImGuiAxis_X)
                *out_grab_bb =
                    ImRect(grab_pos - grab_sz * 0.5f, bb.Min.y + grab_padding, grab_pos + grab_sz * 0.5f, bb.Max.y - grab_padding);
            else
                *out_grab_bb =
                    ImRect(bb.Min.x + grab_padding, grab_pos - grab_sz * 0.5f, bb.Max.x - grab_padding, grab_pos + grab_sz * 0.5f);
        }

        return value_changed;
    }

    bool MySliderScalar(
        const char *label, ImGuiDataType data_type, void *p_data, const void *p_min, const void *p_max, const char *format,
        ImGuiSliderFlags flags, const SliderBarTextures &background, ButtonTextures button, ImVec2 &uv0, ImVec2 &uv1,
        const ImVec4 &tint_col, int *mouseDown
    ) {
        ImGuiWindow *window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext &g = *GImGui;
        const ImGuiStyle &style = g.Style;
        const ImGuiID id = window->GetID(label);
        const float w = CalcItemWidth();

        const ImVec2 label_size = CalcTextSize(label, nullptr, true);
        const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + background.at(0).size);

        const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
        if (!ItemAdd(total_bb, id, &total_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0))
            return false;

        // Default format string when passing NULL
        if (format == nullptr)
            format = DataTypeGetInfo(data_type)->PrintFmt;

        const bool hovered = ItemHoverable(total_bb, id, g.LastItemData.InFlags);
        bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
        bool clicked;
        if (!temp_input_is_active) {
            // Tabbing or CTRL-clicking on Slider turns it into an input box
            clicked = hovered && IsMouseClicked(0, ImGuiInputFlags_None, id);
            const bool make_active = (clicked || g.NavActivateId == id);
            if (make_active && clicked)
                SetKeyOwner(ImGuiKey_MouseLeft, id);
            if (make_active && temp_input_allowed)
                if ((clicked && g.IO.KeyCtrl) || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput)))
                    temp_input_is_active = true;

            if (make_active && !temp_input_is_active) {
                SetActiveID(id, window);
                SetFocusID(id, window);
                FocusWindow(window);
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
            }
        }

        if (IsMouseDown(0, id)) {
            *mouseDown = 1;
        }

        if (temp_input_is_active) {
            // Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
            const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0;
            return TempInputScalar(
                total_bb, id, label, data_type, p_data, format, is_clamp_input ? p_min : nullptr, is_clamp_input ? p_max : nullptr
            );
        }

        int total = abs(*(int *)p_max) + abs(*(int *)p_min);

        int index = (int)(*(int *)p_data / float(total / (float)background.size()));
        if (index == background.size()) {
            index--;
        }

        ImTextureID tid;
        try {
            tid = background.at(index).textureId;
        } catch (std::out_of_range &ofRange) {
            tid = background.at(0).textureId;
        }

        window->DrawList->AddImage(tid, total_bb.Min, total_bb.Max, uv0, uv1, GetColorU32(tint_col));

        // Slider behavior
        ImRect grab_bb;
        const bool value_changed = MySliderBehaviorT<ImS32, ImS32, float>(
            total_bb,
            id,
            ImGuiDataType_S32,
            (ImS32 *)p_data,
            *(const ImS8 *)p_min,
            *(const ImS8 *)p_max,
            format,
            flags,
            &grab_bb,
            button.at(0).size
        );
        if (value_changed)
            MarkItemEdited(id);

        // Render grab
        if (grab_bb.Max.x > grab_bb.Min.x) {
            if (g.ActiveId == id) {
                window->DrawList->AddImage(button.at(0).pressed, grab_bb.Min, grab_bb.Max, uv0, uv1, GetColorU32(tint_col));
            } else {
                window->DrawList->AddImage(button.at(0).active, grab_bb.Min, grab_bb.Max, uv0, uv1, GetColorU32(tint_col));
            }
        }

        //         Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
        if (strcmp(format, "") != 0) {
            char value_buf[64];
            const char *value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
            if (g.LogEnabled)
                LogSetNextTextDecoration("{", "}");
            RenderTextClipped(total_bb.Min, total_bb.Max, value_buf, value_buf_end, nullptr, ImVec2(0.5f, 0.5f));
        }

        if (label_size.x > 0.0f)
            RenderText(ImVec2(total_bb.Max.x + style.ItemInnerSpacing.x, total_bb.Min.y + style.FramePadding.y), label);

        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (temp_input_allowed ? ImGuiItemStatusFlags_Inputable : 0));
        return value_changed;
    }
} // namespace ImGui
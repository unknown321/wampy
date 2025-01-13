#ifndef WAMPY_IMGUI_CURVE_H
#define WAMPY_IMGUI_CURVE_H

#include "imgui_internal.h"
namespace ImGui {

    enum class CurveEditorFlags { NO_TANGENTS = 1 << 0, SHOW_GRID = 1 << 1, RESET = 1 << 2 };

    int CurveEditor(
        const char *label, float *values, int points_count, int capacity, const ImVec2 &editor_size, ImU32 flags, int *new_count,
        int *selected_point, int *hovered_point, const int *zoomDirection, int nodeSize = 10, bool editable = true, float Ylimit = 255.0f,
        const int *volume = nullptr
    );
}; // namespace ImGui

#endif
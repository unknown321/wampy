#include "skin.h"

#include <libintl.h>

void Skin::Winamp() {
    if (!connector->storagePresent) {
        ImGui::Text(gettext("Disable USB mass storage mode"));
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    if (ImGui::BeginCombo("##", skinListWinamp.at(selectedSkinIdx).name.c_str(), ImGuiComboFlags_HeightRegular)) {
        for (int n = 0; n < skinListWinamp.size(); n++) {
            const bool is_selected = (selectedSkinIdx == n);
            if (ImGui::Selectable(skinListWinamp.at(n).name.c_str(), is_selected)) {
                selectedSkinIdx = n;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(2);

    ImGui::SameLine();
    if (activeSkinVariant == WINAMP) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 11);
        if (ImGui::Button(gettext("Load skin"), ImVec2(186, 50))) {
            loadStatusStr = gettext("Loading ") + skinListWinamp.at(selectedSkinIdx).name;
            winamp.changeSkin(skinListWinamp.at(selectedSkinIdx).fullPath);
            needLoad = true;
        }
    }

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8);
    ImGui::Text("%s", loadStatusStr.c_str());
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

    static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp;

    ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 7);

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    if (ImGui::BeginTable("winampConfigTable", 2, flags, outer_size)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Use bitmap font"), &config->winamp.useBitmapFont)) {
            config->Save();
            if (activeSkinVariant == WINAMP) {
                winamp.Format(true);
            }
        }
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Enable visualizer"), &config->winamp.visualizerEnable)) {
            config->Save();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Use bitmap font in playlist"), &config->winamp.useBitmapFontInPlaylist)) {
            config->Save();
            if (activeSkinVariant == WINAMP) {
                winamp.Format(true);
            }
        }
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Visualizer Winamp mode"), &config->winamp.visualizerWinampBands)) {
            config->Save();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Prefer time remaining"), &config->winamp.preferTimeRemaining)) {
            config->Save();
        }
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Visualizer sensitivity"));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Show clutterbar"), &config->winamp.showClutterbar)) {
            config->Save();
        }

        ImGui::TableNextColumn();
        if (ImGui::SliderFloat("##sensslider", &config->winamp.visualizerSensitivity, WINAMP_VISUALIZER_SENS_MIN, WINAMP_VISUALIZER_SENS_MAX, "")) {
            config->winamp.visualizerSensitivity *= 100;
            config->winamp.visualizerSensitivity = round(config->winamp.visualizerSensitivity) / 100;
            config->Save();
        }
        ImGui::SameLine();
        ImGui::Text("%.02f", config->winamp.visualizerSensitivity);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Skin transparency"), &config->winamp.skinTransparency)) {
            config->Save();
        }

        ImGui::TableNextColumn();

        ImGui::EndTable();
    }

    ImGui::PopStyleVar(2);
}

#include "skin.h"
#include <libintl.h>

void Skin::TabEQ() {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

    if (prevSong != connector->status.Filename) {
        connector->soundSettings.Update();
        connector->soundSettingsFw.Update();
        eqSongExists = SoundSettings::Exists(connector->status.Filename);
        eqSongDirExists = SoundSettings::ExistsDir(connector->status.Filename);
        prevSong = connector->status.Filename;
        eqStatus = gettext("Refreshed");
    }

    if (prevPlayState != connector->status.State) {
        prevPlayState = connector->status.State;
        if (connector->status.State == PLAYING) {
            effectQueueStatus.clear();
            connector->soundSettingsFw.Update();
        }
    }

    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        ImGui::BeginChild("ChildL", ImVec2(580, 425), ImGuiChildFlags_Border, window_flags);

        if (connector->soundSettingsFw.s->directSourceOn) {
            ImGui::Text(gettext("Direct Source is on, filters disabled"));
            ImGui::NewLine();
            TabEQ_Misc();
        } else {
            switch (eActiveFilterTab) {

            case ActiveFilterTab_Invalid:
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - ImGui::CalcTextSize(gettext("Select filter")).x / 2);
                ImGui::SetCursorPosY(
                    ImGui::GetCursorPosY() + 425 / 2 - ImGui::CalcTextSize(gettext("Select filter")).y / 2 - ImGui::GetTextLineHeightWithSpacing());
                ImGui::Text(gettext("Select filter"));
                break;
            case ActiveFilterTab_DynamicNormalizer:
                TabEQ_DynamicNormalizer();
                break;
            case ActiveFilterTab_Eq6Band:
                TabEQ_Eq6Band();
                break;
            case ActiveFilterTab_Vpt:
                TabEQ_Vpt();
                break;
            case ActiveFilterTab_DCPhaseLinearizer:
                TabEQ_DCPhase();
                break;
            case ActiveFilterTab_Vinylizer:
                TabEQ_Vinylizer();
                break;
            case ActiveFilterTab_Eq10Band:
                TabEQ_Eq10Band();
                break;
            case ActiveFilterTab_EqTone:
                TabEQ_EqTone();
                break;
            case ActiveFilterTab_Donate:
                TabEQ_Donate();
                break;
            case ActiveFilterTab_Misc:
                TabEQ_Misc();
                break;
            }
        }

        ImGui::EndChild();
    }

    ImGui::SameLine();

    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        ImGui::BeginChild("ChildR", ImVec2(180, 425), ImGuiChildFlags_Border, window_flags);

        static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;
        if (ImGui::BeginTable("##sswFilters", 2, flags)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 10.0f);
            ImGui::TableSetupColumn(gettext("Filter"), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 120.0f);
            ImGui::TableHeadersRow();
            for (auto v : connector->soundSettingsFw.s->FilterStatus) {
                if (v.name[0] == '\0') {
                    continue;
                }
                if (strcmp(v.name, "i2f") == 0) {
                    continue;
                }
                if (strcmp(v.name, "f2i") == 0) {
                    continue;
                }
                if (strcmp(v.name, "heq") == 0) {
                    continue;
                }
                if (strcmp(v.name, "alc") == 0) {
                    continue;
                }
                if (strcmp(v.name, "attn") == 0) {
                    continue;
                }
                if (strcmp(v.name, "clearphase") == 0) {
                    continue;
                }
                if (strlen(v.name) > 4) {
                    if (strncmp(v.name, "dsee", 4) == 0) {
                        continue;
                    }
                }
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (v.is_proc) {
                    ImGui::Text("+");
                }
                ImGui::TableNextColumn();

                if (strcmp(v.name, activeFilter.c_str()) == 0) {
                    ImGui::PushStyleColor(ImGuiCol_Button, PLEASANT_GREEN);
                    if (ImGui::Button(v.name, ImVec2(130, 38))) {
                        SetActiveEqFilter(v.name);
                    }
                    ImGui::PopStyleColor();
                } else {
                    if (ImGui::Button(v.name, ImVec2(130, 38))) {
                        SetActiveEqFilter(v.name);
                    }
                }
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            if (strcmp("misc", activeFilter.c_str()) == 0) {
                ImGui::PushStyleColor(ImGuiCol_Button, PLEASANT_GREEN);
                if (ImGui::Button(gettext("Misc"), ImVec2(130, 38))) {
                    SetActiveEqFilter("misc");
                }
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Button(gettext("Misc"), ImVec2(130, 38))) {
                    SetActiveEqFilter("misc");
                }
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Button, GOLD_DONATE);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));
            if (ImGui::Button(gettext("Donate"), ImVec2(130, 38))) {
                SetActiveEqFilter("donate");
            }
            ImGui::PopStyleColor(2);

            ImGui::EndTable();
        }

        ImGui::EndChild();
    }
}

void Skin::SetActiveEqFilter(const std::string &v) {
    DLOG("setting active filter tab: %s\n", v.c_str());
    if (v == "donate") {
        eActiveFilterTab = ActiveFilterTab_Donate;
        activeFilter = v;
        return;
    }

    if (v == "misc") {
        eActiveFilterTab = ActiveFilterTab_Misc;
        activeFilter = v;
        return;
    }

    if (connector->soundSettingsFw.filters.find(v) == connector->soundSettingsFw.filters.end()) {
        activeFilter = connector->soundSettingsFw.filterInvalid;
        eActiveFilterTab = ActiveFilterTab_Invalid;
        return;
    }

    activeFilter = v;

    if (filterToTab.find(activeFilter) == filterToTab.end()) {
        eActiveFilterTab = ActiveFilterTab_Invalid;
        activeFilter = connector->soundSettingsFw.filterInvalid;
        return;
    }

    eActiveFilterTab = filterToTab[activeFilter];
}

void Skin::SetConfigFilter(std::string name, bool value) {
    switch (hash(name.c_str())) {
    case hash("vpt"):
        config->filters.vpt = value;
        break;
    case hash("vinylizer"):
        config->filters.vinylizer = value;
        break;
    case hash("eqtone"):
        config->filters.eqtone = value;
        break;
    case hash("eq6band"):
        config->filters.eq6Band = value;
        break;
    case hash("eq10band"):
        config->filters.eq10Band = value;
        break;
    case hash("dcphaselinear"):
        config->filters.dcphaselinear = value;
        break;
    case hash("dynamicnormalizer"):
        config->filters.dynamicnormalizer = value;
        break;
    default:
        break;
    }

    config->Save();
}
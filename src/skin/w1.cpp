#include "skin.h"
#include <libintl.h>

void Skin::WalkmanOne() {
    if (!connector->storagePresent) {
        ImGui::NewLine();
        ImGui::Text(gettext("Disable USB mass storage mode"));
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 15.0f));
    ImGui::SeparatorText(gettext("Walkman One settings"));
    ImGui::PopStyleVar();

    if (!walkmanOneOptions.configFound) {
        ImGui::Text(gettext("Config file not found\n"));
        return;
    }

    static ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchSame;

    auto outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 3);

    if (ImGui::BeginTable("walkmanOneTable", 2, flags, outer_size)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext(gettext("Interface color")));

        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##walkmanOneColor", W1::colorByValueWalkmanOne.at(walkmanOneOptions.color).c_str(), ImGuiComboFlags_HeightRegular)) {
            for (const auto &entry : W1::colorByNameWalkmanOne) {
                if (ImGui::Selectable(entry.first.c_str(), false)) {
                    walkmanOneOptions.color = entry.second;
                    DLOG("selected color %s\n", W1::colorByValueWalkmanOne.at(walkmanOneOptions.color).c_str());
                    walkmanOneOptions.Save();
                    needRestart = true;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Sound signature"));

        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        if (ImGui::BeginCombo("##walkmanOneSignature", W1::signatureByValueWalkmanOne.at(walkmanOneOptions.signature).c_str(), ImGuiComboFlags_HeightRegular)) {
            for (const auto &entry : W1::signatureByNameWalkmanOne) {
                if (ImGui::Selectable(entry.first.c_str(), false)) {
                    walkmanOneOptions.signature = entry.second;
                    DLOG("selected signature %s\n", W1::signatureByValueWalkmanOne.at(walkmanOneOptions.signature).c_str());
                    walkmanOneOptions.Save();
                    needRestart = true;
                    walkmanOneOptions.tuningChanged = true;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Region"));

        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        if (ImGui::BeginCombo("##walkmanOneRegion", walkmanOneOptions.region, ImGuiComboFlags_HeightRegular)) {
            for (const auto &entry : W1::regionWalkmanOne) {
                if (ImGui::Selectable(entry.c_str(), false)) {
                    strncpy(walkmanOneOptions.region, entry.c_str(), sizeof walkmanOneOptions.region);
                    DLOG("selected region %s\n", entry.c_str());
                    walkmanOneOptions.Save();
                    needRestart = true;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Remote option with any region"));
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##remote", &walkmanOneOptions.remote)) {
            DLOG("Set remote option to %d\n", walkmanOneOptions.remote);
            walkmanOneOptions.Save();
            needRestart = true;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Plus mode v2"));
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##plusv2", &walkmanOneOptions.plusModeVersionBOOL)) {
            DLOG("Set plus mode v2 to %d\n", walkmanOneOptions.plusModeVersionBOOL);
            walkmanOneOptions.Save();
            needRestart = true;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Plus mode by default"));
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##plusdefault", &walkmanOneOptions.plusModeByDefault)) {
            DLOG("Set plus mode default to %d\n", walkmanOneOptions.plusModeByDefault);
            walkmanOneOptions.Save();
            needRestart = true;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Lower gain mode"));
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##gainmode", &walkmanOneOptions.gainMode)) {
            DLOG("Set gain mode to %d\n", walkmanOneOptions.gainMode);
            walkmanOneOptions.Save();
            needRestart = true;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Different DAC init mode"));
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##dacinitmode", &walkmanOneOptions.dacInitializationMode)) {
            DLOG("Set dac init mode to %d\n", walkmanOneOptions.dacInitializationMode);
            walkmanOneOptions.Save();
            needRestart = true;
        }

        ImGui::EndTable();
    }

    ImGui::NewLine();
    if (needRestart) {
#ifndef DESKTOP
        if (walkmanOneOptions.tuningChanged) {
            if (ImGui::Button(gettext("Apply tuning and reboot"), ImVec2(300, 60))) {
                walkmanOneOptions.Reboot();
            }
        } else {
            ImGui::Text(gettext("Reboot the device to apply changes"));
        }
#endif
    }
}

void Skin::Wee1() {
    ImGui::NewLine();
    ImGui::Text(gettext("Interface color"));

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    if (ImGui::BeginCombo("##w1color", W1::colorByValue.at(w1Options.deviceColor).c_str(), ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : W1::colorByName) {
            if (ImGui::Selectable(entry.first.c_str(), false)) {
                w1Options.deviceColor = entry.second;
                DLOG("selected %s\n", W1::colorByValue.at(w1Options.deviceColor).c_str());
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(2);
    ImGui::SameLine();
    if (ImGui::Button(gettext("Change color"))) {
        W1::SetColor(w1Options.deviceColor);
        needRestart = true;
    }

    if (needRestart) {
        ImGui::NewLine();
        ImGui::Text(gettext("Reboot the device to apply changes"));
        //            if (ImGui::Button(gettext("Reboot device"), ImVec2(200, 60))) {
        //                DLOG("rebooting\n");
        // #ifndef DESKTOP
        //                system("sync");
        //                system("reboot");
        // #endif
        //            }
    }
}

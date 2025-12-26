#include "langToString/langToString.h"
#include "skin.h"

#include <libintl.h>

void Skin::Misc() {
    ImGui::NewLine();
    if (!connector->storagePresent) {
        ImGui::Text(gettext("Disable USB mass storage mode"));
        return;
    }

    if (ImGui::BeginTable("##misctable", 4, ImGuiTableFlags_None)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Swap prev/next buttons"), &config->misc.swapTrackButtons)) {
            config->Save();
        }
        ImGui::TableNextColumn();
        ImGui::Text("     "); // padding

        ImGui::TableNextColumn();
        if (ImGui::Button(gettext("Export Walkman bookmarks"))) {
            ExportBookmarks();
            bookmarkExportStatus = gettext("Exported");
        }

        ImGui::TableNextColumn();
        ImGui::Text(bookmarkExportStatus.c_str());
        ImGui::SameLine(20);
        if (ImGui::InvisibleButton("##bookmarkStatus", ImVec2(246, 30))) {
            bookmarkExportStatus = "";
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Huge cover art"), &config->features.bigCover)) {
            config->Save();
            connector->FeatureBigCover(config->features.bigCover);
        }

        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        if (ImGui::Button(logCleanupButtonLabel.c_str())) {
            RemoveLogs();
            logCleanupStatus = gettext("Removed!");
            GetLogsDirSize();
        }

        ImGui::TableNextColumn();
        ImGui::Text(logCleanupStatus.c_str());
        ImGui::SameLine(20);
        if (ImGui::InvisibleButton("##logCleanupStatus", ImVec2(246, 30))) {
            logCleanupStatus = "";
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (isWalkmanOne) {
            ImGui::BeginDisabled();
            ImGui::Checkbox(gettext("Show time"), &alwaysFalse);
            ImGui::EndDisabled();
        } else {
            if (ImGui::Checkbox(gettext("Show time"), &config->features.showTime)) {
                config->Save();
                connector->FeatureShowTime(config->features.showTime);
            }
        }

        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        if (ImGui::Button(gettext("Set default volume tables"))) {
            config->volumeTables.ToneControl = "";
            config->volumeTables.MasterVolumeTable = "";
            config->volumeTables.MasterVolumeTableDSD = "";
            config->Save();
            resetVolTablesStatus = gettext("OK!");

            std::vector<std::vector<directoryEntry> *> tables = {&masterVolumeFiles, &masterVolumeDSDFiles, &toneControlFiles};
            for (auto &table : tables) {
                for (int i = 0; i < table->size(); i++) {
                    auto entry = table->at(i);
                    if (entry.name.rfind(defaultMark, 0) == 0) {
                        table->at(i).name.erase(0, defaultMark.length());
                    }
                }
            }
        }

        ImGui::TableNextColumn();
        ImGui::Text(resetVolTablesStatus.c_str());
        ImGui::SameLine(20);
        if (ImGui::InvisibleButton("##resetVolTablesStatus", ImVec2(246, 30))) {
            resetVolTablesStatus = "";
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Limit max volume"), &config->features.limitVolume)) {
            config->Save();
            connector->FeatureSetMaxVolume(config->features.limitVolume);
        }

        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Window position"));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Enable EQ per song"), &config->features.eqPerSong)) {
            config->Save();
        }

        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        if (ImGui::BeginCombo("##windowPos", gettext(WindowOffsetToString.at(config->windowOffset).c_str()), ImGuiComboFlags_HeightRegular)) {
            for (const auto &entry : WindowOffsetToString) {
                if (ImGui::Selectable(gettext(entry.second.c_str()), false)) {
                    config->windowOffset = entry.first;
                    DLOG("selected offset %s\n", WindowOffsetToString.at(config->windowOffset).c_str());
                    config->Save();
                    CalcWindowPos();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Disable touchscreen"), &config->features.touchscreenStaysOFF)) {
            config->Save();
        }

        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::Text("Language (need restart)");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Lock when screen is off"), &config->disableKeysWhenPowerOff)) {
            config->Save();
        }
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
        if (ImGui::BeginCombo("##lang", gettext(LangToString.at(config->language).c_str()), ImGuiComboFlags_HeightRegular)) {
            for (const auto &entry : LangToString) {
                if (ImGui::Selectable(gettext(entry.second.c_str()), false)) {
                    config->language = entry.first;
                    DLOG("selected lang %s\n", entry.first.c_str());
                    config->Save();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);

        ImGui::TableNextColumn();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Control filters"), &config->controlFilters)) {
            for (auto f : connector->soundSettingsFw.s->FilterStatus) {
                SetConfigFilter(f.name, f.is_proc);
            }
            config->Save();
        }
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Show FM tab"), &config->showFmInSettings)) {
            config->Save();
        }
        ImGui::TableNextColumn();

        ImGui::EndTable();
    }

    ImGui::NewLine();

    if (ImGui::Button(gettext("Debug"))) {
        displayTab = SettingsTab::TabDebug;
    }

    ImGui::SameLine();
    if (ImGui::Button(gettext("License"))) {
        displayTab = SettingsTab::TabLicense;
        loadStatusStr = "";
    }

    ImGui::SameLine();
    if (ImGui::Button(gettext("License 3rdparty"))) {
        displayTab = SettingsTab::TabLicense3rd;
        loadStatusStr = "";
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, GOLD_DONATE);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));
    if (ImGui::Button(gettext("Website / Donate"))) {
        displayTab = SettingsTab::TabWebsite;
        loadStatusStr = "";
    }
    ImGui::PopStyleColor(2);

    auto verSize = ImGui::CalcTextSize(SOFTWARE_VERSION);
    ImGui::SetCursorPosX(800 - verSize.x - ImGui::GetStyle().FramePadding.x);
    ImGui::SetCursorPosY(480 - verSize.y - ImGui::GetStyle().FramePadding.y);
    ImGui::Text("%s", SOFTWARE_VERSION);
}
#include "skin.h"
#include <libintl.h>

void Skin::Misc() {
    ImGui::NewLine();
    if (!connector->storagePresent) {
        ImGui::Text(gettext("Disable USB mass storage mode"));
        return;
    }

    if (ImGui::BeginTable("##misctable", 4, ImGuiTableFlags_None)) {
        // #ifndef DESKTOP
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
        if (ImGui::BeginCombo("##windowPos", WindowOffsetToString.at(config->windowOffset).c_str(), ImGuiComboFlags_HeightRegular)) {
            for (const auto &entry : WindowOffsetToString) {
                if (ImGui::Selectable(entry.second.c_str(), false)) {
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
        if (ImGui::Checkbox(gettext("Lock when screen is off"), &config->disableKeysWhenPowerOff)) {
            config->Save();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Control filters"), &config->controlFilters)) {
            for (auto f : connector->soundSettingsFw.s->FilterStatus) {
                SetConfigFilter(f.name, f.is_proc);
            }
            config->Save();
        }

        ImGui::TableNextColumn();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Debug"), &config->debug)) {
            winamp.debug = config->debug;
            cassette.debug = config->debug;
            config->Save();
        }
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Show FM tab"), &config->showFmInSettings)) {
            config->Save();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox(gettext("Limit fps"), &config->limitFPS)) {
            config->Save();
        }
        ImGui::TableNextColumn();

        ImGui::EndTable();
    }

    auto website = ImGui::CalcTextSize(gettext("Website / Donate"));
    auto verSize = ImGui::CalcTextSize(SOFTWARE_VERSION);
    auto licenseSize = ImGui::CalcTextSize(gettext("License"));
    auto license3Size = ImGui::CalcTextSize(gettext("License 3rdparty"));
    auto offset = 15.0f;

    // #ifndef DESKTOP
    if (config->debug) {

        ImGui::SetCursorPosY(480 - verSize.y - license3Size.y * 2 - ImGui::GetStyle().FramePadding.y * 3 - offset);
        if (ImGui::Button(gettext("Start ADB daemon (next boot)"))) {
            startADB();
        }

        ImGui::SetCursorPosY(480 - verSize.y - license3Size.y - ImGui::GetStyle().FramePadding.y * 2);
        if (ImGui::Button(gettext("Create log file"))) {
            createDump();
        }
    }
    // #endif
    ImGui::SetCursorPosY(480 - verSize.y - ImGui::GetStyle().FramePadding.y);
    printFPS();

    ImGui::SetCursorPosY(480 - verSize.y - licenseSize.y * 3 - ImGui::GetStyle().FramePadding.y * 2 - offset * 2 - 30);
    ImGui::SetCursorPosX(800 - website.x - ImGui::GetStyle().FramePadding.x - offset - 9);
    ImGui::PushStyleColor(ImGuiCol_Button, GOLD_DONATE);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));
    if (ImGui::Button(gettext("Website / Donate"), ImVec2(200, 60))) {
        displayTab = SettingsTab::TabWebsite;
        loadStatusStr = "";
    }
    ImGui::PopStyleColor(2);

    ImGui::SetCursorPosY(480 - verSize.y - licenseSize.y * 2 - ImGui::GetStyle().FramePadding.y * 2 - offset);
    ImGui::SetCursorPosX(800 - licenseSize.x - ImGui::GetStyle().FramePadding.x - offset);
    if (ImGui::Button(gettext("License"))) {
        displayTab = SettingsTab::TabLicense;
        loadStatusStr = "";
    }

    ImGui::SetCursorPosY(480 - verSize.y - license3Size.y - ImGui::GetStyle().FramePadding.y * 2);
    ImGui::SetCursorPosX(800 - license3Size.x - ImGui::GetStyle().FramePadding.x - offset);
    if (ImGui::Button(gettext("License 3rdparty"))) {
        displayTab = SettingsTab::TabLicense3rd;
        loadStatusStr = "";
    }

    ImGui::SetCursorPosX(800 - verSize.x - ImGui::GetStyle().FramePadding.x);
    ImGui::SetCursorPosY(480 - verSize.y - ImGui::GetStyle().FramePadding.y);
    ImGui::Text("%s", SOFTWARE_VERSION);
}
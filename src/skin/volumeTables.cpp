#include "implot_widgets.h"
#include "mkpath.h"
#include "skin.h"

#include <libintl.h>

void Skin::SoundSettingsTab() {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
    if (!connector->storagePresent) {
        ImGui::Text(gettext("Disable USB mass storage mode"));
        return;
    }

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("SoundSettingsTabBar", tab_bar_flags)) {
        TabMasterVolume();
        TabMasterDSDVolume();
        TabToneControl();
        TabSoundStatus();
        ImGui::EndTabBar();
    }
}


void Skin::MasterVolumeTableToImVec2() {
    for (int index = 0; index < 2; index++) {
        for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX + 1; tableID++) {
            for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
                for (int valType = MASTER_VOLUME_VALUE_MIN; valType < MASTER_VOLUME_VALUE_MAX; valType++) {
                    const auto val = masterVolume.GetValue(index, tableID, i, (MASTER_VOLUME_VALUE)valType);
                    //                        if (val != 0) {
                    //                            DLOG("Got %d -> %f, %f\n", val, (float)val, float(val));
                    //                        }
                    masterVolumeValues[index][tableID][valType][i].x = static_cast<float>(i);
                    masterVolumeValues[index][tableID][valType][i].y = static_cast<float>(val);
                }
            }
        }
    }
}

void Skin::MasterVolumeDSDTableToImVec2() {
    for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX + 1; tableID++) {
        for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
            const auto val = masterVolumeDSD.v[tableID][i];
            masterVolumeDSDValues[tableID][i] = ImVec2(static_cast<float>(i), static_cast<float>(val));
        }
    }
}

void Skin::ToneControlToImVec2() {
    for (int tableID = 0; tableID < TONE_CONTROL_TABLE_MAX + 1; tableID++) {
        for (int i = 0; i < CODEC_RAM_SIZE; i++) {
            const auto val = toneControl.v[tableID][i];
            toneControlValues[tableID][i] = ImVec2(static_cast<float>(i), static_cast<float>(val));
        }
    }
}

void Skin::MasterVolumeImVec2ToTable() {
    for (int index = 0; index < 2; index++) {
        for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX + 1; tableID++) {
            for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
                for (int valType = 1; valType < MASTER_VOLUME_VALUE_MAX; valType++) {
                    const auto val = masterVolumeValues[index][tableID][valType][i].y;
                    masterVolume.SetValue(index, tableID, i, static_cast<MASTER_VOLUME_VALUE>(valType), static_cast<uint>(val));
                }
            }
        }
    }
}

void Skin::MasterVolumeDSDImVec2ToTable() {
    for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX + 1; tableID++) {
        for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
            for (int valType = 1; valType < MASTER_VOLUME_VALUE_MAX + 1; valType++) {
                const auto val = masterVolumeDSDValues[tableID][i];
                masterVolumeDSD.v[tableID][i] = static_cast<int>(val.y);
            }
        }
    }
}

void Skin::ToneControlImVec2ToTable() {
    for (int tableID = 0; tableID < TONE_CONTROL_TABLE_MAX + 1; tableID++) {
        for (int i = 0; i < CODEC_RAM_SIZE; i++) {
            const auto val = toneControlValues[tableID][i];
            toneControl.v[tableID][i] = static_cast<int>(val.y);
        }
    }
}

void Skin::WithMasterVolumeTableDirs(const std::string &d) { masterVolumeTableDirectories.push_back(d); }

void Skin::RefreshMasterVolumeTableFiles() {
    DLOG("\n");
    masterVolumeFiles.clear();
    for (const auto &d : masterVolumeTableDirectories) {
        listdir(d.c_str(), &masterVolumeFiles, ".tbl");
    }

    if (masterVolumeFiles.empty()) {
        listdir("/system/usr/share/audio_dac/", &masterVolumeFiles, ".tbl");
    }
}

void Skin::WithMasterVolumeTableDSDDirs(const std::string &d) { masterVolumeTableDSDDirectories.push_back(d); }

void Skin::RefreshMasterVolumeTableDSDFiles() {
    DLOG("\n");
    masterVolumeDSDFiles.clear();
    for (const auto &d : masterVolumeTableDSDDirectories) {
        listdir(d.c_str(), &masterVolumeDSDFiles, ".tbl");
    }

    if (masterVolumeDSDFiles.empty()) {
        listdir("/system/usr/share/audio_dac/", &masterVolumeDSDFiles, ".tbl");
    }
}

void Skin::WithToneControlTableDirs(const std::string &d) { toneControlTableDirectories.push_back(d); }

void Skin::RefreshToneControlFiles() {
    DLOG("\n");
    toneControlFiles.clear();
    for (const auto &d : toneControlTableDirectories) {
        listdir(d.c_str(), &toneControlFiles, ".tbl");
    }

    if (toneControlFiles.empty()) {
        listdir("/system/usr/share/audio_dac/", &toneControlFiles, ".tbl");
    }
}

void Skin::PreprocessTableFilenames() {
    std::vector<std::vector<directoryEntry> *> tables = {&masterVolumeFiles, &masterVolumeDSDFiles, &toneControlFiles};
    for (auto &table : tables) {
        for (int i = 0; i < table->size(); i++) {
            auto entry = table->at(i);
            if (entry.fullPath.rfind(soundSettingsPathSystemWampy, 0) == 0) {
                table->at(i).name = systemMark + entry.name;
                continue;
            }
            if (entry.fullPath.rfind(soundSettingsPathSystemHagoromo, 0) == 0) {
                table->at(i).name = systemMarkHagoromo + entry.name;
                continue;
            }
        }
    }

    for (int i = 0; i < masterVolumeFiles.size(); i++) {
        if (masterVolumeFiles.at(i).fullPath == config->volumeTables.MasterVolumeTable) {
            masterVolumeFiles.at(i).name = defaultMark + masterVolumeFiles.at(i).name;
            break;
        }
    }

    for (int i = 0; i < masterVolumeDSDFiles.size(); i++) {
        if (masterVolumeDSDFiles.at(i).fullPath == config->volumeTables.MasterVolumeTableDSD) {
            masterVolumeDSDFiles.at(i).name = defaultMark + masterVolumeDSDFiles.at(i).name;
            break;
        }
    }

    for (int i = 0; i < toneControlFiles.size(); i++) {
        if (toneControlFiles.at(i).fullPath == config->volumeTables.ToneControl) {
            toneControlFiles.at(i).name = defaultMark + toneControlFiles.at(i).name;
            break;
        }
    }
}

void Skin::CopyTableEntry(TableLike *table, std::vector<directoryEntry> *fileList, int *selectedIndex, const std::string &outDir) {
    mkpath(outDir.c_str(), 0755);

    auto d = directoryEntry{};
    d.name = basename(fileList->at(*selectedIndex).fullPath.c_str());
    d.fullPath = outDir + d.name;
    d.valid = true;

    DLOG("copying to %s\n", d.fullPath.c_str());

    table->ToFile(d.fullPath);

    bool exists{};
    for (int i = 0; i < fileList->size(); i++) {
        if (fileList->at(i).fullPath == d.fullPath) {
            exists = true;
            *selectedIndex = i;
            break;
        }
    }

    if (!exists) {
        fileList->push_back(d);
        *selectedIndex = (int)fileList->size() - 1;
    }
}

void Skin::TabMasterVolume() {
    if (ImGui::BeginTabItem(Dac::TableTypeToString.at(TABLE_ID_MASTER_VOLUME).c_str())) {
        curveEditorTarget = (float *)masterVolumeValues[int(soundEffectOn)][MasterVolumeTableType][MasterVolumeValueType];
        curveYLimit = (1 << 8) - 1; // 255
        curveElementCount = MASTER_VOLUME_MAX + 1;

        if (ImGui::BeginTable("##mvtable", 3, ImGuiTableFlags_None)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, 388);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("File:"));

            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            ImGui::PushItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##masterVolumeFileCombo", masterVolumeFiles.at(masterVolumeFileSelected).name.c_str(), ImGuiComboFlags_HeightRegular)) {
                for (int idx = 0; idx < masterVolumeFiles.size(); idx++) {
                    auto entry = masterVolumeFiles.at(idx);
                    if (ImGui::Selectable(entry.name.c_str(), false)) {
                        masterVolumeFileSelected = idx;
                        DLOG("selected file %s\n", entry.name.c_str());
                        masterVolume.Reset();
                        memset(&masterVolumeValues, 0, sizeof(masterVolumeValues));
                        if (masterVolume.FromFile(entry.fullPath) != 0) {
                            DLOG("failed to load master volume table file %s\n", entry.fullPath.c_str());
                            statusStringMasterVolume = gettext("Failed");
                        } else {
                            MasterVolumeTableToImVec2();
                            statusStringMasterVolume = gettext("Loaded");
                        }
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::PopStyleVar(2);

            ImGui::TableNextColumn();

            if (!statusStringMasterVolume.empty()) {
                ImGui::Text(statusStringMasterVolume.c_str());
                ImGui::SameLine(20);
                if (ImGui::InvisibleButton("##statusmastervolume", ImVec2(246, 30))) {
                    statusStringMasterVolume = "";
                }
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Table type:"));
            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            const char *preview = Dac::MasterVolumeTableTypeToString.at(MasterVolumeTableType).c_str();
            ImGui::PushItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##masterVolumeTypeCombo", preview, ImGuiComboFlags_HeightRegular)) {
                for (const auto &entry : Dac::MasterVolumeTableTypeToString) {
                    if (ImGui::Selectable(entry.second.c_str(), false)) {
                        MasterVolumeTableType = entry.first;
                        DLOG("selected type %s\n", entry.second.c_str());
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::PopStyleVar(2);
            ImGui::TableNextColumn();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Value type:"));
            ImGui::TableNextColumn();

            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            ImGui::PushItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo(
                    "##masterVolumeValueTypeCombo", Dac::MasterVolumeValueTypeToString.at(MasterVolumeValueType).c_str(), ImGuiComboFlags_HeightRegular)) {
                for (const auto &entry : Dac::MasterVolumeValueTypeToString) {
                    if (ImGui::Selectable(entry.second.c_str(), false)) {
                        MasterVolumeValueType = entry.first;
                        DLOG("selected value type %s (%d)\n", entry.second.c_str(), entry.first);
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::PopStyleVar(2);

            ImGui::TableNextColumn();
            ImGui::Checkbox(gettext("Sound effect"), &soundEffectOn);
        }
        ImGui::EndTable();

        if (ImGui::BeginTable("##mvtcontent", 2, ImGuiTableFlags_None)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, 512);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (masterVolumeValues[0][0][0][0].x == -FLT_MIN) {
                ImGui::NewLine();
                if (ImGui::Button(gettext("Load"), ImVec2(512, 150))) {
                    if (masterVolume.FromFile(masterVolumeFiles.at(masterVolumeFileSelected).fullPath) != 0) {
                        DLOG("failed to load master volume table file %s\n", masterVolumeFiles.at(masterVolumeFileSelected).fullPath.c_str());
                        statusStringMasterVolume = gettext("Failed");
                    } else {
                        MasterVolumeTableToImVec2();
                        statusStringMasterVolume = gettext("Loaded");
                    }
                }
            } else {
                auto flags = ImPlotFlags_None;
                if (ImPlot::BeginPlot("##lines", ImVec2(512, 270), ImPlotFlags_NoMouseText | ImPlotFlags_NoInputs)) {
                    ImPlot::SetupAxesLimits(
                        curveElementCount * -0.05, curveElementCount + curveElementCount * 0.05, curveYLimit * -0.05, curveYLimit + curveYLimit * 0.1);
                    ImPlot::SetNextLineStyle(GOLD_DONATE, 2.0f);
                    ImPlot::PlotInfLines("##volline", &connector->status.VolumeRaw, 1);
                    ImPlot::SetNextLineStyle(IMPLOT_AUTO_COL, 2.0f);
                    ImPlot::PlotLineG("##curvaSmall", ImPlot::ImVec2Getter, curveEditorTarget, curveElementCount);
                    ImPlot::EndPlot();
                }

                ImGui::TableNextColumn();

                // not efficient at all
                if (masterVolumeFiles.at(masterVolumeFileSelected).name.rfind(systemMark, 0) == 0) {
                    if (ImGui::Button(gettext("Copy and edit"), ImVec2(246, 60))) {
                        auto outDir = soundSettingsPathUser + "master_volume/";
                        CopyTableEntry(&masterVolume, &masterVolumeFiles, &masterVolumeFileSelected, outDir);

                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                } else {
                    if (ImGui::Button(gettext("Edit"), ImVec2(246, 60))) {

                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                }

                if (masterVolumeFiles.at(masterVolumeFileSelected).name.rfind(systemMark, 0) == 0) {
                    ImGui::BeginDisabled();
                }

                if (ImGui::Button(gettext("Save"), ImVec2(121, 60))) {
                    auto out = masterVolumeFiles.at(masterVolumeFileSelected).fullPath;
                    DLOG("Saving to %s\n", out.c_str());
                    MasterVolumeImVec2ToTable();
                    if (masterVolume.ToFile(out) == 0) {
                        statusStringMasterVolume = gettext("Saved");
                    } else {
                        statusStringMasterVolume = gettext("Failed");
                    }
                }

                if (masterVolumeFiles.at(masterVolumeFileSelected).name.rfind(systemMark, 0) == 0) {
                    ImGui::EndDisabled();
                }

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
                ImGui::SameLine();

                if (ImGui::Button(gettext("Apply"), ImVec2(121, 60))) {
                    DLOG("Applying\n");
                    MasterVolumeImVec2ToTable();
                    if (masterVolume.Apply(Dac::volumeTableOutPath) == 0) {
                        statusStringMasterVolume = gettext("Applied");
                    } else {
                        statusStringMasterVolume = gettext("Failed");
                    }
                }

                if (ImGui::Button(gettext("Copy val"), ImVec2(121, 60))) {
                    memcpy(masterVolumeValueBuffer,
                        masterVolumeValues[(int)soundEffectOn][MasterVolumeTableType][MasterVolumeValueType],
                        sizeof(masterVolumeValueBuffer));
                    statusStringMasterVolume = gettext("Copied");
                }

                ImGui::SameLine();
                if (ImGui::Button(gettext("Paste val"), ImVec2(121, 60))) {
                    memcpy(masterVolumeValues[(int)soundEffectOn][MasterVolumeTableType][MasterVolumeValueType],
                        masterVolumeValueBuffer,
                        sizeof(masterVolumeValueBuffer));
                    statusStringMasterVolume = gettext("Pasted");
                }

                if (ImGui::Button(gettext("Set as default"), ImVec2(246, 60))) {
                    DLOG("Setting master volume table %s as default\n", masterVolumeFiles.at(masterVolumeFileSelected).fullPath.c_str());
                    config->volumeTables.MasterVolumeTable = masterVolumeFiles.at(masterVolumeFileSelected).fullPath;
                    config->Save();

                    for (int i = 0; i < masterVolumeFiles.size(); i++) {
                        if (masterVolumeFiles.at(i).name.rfind(defaultMark, 0) == 0) {
                            masterVolumeFiles.at(i).name.erase(0, defaultMark.length());
                        }
                        if (i == masterVolumeFileSelected) {
                            masterVolumeFiles.at(i).name = defaultMark + masterVolumeFiles.at(i).name;
                        }
                    }

                    statusStringMasterVolume = gettext("Set as default");
                }

                ImGui::PopStyleVar();
            }
            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
}

void Skin::TabMasterDSDVolume() {
    if (ImGui::BeginTabItem(Dac::TableTypeToString.at(TABLE_ID_MASTER_VOLUME_DSD).c_str())) {
        curveEditorTarget = (float *)masterVolumeDSDValues[MasterVolumeDSDTableType];
        curveYLimit = (1 << 15) - 1; // 32767
        curveElementCount = MASTER_VOLUME_MAX + 1;

        if (ImGui::BeginTable("##mvDSDtable", 3, ImGuiTableFlags_None)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, 384);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("File:"));

            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            ImGui::PushItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo(
                    "##masterVolumeDSDFileCombo", masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).name.c_str(), ImGuiComboFlags_HeightRegular)) {
                for (int idx = 0; idx < masterVolumeDSDFiles.size(); idx++) {
                    auto entry = masterVolumeDSDFiles.at(idx);
                    if (ImGui::Selectable(entry.name.c_str(), false)) {
                        DLOG("selected dsd file %s\n", entry.fullPath.c_str());
                        masterVolumeDSDFileSelected = idx;
                        masterVolumeDSD.Reset();
                        memset(&masterVolumeDSDValues, 0, sizeof(masterVolumeDSDValues));
                        if (masterVolumeDSD.FromFile(entry.fullPath) != 0) {
                            DLOG("failed to load master volume dsd table file %s\n", entry.fullPath.c_str());
                            statusStringMasterVolumeDSD = gettext("Failed");
                        } else {
                            MasterVolumeDSDTableToImVec2();
                            statusStringMasterVolumeDSD = gettext("Loaded");
                        }
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::PopStyleVar(2);

            ImGui::TableNextColumn();
            if (!statusStringMasterVolumeDSD.empty()) {
                ImGui::Text(statusStringMasterVolumeDSD.c_str());
                ImGui::SameLine(20);
                if (ImGui::InvisibleButton("##statusmastervolumeDSD", ImVec2(246, 30))) {
                    statusStringMasterVolumeDSD = "";
                }
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Table type:"));

            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            ImGui::PushItemWidth(-FLT_MIN);
            const char *preview = Dac::MasterVolumeTableTypeToString.at(MasterVolumeDSDTableType).c_str();
            if (ImGui::BeginCombo("##masterVolumeDSDTypeCombo", preview, ImGuiComboFlags_HeightRegular)) {
                for (const auto &entry : Dac::MasterVolumeTableTypeToString) {
                    if (ImGui::Selectable(entry.second.c_str(), false)) {
                        MasterVolumeDSDTableType = entry.first;
                        DLOG("selected dsd type %s\n", entry.second.c_str());
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::PopStyleVar(2);
            ImGui::TableNextColumn();

            ImGui::EndTable();
        }

        if (ImGui::BeginTable("##mvtDSDcontent", 2, ImGuiTableFlags_None)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (masterVolumeDSDValues[0][0].x == -FLT_MIN) {
                ImGui::NewLine();
                if (ImGui::Button(gettext("Load"), ImVec2(512, 150))) {
                    if (masterVolumeDSD.FromFile(masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).fullPath) != 0) {
                        DLOG("failed to load master volume DSD table file %s\n", masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).fullPath.c_str());
                        statusStringMasterVolumeDSD = gettext("Failed");
                    } else {
                        MasterVolumeDSDTableToImVec2();
                        statusStringMasterVolumeDSD = gettext("Loaded");
                    }
                }
            } else {
                if (ImPlot::BeginPlot("##lines", ImVec2(512, 305), ImPlotFlags_NoMouseText | ImPlotFlags_NoInputs)) {
                    ImPlot::SetupAxesLimits(
                        curveElementCount * -0.05, curveElementCount + curveElementCount * 0.05, curveYLimit * -0.05, curveYLimit + curveYLimit * 0.1);
                    ImPlot::SetNextLineStyle(GOLD_DONATE, 2.0f);
                    ImPlot::PlotInfLines("##volline", &connector->status.VolumeRaw, 1);
                    ImPlot::SetNextLineStyle(IMPLOT_AUTO_COL, 2.0f);
                    ImPlot::PlotLineG("##curvaSmall", ImPlot::ImVec2Getter, curveEditorTarget, curveElementCount);
                    ImPlot::EndPlot();
                }

                ImGui::TableNextColumn();

                // not efficient at all
                if (masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).name.rfind(systemMark, 0) == 0) {
                    if (ImGui::Button(gettext("Copy and edit"), ImVec2(246, 60))) {
                        auto outDir = soundSettingsPathUser + "master_volume_dsd/";
                        CopyTableEntry(&masterVolumeDSD, &masterVolumeDSDFiles, &masterVolumeDSDFileSelected, outDir);

                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                } else {
                    if (ImGui::Button(gettext("Edit"), ImVec2(246, 60))) {

                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                }

                if (masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).name.rfind(systemMark, 0) == 0) {
                    ImGui::BeginDisabled();
                }

                if (ImGui::Button(gettext("Save"), ImVec2(121, 60))) {
                    auto out = masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).fullPath;
                    DLOG("Saving to %s\n", out.c_str());
                    MasterVolumeDSDImVec2ToTable();
                    if (masterVolumeDSD.ToFile(out) == 0) {
                        statusStringMasterVolumeDSD = gettext("Saved");
                    } else {
                        statusStringMasterVolumeDSD = gettext("Failed");
                    }
                }

                if (masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).name.rfind(systemMark, 0) == 0) {
                    ImGui::EndDisabled();
                }
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
                ImGui::SameLine();

                if (ImGui::Button(gettext("Apply"), ImVec2(121, 60))) {
                    DLOG("Applying to %s\n", Dac::volumeTableDSDOutPath.c_str());
                    MasterVolumeDSDImVec2ToTable();
                    if (masterVolumeDSD.Apply(Dac::volumeTableDSDOutPath) == 0) {
                        statusStringMasterVolumeDSD = gettext("Applied");
                    } else {
                        statusStringMasterVolumeDSD = gettext("Failed");
                    }
                }

                if (ImGui::Button("Copy val", ImVec2(121, 60))) {
                    memcpy(masterVolumeDSDValueBuffer, masterVolumeDSDValues[MasterVolumeDSDTableType], sizeof(masterVolumeDSDValueBuffer));
                    statusStringMasterVolumeDSD = "Copied";
                }

                ImGui::SameLine();
                if (ImGui::Button(gettext("Paste val"), ImVec2(121, 60))) {
                    memcpy(masterVolumeDSDValues[MasterVolumeDSDTableType], masterVolumeDSDValueBuffer, sizeof(masterVolumeDSDValueBuffer));
                    statusStringMasterVolumeDSD = gettext("Pasted");
                }

                if (ImGui::Button(gettext("Set as default"), ImVec2(246, 60))) {
                    DLOG("Setting master volume DSD table %s as default\n", masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).fullPath.c_str());
                    config->volumeTables.MasterVolumeTableDSD = masterVolumeDSDFiles.at(masterVolumeDSDFileSelected).fullPath;
                    config->Save();

                    for (int i = 0; i < masterVolumeDSDFiles.size(); i++) {
                        if (masterVolumeDSDFiles.at(i).name.rfind(defaultMark, 0) == 0) {
                            masterVolumeDSDFiles.at(i).name.erase(0, defaultMark.length());
                        }
                        if (i == masterVolumeDSDFileSelected) {
                            masterVolumeDSDFiles.at(i).name = defaultMark + masterVolumeDSDFiles.at(i).name;
                        }
                    }

                    statusStringMasterVolumeDSD = gettext("Set as default");
                }

                ImGui::PopStyleVar();
            }

            ImGui::EndTable();
        }

        ImGui::EndTabItem();
    }
}

void Skin::TabToneControl() {
    if (ImGui::BeginTabItem(Dac::TableTypeToString.at(TABLE_ID_TONE_CONTROL).c_str())) {
        curveEditorTarget = (float *)toneControlValues[toneControlTableType];
        curveYLimit = (1 << 8) - 1; // 255
        curveElementCount = CODEC_RAM_SIZE;

        if (ImGui::BeginTable("##mvtable", 3, ImGuiTableFlags_None)) {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, 384);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("File:"));

            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            ImGui::PushItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##toneControlFileCombo", toneControlFiles.at(toneControlFileSelected).name.c_str(), ImGuiComboFlags_HeightRegular)) {
                for (int idx = 0; idx < toneControlFiles.size(); idx++) {
                    auto entry = toneControlFiles.at(idx);
                    if (ImGui::Selectable(entry.name.c_str(), false)) {
                        DLOG("selected tone control file %s\n", entry.fullPath.c_str());
                        toneControlFileSelected = idx;
                        toneControl.Reset();
                        memset(&toneControlValues, 0, sizeof(toneControlValues));
                        if (toneControl.FromFile(entry.fullPath) != 0) {
                            DLOG("failed to load tone control table file %s\n", entry.fullPath.c_str());
                            statusStringToneControl = gettext("Failed");
                        } else {
                            ToneControlToImVec2();
                            statusStringToneControl = gettext("Loaded");
                        }
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::PopItemWidth();
            ImGui::PopStyleVar(2);

            ImGui::TableNextColumn();
            if (!statusStringToneControl.empty()) {
                ImGui::Text(statusStringToneControl.c_str());
                ImGui::SameLine(20);
                if (ImGui::InvisibleButton("##statusToneControl", ImVec2(246, 30))) {
                    statusStringToneControl = "";
                }
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Table type:"));

            ImGui::TableNextColumn();
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
            const char *preview;
            preview = Dac::ToneControlTableTypeToString.at(toneControlTableType).c_str();
            ImGui::PushItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##toneControlTypeCombo", preview, ImGuiComboFlags_HeightRegular)) {
                for (const auto &entry : Dac::ToneControlTableTypeToString) {
                    if (ImGui::Selectable(entry.second.c_str(), false)) {
                        toneControlTableType = entry.first;
                        DLOG("selected tone control type %s\n", entry.second.c_str());
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::PopStyleVar(2);

            ImGui::EndTable();
        }

        if (ImGui::BeginTable("##ttcontent", 2, ImGuiTableFlags_None)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (toneControlValues[0][0].x == -FLT_MIN) {
                ImGui::NewLine();
                if (ImGui::Button(gettext("Load"), ImVec2(512, 150))) {
                    if (toneControl.FromFile(toneControlFiles.at(toneControlFileSelected).fullPath) != 0) {
                        DLOG("failed to load tone control table file %s\n", toneControlFiles.at(toneControlFileSelected).fullPath.c_str());
                        statusStringToneControl = gettext("Failed");
                    } else {
                        ToneControlToImVec2();
                        statusStringToneControl = gettext("Loaded");
                    }
                }
            } else {
                if (ImPlot::BeginPlot("##lines", ImVec2(512, 305), ImPlotFlags_NoMouseText | ImPlotFlags_NoInputs)) {
                    ImPlot::SetupAxesLimits(
                        curveElementCount * -0.05, curveElementCount + curveElementCount * 0.05, curveYLimit * -0.05, curveYLimit + curveYLimit * 0.1);
                    ImPlot::SetNextLineStyle(GOLD_DONATE, 2.0f);
                    ImPlot::PlotLineG("##curvaSmall", ImPlot::ImVec2Getter, curveEditorTarget, curveElementCount);
                    ImPlot::EndPlot();
                }

                ImGui::TableNextColumn();

                // not efficient at all
                if (toneControlFiles.at(toneControlFileSelected).name.rfind(systemMark, 0) == 0) {
                    if (ImGui::Button(gettext("Copy and edit"), ImVec2(246, 60))) {
                        auto outDir = soundSettingsPathUser + "tone_control/";
                        CopyTableEntry(&toneControl, &toneControlFiles, &toneControlFileSelected, outDir);

                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                } else {
                    if (ImGui::Button(gettext("Edit"), ImVec2(246, 60))) {
                        for (int i = 0; i < curveElementCount; i++) {
                            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
                            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
                        }

                        zoomLimitXL = (double)curveElementCount * -0.05;
                        zoomLimitXR = (double)curveElementCount * 1.05;
                        zoomLimitYT = (double)curveYLimit * 1.05;
                        zoomLimitYB = (double)curveYLimit * -0.05;

                        displayTab = TabCurveEditor;
                    }
                }

                if (toneControlFiles.at(toneControlFileSelected).name.rfind(systemMark, 0) == 0) {
                    ImGui::BeginDisabled();
                }

                if (ImGui::Button(gettext("Save"), ImVec2(121, 60))) {
                    auto out = toneControlFiles.at(toneControlFileSelected).fullPath;
                    DLOG("Saving to %s\n", out.c_str());
                    ToneControlImVec2ToTable();
                    if (toneControl.ToFile(out) == 0) {
                        statusStringToneControl = gettext("Saved");
                    } else {
                        statusStringToneControl = gettext("Failed");
                    }
                }

                if (toneControlFiles.at(toneControlFileSelected).name.rfind(systemMark, 0) == 0) {
                    ImGui::EndDisabled();
                }

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
                ImGui::SameLine();

                if (ImGui::Button(gettext("Apply"), ImVec2(121, 60))) {
                    DLOG("Applying to %s\n", Dac::toneControlOutPath.c_str());
                    ToneControlImVec2ToTable();
                    if (toneControl.Apply(Dac::toneControlOutPath) == 0) {
                        statusStringToneControl = gettext("Applied");
                    } else {
                        statusStringToneControl = gettext("Failed");
                    }
                }

                if (ImGui::Button(gettext("Copy val"), ImVec2(121, 60))) {
                    memcpy(toneControlValueBuffer, toneControlValues[toneControlTableType], sizeof(toneControlValueBuffer));
                    statusStringToneControl = gettext("Copied");
                }

                ImGui::SameLine();
                if (ImGui::Button(gettext("Paste val"), ImVec2(121, 60))) {
                    memcpy(toneControlValues[toneControlTableType], toneControlValueBuffer, sizeof(toneControlValueBuffer));
                    statusStringToneControl = gettext("Pasted");
                }

                if (ImGui::Button(gettext("Set as default"), ImVec2(246, 60))) {
                    DLOG("Setting tone control table %s as default\n", toneControlFiles.at(toneControlFileSelected).fullPath.c_str());
                    config->volumeTables.ToneControl = toneControlFiles.at(toneControlFileSelected).fullPath;
                    config->Save();

                    for (int i = 0; i < toneControlFiles.size(); i++) {
                        if (toneControlFiles.at(i).name.rfind(defaultMark, 0) == 0) {
                            toneControlFiles.at(i).name.erase(0, defaultMark.length());
                        }
                        if (i == toneControlFileSelected) {
                            toneControlFiles.at(i).name = defaultMark + toneControlFiles.at(i).name;
                        }
                    }

                    statusStringToneControl = gettext("Set as default");
                }

                ImGui::PopStyleVar();
            }

            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
}

void Skin::TabSoundStatus() {
    if (ImGui::BeginTabItem(gettext("Status"))) {
        ImGui::NewLine();
        if (ImGui::BeginTable("##soundstatustable", 2, ImGuiTableFlags_None)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Button(gettext("Refresh"), ImVec2(240, 60))) {
                statusMasterVTFile = Dac::getStatus(&masterVolumeFiles, Dac::volumeTableOutPath);
                statusMasterVTDSDFile = Dac::getStatus(&masterVolumeDSDFiles, Dac::volumeTableDSDOutPath);
                statusToneControlFile = Dac::getStatus(&toneControlFiles, Dac::toneControlOutPath);
                deviceProduct = GetProduct();
                deviceModelID = GetModelID();
                deviceRegionID = GetRegionID();
                deviceRegionStr = GetRegionIDStr();
                deviceAudioInUse = AudioDeviceInUse();
                minCpuFreq = ReadFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
                rstrip(&minCpuFreq, '\n');
                curCpuFreq = ReadFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
                rstrip(&curCpuFreq, '\n');
                freqStr = minCpuFreq + " (" + curCpuFreq + ") Hz";
                deviceAudioSampleRate = CardSampleRate();

                refreshStatus = gettext("Refreshed");
            }

            ImGui::TableNextColumn();

            ImGui::Text(refreshStatus.c_str());
            ImGui::SameLine(20);
            if (ImGui::InvisibleButton("##refreshStatus", ImVec2(246, 30))) {
                refreshStatus = "";
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Model ID"));
            ImGui::TableNextColumn();
            ImGui::Text(deviceModelID.c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Region ID"));
            ImGui::TableNextColumn();
            if (!deviceRegionID.empty() && !deviceRegionStr.empty()) {
                ImGui::Text("%s (%s)", deviceRegionID.c_str(), deviceRegionStr.c_str());
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Device product"));
            ImGui::TableNextColumn();
            ImGui::Text(deviceProduct.c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Master volume table"));
            ImGui::TableNextColumn();
            ImGui::Text(statusMasterVTFile.c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Master volume table (DSD)"));
            ImGui::TableNextColumn();
            ImGui::Text(statusMasterVTDSDFile.c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Tone control"));
            ImGui::TableNextColumn();
            ImGui::Text(statusToneControlFile.c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Audio card"));
            ImGui::TableNextColumn();
            ImGui::Text(deviceAudioInUse.first.c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Audio device"));
            ImGui::TableNextColumn();
            ImGui::Text(deviceAudioInUse.second.c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("Sample rate"));
            ImGui::TableNextColumn();
            ImGui::Text(deviceAudioSampleRate.c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(gettext("CPU frequency (min/cur)"));
            ImGui::TableNextColumn();
            ImGui::Text(freqStr.c_str(), ImVec2(200, 100));

            ImGui::EndTable();
        }

        ImGui::EndTabItem();
    }
}

void Skin::CurveEditor() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
    ImGui::BeginChild("##curvaBig",
        ImVec2(windowSize.x - 15 * 2 - (ImGui::GetStyle().FramePadding.x * 2.0f + 98 * 2 + ImGui::GetStyle().ItemSpacing.x * 2), 480),
        ImGuiChildFlags_None,
        window_flags);

    bool clicked = false;
    bool hovered = false;
    bool held[curveElementCount];

    for (int i = 0; i < curveElementCount; i++) {
        valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
    }

    auto plotWidth = windowSize.x - 15 * 2 - (ImGui::GetStyle().FramePadding.x * 2.0f + 98 * 2 + ImGui::GetStyle().ItemSpacing.x * 2);
    if (ImPlot::BeginPlot("##lines", ImVec2(plotWidth, 480), ImPlotFlags_NoMouseText | ImPlotFlags_NoBoxSelect)) {
        ImPlot::SetupAxesLimits(zoomLimitXL, zoomLimitXR, zoomLimitYB, zoomLimitYT, ImPlotCond_Always);

        float radius = 10;
        for (int i = 0; i < curveElementCount; i++) {
            ImPlot::DragPoint(i * 134, &valuesx[i], &valuesy[i], GOLD_DONATE, radius, ImPlotDragToolFlags_None, &clicked, &hovered, &held[i]);
            if (held[i]) {
                auto posx = double(((ImVec2 *)curveEditorTarget + i)->x);
                float offset = 70;
                if ((plotWidth - ImGui::GetMousePos().x) < 120) {
                    offset = -70;
                }
                ImPlot::Annotation(posx, valuesy[i], IMPLOT_AUTO_COL, ImVec2(offset, 10), false, true);
            }

            if (valuesy[i] > curveYLimit) {
                valuesy[i] = curveYLimit;
            }

            if (valuesy[i] < 0) {
                valuesy[i] = 0;
            }
        }

        ImPlot::SetNextLineStyle(IMPLOT_AUTO_COL, 2.0f);
        ImPlot::PlotLine("##curvaBig", valuesx, valuesy, curveElementCount);
        ImPlot::EndPlot();
    }

    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("##curvaBigButtons", ImVec2(windowSize.x - ImGui::GetCursorPosX(), 480), ImGuiChildFlags_None, window_flags);

    if (ImGui::Button(gettext("Zoom in"), ImVec2(110, 60))) {
        zoomLimitXL = zoomLimitXL + curveElementCount * 0.06;
        zoomLimitXR = zoomLimitXR - curveElementCount * 0.06;
        zoomLimitYB = zoomLimitYB + curveYLimit * 0.06;
        zoomLimitYT = zoomLimitYT - curveYLimit * 0.06;
    }

    ImGui::SameLine();
    if (ImGui::Button(gettext("Zoom out"), ImVec2(110, 60))) {
        zoomLimitXL = zoomLimitXL - curveElementCount * 0.07;
        zoomLimitXR = zoomLimitXR + curveElementCount * 0.07;
        zoomLimitYB = zoomLimitYB - curveYLimit * 0.07;
        zoomLimitYT = zoomLimitYT + curveYLimit * 0.07;
    }

    ImGui::NewLine();
    if (ImGui::Button(gettext("Left"), ImVec2(110, 60))) {
        zoomLimitXL = zoomLimitXL - curveElementCount * 0.1;
        zoomLimitXR = zoomLimitXR - curveElementCount * 0.1;
    }

    ImGui::SameLine();
    if (ImGui::Button(gettext("Right"), ImVec2(110, 60))) {
        zoomLimitXL = zoomLimitXL + curveElementCount * 0.09;
        zoomLimitXR = zoomLimitXR + curveElementCount * 0.09;
    }

    ImGui::NewLine();
    if (ImGui::Button(gettext("Up"), ImVec2(110, 60))) {
        zoomLimitYB = zoomLimitYB + curveYLimit * 0.1;
        zoomLimitYT = zoomLimitYT + curveYLimit * 0.1;
    }

    ImGui::SameLine();
    if (ImGui::Button(gettext("Down"), ImVec2(110, 60))) {
        zoomLimitYB = zoomLimitYB - curveYLimit * 0.09;
        zoomLimitYT = zoomLimitYT - curveYLimit * 0.09;
    }

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();
    ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_RED);
    if (ImGui::Button(gettext("Reset"), ImVec2(230, 60))) {
        for (int i = 0; i < curveElementCount; i++) {
            valuesx[i] = double(((ImVec2 *)(curveEditorTarget) + i)->x);
            valuesy[i] = double(((ImVec2 *)(curveEditorTarget) + i)->y);
        }
    }
    ImGui::PopStyleColor();
    ImGui::NewLine();
    if (ImGui::Button(gettext("Back"), ImVec2(230, 60))) {
        for (int i = 0; i < curveElementCount; i++) {
            auto point = ((ImVec2 *)(curveEditorTarget) + i);
            point->y = static_cast<float>(valuesy[i]);
        }
        displayTab = TabSoundSettings;
    }

    ImGui::EndChild();
}

#include "skin.h"
#include <libintl.h>

void Skin::TabEQOld() {
    ImGui::NewLine();
    if (!connector->storagePresent) {
        ImGui::Text(gettext("Disable USB mass storage mode"));
        return;
    }

    if (!config->features.eqPerSong) {
        ImGui::Text(gettext("Feature disabled. Toggle it on 'Misc' tab or use button below."));
        ImGui::NewLine();
        if (ImGui::Button(gettext("Enable"), ImVec2(186, 60))) {
            config->features.eqPerSong = true;
            config->Save();
        }
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::BeginChild("##textwrap", ImVec2(800 - 20 - 10, ImGui::GetTextLineHeight() * 2 + 10), ImGuiWindowFlags_NoResize);

    ImGui::PushTextWrapPos(800 - 20 - 40 - 20);
    ImGui::Text(gettext("File: %s\n"), connector->status.Filename.c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndChild();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);

    static ImGuiTableFlags flags =
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;

    ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 7);

    if (ImGui::BeginTable("##eqtable", 2, flags, outer_size)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("ClearAudio+");
        ImGui::TableNextColumn();
        ImGui::Text("%s (%s)",
            connector->soundSettings.s->status.clearAudioOn == 1 ? gettext("On") : gettext("Off"),
            connector->soundSettings.s->status.clearAudioAvailable == 1 ? gettext("available") : gettext("unavailable"));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Direct Source");
        ImGui::TableNextColumn();
        ImGui::Text("%s (%s)",
            connector->soundSettings.s->status.directSourceOn == 1 ? gettext("On") : gettext("Off"),
            connector->soundSettings.s->status.directSourceAvailable == 1 ? gettext("available") : gettext("unavailable"));

        if ((connector->soundSettings.s->status.clearAudioOn == 1 && connector->soundSettings.s->status.clearAudioAvailable) ||
            (connector->soundSettings.s->status.directSourceOn == 1 && connector->soundSettings.s->status.directSourceAvailable)) {
            ImGui::BeginDisabled(true);
        }
        //
        //            ImGui::TableNextRow();
        //            ImGui::TableNextColumn();
        //            ImGui::Text("EQ6");
        //            ImGui::TableNextColumn();
        //            ImGui::Text("%s", connector->soundSettings.s->eq6On == 1 ? "On" : "Off");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("EQ6 Preset"));
        ImGui::TableNextColumn();
        ImGui::Text("%s", eq6PresetToString.at(connector->soundSettings.s->status.eq6Preset).c_str());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("EQ6 Bands"));
        ImGui::TableNextColumn();

        ImGui::Text("%d, %d, %d, %d, %d, %d",
            connector->soundSettings.s->status.eq6Bands[0],
            connector->soundSettings.s->status.eq6Bands[1],
            connector->soundSettings.s->status.eq6Bands[2],
            connector->soundSettings.s->status.eq6Bands[3],
            connector->soundSettings.s->status.eq6Bands[4],
            connector->soundSettings.s->status.eq6Bands[5]);
        //
        //            ImGui::TableNextRow();
        //            ImGui::TableNextColumn();
        //            ImGui::Text("EQ10");
        //            ImGui::TableNextColumn();
        //            ImGui::Text("%s", connector->soundSettings.s->eq10On == 1 ? "On" : "Off");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("EQ10 Preset"));
        ImGui::TableNextColumn();
        ImGui::Text("%d", connector->soundSettings.s->status.eq10Preset);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("EQ10 Bands"));
        ImGui::TableNextColumn();
        ImGui::Text("%.1f, %.1f, %.1f, %.1f, %.1f,\n%.1f, %.1f, %.1f, %.1f, %.1f",
            float(connector->soundSettings.s->status.eq10Bands[0]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[1]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[2]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[3]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[4]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[5]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[6]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[7]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[8]) / 2,
            float(connector->soundSettings.s->status.eq10Bands[9]) / 2);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("DSEE HX");
        ImGui::TableNextColumn();
        ImGui::Text("%s", connector->soundSettings.s->status.dseeHXOn == 1 ? gettext("On") : gettext("Off"));

        //            ImGui::TableNextRow();
        //            ImGui::TableNextColumn();
        //            ImGui::Text("DSEE");
        //            ImGui::TableNextColumn();
        //            ImGui::Text("%s", connector->soundSettings.s->dseeOn == 1 ? "On" : "Off");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("DSEE custom"));
        ImGui::TableNextColumn();
        ImGui::Text("%s", connector->soundSettings.s->status.dseeCustOn == 1 ? gettext("On") : gettext("Off"));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("DSEE custom mode"));
        ImGui::TableNextColumn();
        ImGui::Text("%s", dseeModeToString.at(connector->soundSettings.s->status.dseeCustMode).c_str());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("DC Phase"));
        ImGui::TableNextColumn();
        ImGui::Text("%s", connector->soundSettings.s->status.dcLinearOn == 1 ? gettext("On") : gettext("Off"));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("DC Phase Filter"));
        ImGui::TableNextColumn();
        ImGui::Text("%s", dcFilterToString.at(connector->soundSettings.s->status.dcLinearFilter).c_str());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("VPT");
        ImGui::TableNextColumn();
        ImGui::Text("%s", connector->soundSettings.s->status.vptOn == 1 ? gettext("On") : gettext("Off"));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("VPT Mode"));
        ImGui::TableNextColumn();
        ImGui::Text("%s", vptA50SmallToString.at(connector->soundSettings.s->status.vptMode).c_str());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Tone Control or 10 band EQ?"));
        ImGui::TableNextColumn();
        ImGui::Text("%s", connector->soundSettings.s->status.eqUse == 2 ? gettext("10 band EQ") : gettext("Tone Control"));
        //
        //            ImGui::TableNextRow();
        //            ImGui::TableNextColumn();
        //            ImGui::Text("Tone Control");
        //            ImGui::TableNextColumn();
        //            ImGui::Text("%s", connector->soundSettings.s->toneControlOn == 1 ? "On" : "Off");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Tone Control Low"));
        ImGui::TableNextColumn();
        ImGui::Text("%d", connector->soundSettings.s->status.toneControlLow / 2);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Tone Control Mid"));
        ImGui::TableNextColumn();
        ImGui::Text("%d", connector->soundSettings.s->status.toneControlMid / 2);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Tone Control High"));
        ImGui::TableNextColumn();
        ImGui::Text("%d", connector->soundSettings.s->status.toneControlHigh / 2);
        //
        //            ImGui::TableNextRow();
        //            ImGui::TableNextColumn();
        //            ImGui::Text("Tone Control LowFreq");
        //            ImGui::TableNextColumn();
        //            ImGui::Text("%d", connector->soundSettings.s->toneControlLowFreq);
        //
        //            ImGui::TableNextRow();
        //            ImGui::TableNextColumn();
        //            ImGui::Text("Tone Control MidFreq");
        //            ImGui::TableNextColumn();
        //            ImGui::Text("%d", connector->soundSettings.s->toneControlMidFreq);
        //
        //            ImGui::TableNextRow();
        //            ImGui::TableNextColumn();
        //            ImGui::Text("Tone Control HighFreq");
        //            ImGui::TableNextColumn();
        //            ImGui::Text("%d", connector->soundSettings.s->toneControlHighFreq);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Vinyl Processor"));
        ImGui::TableNextColumn();
        ImGui::Text("%s", connector->soundSettings.s->status.vinylOn == 1 ? gettext("On") : gettext("Off"));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Vinyl Type"));
        ImGui::TableNextColumn();
        ImGui::Text("%s", vinylTypeToString.at(connector->soundSettings.s->status.vinylType).c_str());

        //            ImGui::TableNextRow();
        //            ImGui::TableNextColumn();
        //            ImGui::Text("Master Volume");
        //            ImGui::TableNextColumn();
        //            ImGui::Text("%d", connector->soundSettings.s->masterVolume);

        //            ImGui::TableNextRow();
        //            ImGui::TableNextColumn();
        //            ImGui::Text("Clear Phase");
        //            ImGui::TableNextColumn();
        //            ImGui::Text("%s", connector->soundSettings.s->clearPhaseOn == 1 ? "On" : "Off");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(gettext("Dynamic Normalizer"));
        ImGui::TableNextColumn();
        ImGui::Text("%s", connector->soundSettings.s->status.DNOn == 1 ? gettext("On") : gettext("Off"));

        if ((connector->soundSettings.s->status.clearAudioOn == 1 && connector->soundSettings.s->status.clearAudioAvailable) ||
            (connector->soundSettings.s->status.directSourceOn == 1 && connector->soundSettings.s->status.directSourceAvailable)) {
            ImGui::EndDisabled();
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar(2);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

    if (prevSong == connector->status.Filename) {
        ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_RED);
        if (eqSongExists) {
            if (ImGui::Button(gettext("Remove"), ImVec2(186, 60))) {
                if (SoundSettings::Remove(connector->status.Filename) == 0) {
                    eqStatus = gettext("Removed");
                } else {
                    eqStatus = gettext("Remove failed");
                }

                eqSongExists = SoundSettings::Exists(connector->status.Filename);
            }
        } else {
            ImGui::BeginDisabled();
            ImGui::Button(gettext("Remove"), ImVec2(186, 60));
            ImGui::EndDisabled();
        }

        ImGui::PopStyleColor();

        ImGui::SameLine();
    } else {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 186 + ImGui::GetStyle().ItemSpacing.x);
    }

    if (prevSong != connector->status.Filename) {
        connector->soundSettings.Update();
        connector->soundSettingsFw.Update();
        eqSongExists = SoundSettings::Exists(connector->status.Filename);
        eqSongDirExists = SoundSettings::ExistsDir(connector->status.Filename);
        prevSong = connector->status.Filename;
        eqStatus = gettext("Refreshed");
    }

    ImGui::SameLine();
    if (ImGui::Button(gettext("Set as default"), ImVec2(187, 60))) {
        if (connector->soundSettings.Save("default") == 0) {
            eqStatus = gettext("Default EQ set");
        } else {
            eqStatus = gettext("Set EQ as default failed");
        }
    }

    ImGui::SameLine();

    if (ImGui::Button(gettext("Save"), ImVec2(186, 60))) {
        if (connector->soundSettings.Save(connector->status.Filename) == 0) {
            eqStatus = gettext("Saved");
        } else {
            eqStatus = gettext("Save failed");
        }
        eqSongExists = SoundSettings::Exists(connector->status.Filename);
    }

    ImGui::SameLine();
    if (ImGui::Button(gettext("Save dir"), ImVec2(186, 60))) {
        if (connector->soundSettings.SaveDir(connector->status.Filename) == 0) {
            eqStatus = gettext("Saved dir");
        } else {
            eqStatus = gettext("Save dir failed");
        }
        eqSongDirExists = SoundSettings::ExistsDir(connector->status.Filename);
    }

    ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_RED);
    if (eqSongDirExists) {
        if (ImGui::Button(gettext("Remove dir"), ImVec2(186, 60))) {
            if (SoundSettings::RemoveDir(connector->status.Filename) == 0) {
                eqStatus = gettext("Dir removed");
            } else {
                eqStatus = gettext("Dir remove failed");
            }
            eqSongDirExists = SoundSettings::ExistsDir(connector->status.Filename);
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button(gettext("Remove dir"), ImVec2(186, 60));
        ImGui::EndDisabled();
    }

    ImGui::PopStyleColor();

    ImGui::SameLine();
    auto curpos = ImGui::GetCursorPos();
    ImGui::SetCursorPosY(curpos.y + 8);
    ImGui::Text(eqStatus.c_str());
    ImGui::SetCursorPosY(curpos.y - 8);

    ImGui::SameLine();
    ImGui::SetCursorPos(curpos);
    if (ImGui::InvisibleButton("##removeEqStatus", ImVec2(186 * 3 + ImGui::GetStyle().ItemSpacing.x + 3, 60))) {
        eqStatus = "";
    }
}
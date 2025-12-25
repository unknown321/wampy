#include "skin.h"

#include <libintl.h>
void Skin::Cassette() {
    if (!connector->storagePresent) {
        ImGui::Text(gettext("Disable USB mass storage mode"));
        return;
    }

    if (ImGui::Checkbox(gettext("Randomize?"), &config->cassette.randomize)) {
        config->Save();
    }

    static ImGuiTableFlags flags =
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;

    auto outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 7);

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    if (ImGui::BeginTable("configTable", 3, flags, outer_size)) {
        ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
        ImGui::TableSetupColumn(gettext("Codec"), ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn(gettext("Tape"), ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn(gettext("Reel"), ImGuiTableColumnFlags_None);
        ImGui::TableHeadersRow();

        for (auto &tt : config->cassette.data) {
            if (tt.second.name == Cassette::hiddenEntry) {
                continue;
            }
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", tt.second.name.c_str());

            ImGui::TableNextColumn();
            if (ImGui::BeginCombo(("##" + tt.second.name + "tape").c_str(), tt.second.tape.c_str(), ImGuiComboFlags_HeightSmall)) {
                for (auto &n : tapeListCassette) {
                    if (!n.valid) {
                        continue;
                    }

                    if (ImGui::Selectable(n.name.c_str(), false)) {
                        tt.second.tape = n.name;
                        config->cassette.data.at(tt.first).tape = n.name;
                        config->Save();

                        if (activeSkinVariant == CASSETTE) {
                            cassette.UnloadUnused();
                            cassette.LoadTape(n.name);
                            cassette.SelectTape();
                        }
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::TableNextColumn();
            if (ImGui::BeginCombo(("##" + tt.second.name + "reel").c_str(), tt.second.reel.c_str(), ImGuiComboFlags_HeightSmall)) {
                for (auto &n : reelListCassette) {
                    if (!n.valid) {
                        DLOG("invalid %s\n", n.name.c_str());
                        continue;
                    }

                    if (ImGui::Selectable(n.name.c_str(), false)) {
                        tt.second.reel = n.name;
                        config->cassette.data.at(tt.first).reel = n.name;
                        config->Save();

                        if (activeSkinVariant == CASSETTE) {
                            cassette.UnloadUnused();
                            if (cassette.LoadReelAtlas(n.name) != Tape::ERR_OK) {
                                cassette.LoadReel(n.name);
                            }
                            cassette.SelectTape();
                        }
                    }
                }
                ImGui::EndCombo();
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar(2);

    if (ImGui::Button(gettext("Reset"), ImVec2(186, 60))) {
        DLOG("resetting cassette config\n");
        cassette.config->Default();
        config->Save();
        if (activeSkinVariant == CASSETTE) {
            cassette.LoadImages();
            cassette.SelectTape();
            cassette.UnloadUnused();
        }
    }
}

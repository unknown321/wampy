#include "editor.h"
#include "../src/dac/dac.h"
#include "emscripten.h"
#include "imgui/imgui.h"
#include "implot.h"
#include <cmath>

void editor::MasterVolOpts() {
    ImGui::Checkbox("Sound effect", &soundEffectOn);

    const char *preview;
    preview = Dac::MasterVolumeTableTypeToString.at(MasterVolumeTableType).c_str();
    if (ImGui::BeginCombo("##masterVolumeTypeCombo", preview, ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : Dac::MasterVolumeTableTypeToString) {
            if (ImGui::Selectable(entry.second.c_str(), false)) {
                MasterVolumeTableType = entry.first;
                DLOG("selected type %s\n", entry.second.c_str());
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo(
            "##masterVolumeValueTypeCombo",
            Dac::MasterVolumeValueTypeToString.at(MasterVolumeValueType).c_str(),
            ImGuiComboFlags_HeightRegular
        )) {
        for (const auto &entry : Dac::MasterVolumeValueTypeToString) {
            if (ImGui::Selectable(entry.second.c_str(), false)) {
                MasterVolumeValueType = (MASTER_VOLUME_VALUE)entry.first;
                memcpy(
                    valuesyCopy,
                    masterVolumeValues[(int)soundEffectOn][MasterVolumeTableType][MasterVolumeValueType],
                    sizeof(masterVolumeValues[(int)soundEffectOn][MasterVolumeTableType][MasterVolumeValueType])
                );
                DLOG("selected value type %s (%d)\n", entry.second.c_str(), entry.first);
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Copy val", ImVec2(121, 60))) {
        memcpy(
            masterVolumeValueBuffer,
            masterVolumeValues[(int)soundEffectOn][MasterVolumeTableType][MasterVolumeValueType],
            sizeof(masterVolumeValueBuffer)
        );
    }

    ImGui::SameLine();
    if (ImGui::Button("Paste val", ImVec2(121, 60))) {
        memcpy(
            masterVolumeValues[(int)soundEffectOn][MasterVolumeTableType][MasterVolumeValueType],
            masterVolumeValueBuffer,
            sizeof(masterVolumeValueBuffer)
        );
    }
}

void editor::DSDOpts() {
    const char *preview;
    preview = Dac::MasterVolumeTableTypeToString.at(MasterVolumeDSDTableType).c_str();
    if (ImGui::BeginCombo("##masterVolumeDSDTypeCombo", preview, ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : Dac::MasterVolumeTableTypeToString) {
            if (ImGui::Selectable(entry.second.c_str(), false)) {
                MasterVolumeDSDTableType = entry.first;
                memcpy(
                    valuesyCopy, masterVolumeDSDValues[MasterVolumeDSDTableType], sizeof(masterVolumeDSDValues[MasterVolumeDSDTableType])
                );
                DLOG("selected dsd type %s\n", entry.second.c_str());
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Copy val", ImVec2(121, 60))) {
        memcpy(masterVolumeDSDValueBuffer, masterVolumeDSDValues[MasterVolumeDSDTableType], sizeof(masterVolumeDSDValueBuffer));
    }

    ImGui::SameLine();
    if (ImGui::Button("Paste val", ImVec2(121, 60))) {
        memcpy(masterVolumeDSDValues[MasterVolumeDSDTableType], masterVolumeDSDValueBuffer, sizeof(masterVolumeDSDValueBuffer));
    }
}

void editor::CurveEditor() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
    auto plotWidth = width * 0.7;
    auto buttWidth = width * 0.3;

    ImGui::BeginChild("##curvaBig", ImVec2(plotWidth, height), ImGuiChildFlags_None, window_flags);

    bool clicked = false;
    bool hovered = false;
    bool held[curveElementCount];

    for (int i = 0; i < curveElementCount; i++) {
        valuesX[i] = double(i);
    }

    switch (tableType) {
    case ETableType_UNKNOWN:
        target = &noValues;
        break;
    case ETableType_VOLUME:
        target = masterVolumeValues[soundEffectOn][MasterVolumeTableType][MasterVolumeValueType];
        break;
    case ETableType_DSD:
        target = masterVolumeDSDValues[MasterVolumeDSDTableType];
        break;
    case ETableType_TONE:
        target = toneControlValues[toneControlTableType];
        break;
    }

    if (ImPlot::BeginPlot("##lines", ImVec2(plotWidth, height), ImPlotFlags_NoMouseText | ImPlotFlags_NoBoxSelect)) {
        ImPlot::SetupLegend(ImPlotLocation_South, ImPlotLegendFlags_Outside);
        ImPlot::SetupAxesLimits(
            curveElementCount * -0.05,
            curveElementCount + curveElementCount * 0.05,
            curveYLimit * -0.05,
            curveYLimit + curveYLimit * 0.1,
            cond
        );

        float radius = 5;
        for (int i = 0; i < curveElementCount; i++) {
            ImPlot::DragPoint(
                i * 134,
                &valuesX[i],
                &target[i],
                ImVec4(0.82f, 0.64f, 0.03f, 1.00f),
                radius,
                ImPlotDragToolFlags_None,
                &clicked,
                &hovered,
                &held[i]
            );
            if (held[i]) {
                ImPlot::Annotation(double(i), target[i], ImVec4(0.90f, 0.90f, 0.90f, 1.00f), ImVec2(10, 10), false, true);
                target[i] = std::round(target[i]);
            }

            if (target[i] > curveYLimit) {
                target[i] = curveYLimit;
            }

            if (target[i] < 0) {
                target[i] = 0;
            }
        }

        ImPlot::SetNextLineStyle(GOLD_DONATE, 2.0f);
        ImPlot::PlotLine(legendName.c_str(), valuesX, target, curveElementCount);
        ImPlot::EndPlot();
    }
    if (cond != ImPlotCond_Once) {
        cond = ImPlotCond_Once;
    }

    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("##curvaBigButtons", ImVec2(buttWidth, 480), ImGuiChildFlags_None, window_flags);

    if (ImGui::Button("Open", ImVec2(230, 60))) {
        auto run = "var input = document.createElement('input'); "
                   "input.type = 'file'; "
                   "input.onchange = e => { "
                   "      open_file(e)"
                   "};"
                   "input.click();";
        emscripten_run_script(run);
    }

    if (tableType != ETableType_UNKNOWN) {
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(230, 60))) {
            switch (tableType) {
            case ETableType_UNKNOWN:
                break;
            case ETableType_VOLUME:
                MasterFromDouble();
                break;
            case ETableType_DSD:
                DSDFromDouble();
                break;
            case ETableType_TONE:
                ToneFromDouble();
                break;
            }

            char run[256];
            sprintf(run, "save_file(\"%s\");", name.c_str());
            emscripten_run_script(run);
        }
    }
    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();

    switch (tableType) {
    case ETableType_UNKNOWN:
        break;
    case ETableType_VOLUME:
        MasterVolOpts();
        break;
    case ETableType_DSD:
        DSDOpts();
        break;
    case ETableType_TONE:
        ToneOpts();
        break;
    }

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();

    ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_RED);
    if (ImGui::Button("Reset", ImVec2(230, 60))) {
        for (int i = 0; i < curveElementCount; i++) {
            target[i] = valuesyCopy[i];
        }
    }
    ImGui::PopStyleColor();

    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::NewLine();
    if (ImGui::TextLink("https://github.com/unknown321/wampy")) {
        auto run = ""
                   "const link = document.createElement('a');"
                   "link.href = \"https://github.com/unknown321/wampy\";"
                   "link.target=\"_blank\";"
                   "link.click();";
        emscripten_run_script(run);
    }
    if (ImGui::TextLink("https://boosty.to/unknown321/donate")) {
        auto run = ""
                   "const link = document.createElement('a');"
                   "link.href = \"https://boosty.to/unknown321/donate\";"
                   "link.target=\"_blank\";"
                   "link.click();";
        emscripten_run_script(run);
    }

    ImGui::EndChild();
}

void editor::ToneOpts() {
    const char *preview;
    preview = Dac::ToneControlTableTypeToString.at(toneControlTableType).c_str();
    if (ImGui::BeginCombo("##toneControlTypeCombo", preview, ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : Dac::ToneControlTableTypeToString) {
            if (ImGui::Selectable(entry.second.c_str(), false)) {
                toneControlTableType = entry.first;
                memcpy(valuesyCopy, toneControlValues[toneControlTableType], sizeof(valuesyCopy));
                DLOG("selected tone control type %s\n", entry.second.c_str());
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Copy val", ImVec2(121, 60))) {
        memcpy(toneControlValueBuffer, toneControlValues[toneControlTableType], sizeof(toneControlValueBuffer));
    }

    ImGui::SameLine();
    if (ImGui::Button("Paste val", ImVec2(121, 60))) {
        memcpy(toneControlValues[toneControlTableType], toneControlValueBuffer, sizeof(toneControlValueBuffer));
    }
}

void editor::DSDToDouble() {
    for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX; tableID++) {
        for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
            masterVolumeDSDValues[tableID][i] = double(masterVolumeDsd.v[tableID][i]);
        }
    }
}

void editor::DSDFromDouble() {
    for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX; tableID++) {
        for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
            for (int valType = 1; valType < MASTER_VOLUME_VALUE_MAX; valType++) {
                masterVolumeDsd.v[tableID][i] = (int)masterVolumeDSDValues[tableID][i];
            }
        }
    }
}

void editor::MasterToDouble() {
    for (int index = 0; index < 2; index++) {
        for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX; tableID++) {
            for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
                for (int valType = MASTER_VOLUME_VALUE_MIN; valType < MASTER_VOLUME_VALUE_MAX; valType++) {
                    auto val = masterVolume.GetValue(index, tableID, i, (MASTER_VOLUME_VALUE)valType);
                    masterVolumeValues[index][tableID][valType][i] = (double)val;
                }
            }
        }
    }
}

void editor::MasterFromDouble() {
    for (int index = 0; index < 2; index++) {
        for (int tableID = 0; tableID < MASTER_VOLUME_TABLE_MAX; tableID++) {
            for (int i = MASTER_VOLUME_MIN; i < MASTER_VOLUME_MAX + 1; i++) {
                for (int valType = 1; valType < MASTER_VOLUME_VALUE_MAX; valType++) {
                    auto val = masterVolumeValues[index][tableID][valType][i];
                    masterVolume.SetValue(index, tableID, i, (MASTER_VOLUME_VALUE)valType, (unsigned int)val);
                }
            }
        }
    }
}
void editor::ToneToDouble() {
    for (int tableID = 0; tableID < TONE_CONTROL_TABLE_MAX; tableID++) {
        for (int i = 0; i < CODEC_RAM_SIZE; i++) {
            auto val = toneControl.v[tableID][i];
            toneControlValues[tableID][i] = (double)val;
        }
    }
}

void editor::ToneFromDouble() {
    for (int tableID = 0; tableID < TONE_CONTROL_TABLE_MAX; tableID++) {
        for (int i = 0; i < CODEC_RAM_SIZE; i++) {
            auto val = toneControlValues[tableID][i];
            toneControl.v[tableID][i] = (int)val;
        }
    }
}

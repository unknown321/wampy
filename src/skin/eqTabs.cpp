#include "skin.h"
#include <libintl.h>

void Skin::TabEQ_DCPhase() {
    TabEq_EnableDisableFilter();
    ImGui::SeparatorText(gettext("Type:"));
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##DcPhaseValues", gettext(dcFilterToString.at(connector->soundSettingsFw.dcValue).c_str()), ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : dcFilterToString) {
            if (entry.second == "?") {
                continue;
            }
            if (ImGui::Selectable(gettext(entry.second.c_str()), false)) {
                connector->soundSettingsFw.SetDcFilterType(entry.first);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(2);
}

void Skin::TabEQ_Vpt() {
    TabEq_EnableDisableFilter();
    ImGui::SeparatorText(gettext("VPT type:"));
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##vptValues", gettext(vptA50SmallToString.at(connector->soundSettingsFw.vptValue).c_str()), ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : vptA50SmallToString) {
            if (entry.second == "?") {
                continue;
            }
            if (ImGui::Selectable(gettext(entry.second.c_str()), false)) {
                connector->soundSettingsFw.SetVptMode(entry.first);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(2);
}

void Skin::TabEQ_DynamicNormalizer() {
    TabEq_EnableDisableFilter();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - ImGui::CalcTextSize(gettext("No options")).x / 2);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 425 / 2 - ImGui::CalcTextSize(gettext("No options")).y / 2 - ImGui::GetTextLineHeightWithSpacing() - 60);
    ImGui::Text(gettext("No options"));
}

void Skin::TabEQ_Eq6Band() {
    TabEq_EnableDisableFilter();

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
    ImGui::SeparatorText(gettext("Preset:"));
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##eq6preset", gettext(eq6PresetToString.at(connector->soundSettingsFw.eq6Value).c_str()), ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : eq6PresetToString) {
            if (ImGui::Selectable(gettext(entry.second.c_str()), false)) {
                connector->soundSettingsFw.SetEq6Preset(entry.first);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(2);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(18, 14));
    static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;
    if (ImGui::BeginTable("##ssweq6", 6, flags)) {
        ImGui::TableNextRow(0, ImGui::TableGetHeaderRowHeight() + 4);
        for (int i = 0; i < 6; i++) {
            ImGui::TableNextColumn();
            ImGui::Text("%d", connector->soundSettingsFw.eq6bands.at(i).second);
        }
        ImGui::TableNextRow();
        for (int i = 0; i < 6; i++) {
            ImGui::TableNextColumn();
            ImGui::PushID(20 + i);
            if (ImGui::VSliderInt("##int", ImVec2(40, 196), &connector->soundSettingsFw.eq6bands.at(i).second, -10, 10, "")) {
                connector->soundSettingsFw.SetEq6Band(i, connector->soundSettingsFw.eq6bands.at(i).second);

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
            ImGui::PopID();
        }
        ImGui::TableNextRow();
        for (int i = 0; i < 6; i++) {
            ImGui::TableNextColumn();
            ImGui::Text(connector->soundSettingsFw.eq6bands.at(i).first.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::PopStyleVar(2);
}

void Skin::TabEQ_Eq10Band() {
    TabEq_EnableDisableFilter();

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(18, 14));

    static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;
    if (ImGui::BeginTable("##ssweq10", 10, flags)) {
        ImGui::TableNextRow(0, ImGui::TableGetHeaderRowHeight() + 4);
        for (int i = 0; i < 10; i++) {
            ImGui::TableNextColumn();
            ImGui::Text("%.1f", float(connector->soundSettingsFw.eq10bands.at(i).second) / 2);
        }
        ImGui::TableNextRow();
        for (int i = 0; i < 10; i++) {
            ImGui::TableNextColumn();
            ImGui::PushID(i);
            if (ImGui::VSliderInt("##int", ImVec2(40, 278), &connector->soundSettingsFw.eq10bands.at(i).second, -20, 20, "")) {
                connector->soundSettingsFw.SetEq10Band(i, connector->soundSettingsFw.eq10bands.at(i).second);

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
            ImGui::PopID();
        }
        ImGui::TableNextRow();
        for (int i = 0; i < 10; i++) {
            ImGui::TableNextColumn();
            ImGui::Text(connector->soundSettingsFw.eq10bands.at(i).first.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::PopStyleVar(2);
}

void Skin::TabEQ_EqTone() {
    TabEq_EnableDisableFilter();

    ImGui::SeparatorText(gettext("Tone:"));

    static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;
    if (ImGui::BeginTable("##sswEqtone", 3, flags)) {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 20.0);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 70.0);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 10.0);
        for (int i = 0; i < 3; i++) {
            ImGui::TableNextRow(0, 40.0f);
            ImGui::TableNextColumn();
            ImGui::Text(gettext(connector->soundSettingsFw.eqtone.at(i).first.c_str()));

            ImGui::TableNextColumn();
            ImGui::PushID(i);
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
            if (ImGui::SliderInt("##int", &connector->soundSettingsFw.eqtone.at(i).second, -20, 20, "")) {
                connector->soundSettingsFw.SetEqTone(i, connector->soundSettingsFw.eqtone.at(i).second);

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
            ImGui::PopStyleVar();
            ImGui::PopItemWidth();
            ImGui::PopID();
            ImGui::TableNextColumn();
            ImGui::Text("%.1f", float(connector->soundSettingsFw.eqtone.at(i).second) / 2);
        }

        ImGui::EndTable();
    }

    ImGui::SeparatorText(gettext("Tone frequency type:"));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

    if (ImGui::BeginTable("##sswEqtone2", 3, flags)) {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 20.0);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 70.0);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 10.0);

        for (int i = 0; i < 3; i++) {
            ImGui::TableNextRow(0, 40.0f);
            ImGui::TableNextColumn();
            ImGui::Text(gettext(connector->soundSettingsFw.eqtoneFreq.at(i).first.c_str()));

            ImGui::TableNextColumn();
            ImGui::PushID(10 + i);
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
            if (ImGui::SliderInt("##int", &connector->soundSettingsFw.eqtoneFreq.at(i).second, 0, 5, "")) {
                connector->soundSettingsFw.SetEqToneFreq(i, connector->soundSettingsFw.eqtoneFreq.at(i).second);

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
            ImGui::PopStyleVar();
            ImGui::PopItemWidth();
            ImGui::PopID();
            ImGui::TableNextColumn();
            ImGui::Text("%d", connector->soundSettingsFw.eqtoneFreq.at(i).second);
        }

        ImGui::EndTable();
    }
}

void Skin::TabEq_EnableDisableFilter() {
    if (connector->soundSettingsFw.filters.at(activeFilter)) {
        if (ImGui::Button(gettext("Disable"), ImVec2(186, 60))) {
            connector->soundSettingsFw.SetFilter(activeFilter, false);
            SetConfigFilter(activeFilter, false);
            nanosleep(&ssfwUpdateDelay, nullptr);
            connector->soundSettingsFw.Update();
            if (connector->status.State != PLAYING) {
                effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
            } else {
                effectQueueStatus.clear();
            }
        }
    } else {
        if (ImGui::Button("Enable", ImVec2(186, 60))) {
            connector->soundSettingsFw.SetFilter(activeFilter, true);
            SetConfigFilter(activeFilter, true);
            nanosleep(&ssfwUpdateDelay, nullptr);
            connector->soundSettingsFw.Update();
            if (connector->status.State != PLAYING) {
                effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
            } else {
                effectQueueStatus.clear();
            }
        }
    }

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 9);
    ImGui::Text(effectQueueStatus.c_str());
}

void Skin::TabEQ_Vinylizer() {
    TabEq_EnableDisableFilter();

    ImGui::SeparatorText(gettext("Vinylizer type:"));
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(
            "##vinylizerValues", gettext(vinylTypeToString.at(connector->soundSettingsFw.vinylizerValue).c_str()), ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : vinylTypeToString) {
            if (entry.second == "?") {
                continue;
            }
            if (ImGui::Selectable(gettext(entry.second.c_str()), false)) {
                connector->soundSettingsFw.SetVinylizerType(entry.first);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();
                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(2);
}

void Skin::TabEQ_Donate() const {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 425 / 2 - qrSide / 2 - 28);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - qrSide / 2);

    ImGui::Image((void *)(intptr_t)qrDonateTexture, ImVec2(qrSide, qrSide));

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 580 / 2 - ImGui::CalcTextSize("boosty.to/unknown321/donate").x / 2);
    ImGui::TextColored(GOLD_DONATE, "boosty.to/unknown321/donate");
}

void Skin::TabEQ_Misc() {
    static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;
    if (ImGui::BeginTable("##sswMisc", 2, flags)) {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 65.0);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 35.0);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Direct Source");

        ImGui::TableNextColumn();
        if (connector->soundSettingsFw.s->directSourceOn) {
            if (ImGui::Button(gettext("Disable"), ImVec2(186, 60))) {
                connector->soundSettingsFw.SetDirectSource(false);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
        } else {
            if (ImGui::Button(gettext("Enable"), ImVec2(186, 60))) {
                connector->soundSettingsFw.SetDirectSource(true);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("ClearAudio+");

        ImGui::TableNextColumn();
        if (isWalkmanOne || !connector->soundSettings.s->status.clearAudioAvailable) {
            ImGui::BeginDisabled();
        }
        if (connector->soundSettingsFw.s->clearAudioPlusOn) {
            ImGui::PushID(30 + 1);
            if (ImGui::Button(gettext("Disable"), ImVec2(186, 60))) {
                connector->soundSettingsFw.SetClearAudioPlus(false);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
            ImGui::PopID();
        } else {
            ImGui::PushID(30 + 1);
            if (ImGui::Button(gettext("Enable"), ImVec2(186, 60))) {
                connector->soundSettingsFw.SetClearAudioPlus(true);
                nanosleep(&ssfwUpdateDelay, nullptr);
                connector->soundSettingsFw.Update();

                if (connector->status.State != PLAYING) {
                    effectQueueStatus = EFFECT_CHANGE_QUEUE_TEXT;
                } else {
                    effectQueueStatus.clear();
                }
            }
            ImGui::PopID();
        }

        if (isWalkmanOne || !connector->soundSettings.s->status.clearAudioAvailable) {
            ImGui::EndDisabled();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::Text(effectQueueStatus.c_str());

        ImGui::EndTable();
    }
}
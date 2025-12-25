#include "skin.h"

#include <libintl.h>

void Skin::DigitalClock() {
    if (!connector->storagePresent) {
        ImGui::Text(gettext("Disable USB mass storage mode"));
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    if (ImGui::BeginCombo("##digiClockColor", DigitalClock::DigitalClock::GetColorPreview(config->digitalClock.color).c_str(), ImGuiComboFlags_HeightRegular)) {
        for (const auto &entry : DigitalClock::colorsDigitalClock) {
            if (ImGui::Selectable(entry.first.c_str(), false)) {
                DLOG("selected color %s\n", entry.second.c_str());
                digitalClock.SetColor(entry.second);
                if (activeSkinVariant == DIGITAL_CLOCK) {
                    needLoad = true;
                } else {
                    config->digitalClock.color = entry.second;
                    config->Save();
                }
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(2);
}

#include "skin.h"

#include <libintl.h>

void Skin::TabDac() {
    if (connector->status.State == PlayStateE::PLAYING) {
        ImGui::NewLine();
        ImGui::Text(gettext("Stop music first."));
        ImGui::NewLine();
        if (ImGui::Button(gettext("Stop music"), ImVec2(186, 60))) {
            connector->Pause();
        }
        return;
    }

    ImGui::Text(llusbdacStatus.c_str());
    auto label = llusbdacLoaded ? gettext("Disable") : gettext("Enable");
    if (ImGui::Button(label, ImVec2(756, 350))) {
        if (llusbdacLoaded) {
            llusbdacLoaded = !DisableLLUSBDAC();
            llusbdacStatus = !llusbdacLoaded ? gettext("Unloaded") : gettext("Failure");
        } else {
            llusbdacLoaded = EnableLLUSBDAC();
            llusbdacStatus = llusbdacLoaded ? gettext("Loaded") : gettext("Failure");
        }
    }
}
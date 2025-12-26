#include "skin.h"

void Skin::TabDebug() {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 15.0f));
    ImGui::SeparatorText(gettext("Debug"));

    if (ImGui::Checkbox(gettext("Skin debug"), &config->debug)) {
        winamp.debug = config->debug;
        cassette.debug = config->debug;
        config->Save();
    }
    ImGui::PopStyleVar();

    ImGui::NewLine();

    if (ImGui::Button(gettext("Start ADB daemon (next boot)"))) {
        startADB();
    }

    ImGui::NewLine();

    if (ImGui::Button(gettext("Create log file"))) {
        createDump();
    }

    ImGui::NewLine();
    if (ImGui::Checkbox(gettext("Limit fps"), &config->limitFPS)) {
        config->Save();
    }

    ImGui::NewLine();

    printFPS();
}
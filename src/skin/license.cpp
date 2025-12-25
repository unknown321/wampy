#include "skin.h"

void Skin::ReadLicense() {
    std::ifstream f;
    f.open(licensePath);
    const std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    license = contents;
    f.close();

    f.open(license3rdPath);
    const std::string contents2((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    license3rd = contents2;
    f.close();
}

void Skin::License() const {
    ImGui::SetCursorPosY(60);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::BeginChild("licenseText", ImVec2(740, 400));
    ImGui::PushTextWrapPos(740 + 18 - 40 - 6);
    ImGui::Text("%s", license.c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void Skin::License3rd() const {
    ImGui::SetCursorPosY(60);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::BeginChild("license3Text", ImVec2(740, 400));
    ImGui::PushTextWrapPos(740 + 18 - 40 - 6);
    ImGui::Text("%s", license3rd.c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}
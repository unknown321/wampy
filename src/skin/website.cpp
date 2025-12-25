#include "skin.h"

#include <libintl.h>

void Skin::Website() const {
    ImGui::NewLine();
    float columnWidth = 380.0f;
    if (ImGui::BeginTable("##website", 2, ImGuiTableFlags_None)) {
        ImGui::TableSetupColumn("GitHub", ImGuiTableColumnFlags_WidthFixed, 380);
        ImGui::TableSetupColumn(gettext("Donate"), ImGuiTableColumnFlags_WidthFixed, 380);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - ImGui::CalcTextSize("Github").x / 2);
        ImGui::Text("GitHub");
        ImGui::TableNextColumn();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - ImGui::CalcTextSize(gettext("Donate")).x / 2);
        ImGui::TextColored(GOLD_DONATE, gettext("Donate"));

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - qrSide / 2);
        ImGui::Image((void *)(intptr_t)qrTexture, ImVec2(qrSide, qrSide));

        ImGui::TableNextColumn();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - qrSide / 2);
        ImGui::Image((void *)(intptr_t)qrDonateTexture, ImVec2(qrSide, qrSide));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - ImGui::CalcTextSize("github.com/unknown321/wampy").x / 2);
        ImGui::Text("github.com/unknown321/wampy");

        ImGui::TableNextColumn();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth / 2 - ImGui::CalcTextSize("boosty.to/unknown321/donate").x / 2);
        ImGui::TextColored(GOLD_DONATE, "boosty.to/unknown321/donate");

        ImGui::EndTable();
    }
}
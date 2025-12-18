#include "hagoromoStatus.h"
#include "rec/rec.h"
#include "skin.h"

void Skin::ReadFMButtons() {
    const std::map<std::string *, GLuint *> m = {
        {&fmDelete, &fmDeleteTexture},
        {&fmLeft, &fmLeftTexture},
        {&fmLeftDouble, &fmLeftDoubleTexture},
        {&fmRight, &fmRightTexture},
        {&fmRightDouble, &fmRightDoubleTexture},
        {&fmSave, &fmSaveTexture},
        {&fmDelete, &fmDeleteTexture},
        {&fmRecord, &fmRecordTexture},
        {&fmSettings, &fmSettingsTexture},
    };

    for (auto k : m) {
        Magick::Image img;
        img.read(*k.first);
        img.magick("RGBA");
        img.depth(8);
        Magick::Blob blob;
        img.write(&blob);
        LoadTextureFromMagic((unsigned char *)blob.data(), k.second, static_cast<int>(img.size().width()), static_cast<int>(img.size().height()));
    }
}

void Skin::TabFM() {
    constexpr float buttonSize = 70.0f;

    ImGui::NewLine();
    if (!radioAvailable) {
        ImGui::Text("FM radio is unavailable on this device");
        ImGui::NewLine();
        if (ImGui::Button("Hide this tab", ImVec2(200, 60))) {
            config->showFmInSettings = false;
            displayTab = SettingsTab::SkinOpts;
            config->Save();
        }
        return;
    }

    if (connector->soundSettings.s->fmStatus.state == 2) {
        if (ImGui::Button("Disable", ImVec2(186, 60))) {
            RadioOff();
            connector->soundSettings.SetFM(0);
        }
    } else {
        if (ImGui::Button("Enable", ImVec2(186, 60))) {
            if (connector->status.State == PlayStateE::PLAYING) {
                connector->Pause();
            }
            RadioOn();
            connector->soundSettings.SetFM(1);
        }
    }

    if (connector->soundSettings.s->fmStatus.state != 2) {
        return;
    }

    ImGui::SameLine();
    if (fmRecordingActive) {
        const auto rsize = ImGui::CalcTextSize("Recording").x + ImGui::GetStyle().FramePadding.x * 2.f;
        ImGui::SetCursorPosX(windowSize.x / 2 - rsize / 2);
        ImGui::TextColored(TEXT_RECORDING, "Recording");
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(windowSize.x - ImGui::CalcTextSize("Stereo").x - 50);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
    if (ImGui::Checkbox("Stereo", &connector->soundSettings.s->fmStatus.stereo)) {
        connector->soundSettings.SetFMStereo(connector->soundSettings.s->fmStatus.stereo);
    }

    ImGui::PushItemWidth(windowSize.x - indent * 2 - ImGui::GetStyle().FramePadding.x * 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(40.0f, 40.0f));
    if (ImGui::SliderInt("##fmfreq", &fmFreq, 76000, 108000, fmFreqFormat, ImGuiSliderFlags_NoInput)) {
        if (fmFreq % 100 > 0) {
            fmFreq = fmFreq - fmFreq % 100;
        }
        connector->soundSettings.SetFMFreq(fmFreq);
        sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
    }
    ImGui::PopStyleVar(2);
    ImGui::PopItemWidth();

    if (ImGui::ImageButton("<<", reinterpret_cast<ImTextureID>(fmLeftDoubleTexture), ImVec2(buttonSize, buttonSize))) {
        fmFreq -= 500;
        if (fmFreq < FM_FREQ_MIN) {
            fmFreq = FM_FREQ_MIN;
        }
        connector->soundSettings.SetFMFreq(fmFreq);
        sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
    }

    ImGui::SameLine();
    if (ImGui::ImageButton("<", reinterpret_cast<ImTextureID>(fmLeftTexture), ImVec2(buttonSize, buttonSize))) {
        fmFreq -= 100;
        if (fmFreq < FM_FREQ_MIN) {
            fmFreq = FM_FREQ_MIN;
        }
        connector->soundSettings.SetFMFreq(fmFreq);
        sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX((windowSize.x / 2) - buttonSize * 2 - ImGui::GetStyle().ItemSpacing.x * 3);
    if (ImGui::ImageButton("Save", reinterpret_cast<ImTextureID>(fmSaveTexture), ImVec2(buttonSize, buttonSize))) {
        if (std::find(config->fmPresets.begin(), config->fmPresets.end(), fmFreq) == config->fmPresets.end()) {
            config->fmPresets.emplace_back(fmFreq);
            std::sort(config->fmPresets.begin(), config->fmPresets.end());
            config->Save();
        }
    }

    ImGui::SameLine();
    if (ImGui::ImageButton("Delete", reinterpret_cast<ImTextureID>(fmDeleteTexture), ImVec2(buttonSize, buttonSize))) {
        auto pos = std::find(config->fmPresets.begin(), config->fmPresets.end(), fmFreq);
        if (pos != config->fmPresets.end()) {
            config->fmPresets.erase(pos);
            std::sort(config->fmPresets.begin(), config->fmPresets.end());
            config->Save();
        }
    }

    ImGui::SameLine();
    if (ImGui::ImageButton("Record", reinterpret_cast<ImTextureID>(fmRecordTexture), ImVec2(buttonSize, buttonSize))) {
        fmRecordingActive = !fmRecordingActive;
        if (fmRecordingActive) {
            fmRecordingStop = false;
            DLOG("starting recording, codec %d, storage %d\n", fm_codec, fm_storage);
            start_rec(fm_codec, fm_storage, &fmRecordingStop);
        } else {
            fmRecordingStop = true;
        }
    }

    ImGui::SameLine();
    if (ImGui::ImageButton("Settings", reinterpret_cast<ImTextureID>(fmSettingsTexture), ImVec2(buttonSize, buttonSize))) {
        calcFMStorageLabels();
        ImGui::OpenPopup("Recording settings");
    }

    ImGui::SetNextWindowPos(ImVec2(windowSize.x / 2, windowSize.y / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Recording settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::NewLine();

        ImGui::Text("Storage: ");
        ImGui::SameLine();
        if (ImGui::RadioButton(fm_storage_internal_free_label.c_str(), reinterpret_cast<int *>(&fm_storage), static_cast<int>(RecordStorage::INTERNAL))) {
            config->fmRecording.Storage = "internal";
            config->Save();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(fm_storage_sd_card_free_label.c_str(), reinterpret_cast<int *>(&fm_storage), static_cast<int>(RecordStorage::SD_CARD))) {
            config->fmRecording.Storage = "sdcard";
            config->Save();
        }

        ImGui::NewLine();

        ImGui::Text("Codec: ");
        ImGui::SameLine();
        if (ImGui::RadioButton("MP3 320kbps", reinterpret_cast<int *>(&fm_codec), static_cast<int>(RecordCodec::MP3))) {
            calcFMStorageLabels();
            config->fmRecording.Codec = "mp3";
            config->Save();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("WAV", reinterpret_cast<int *>(&fm_codec), static_cast<int>(RecordCodec::WAV))) {
            calcFMStorageLabels();
            config->fmRecording.Codec = "wav";
            config->Save();
        }

        ImGui::NewLine();

        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 120);
        if (ImGui::Button("OK", ImVec2(120, buttonSize))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::SameLine();

    ImGui::SetCursorPosX(windowSize.x - buttonSize * 2 - ImGui::GetStyle().ItemSpacing.x - indent * 2 - 1);
    if (ImGui::ImageButton(">", reinterpret_cast<ImTextureID>(fmRightTexture), ImVec2(buttonSize, buttonSize))) {
        fmFreq += 100;
        if (fmFreq > FM_FREQ_MAX) {
            fmFreq = FM_FREQ_MAX;
        }
        connector->soundSettings.SetFMFreq(fmFreq);
        sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
    }

    ImGui::SameLine();
    if (ImGui::ImageButton(">>", reinterpret_cast<ImTextureID>(fmRightDoubleTexture), ImVec2(buttonSize, buttonSize))) {
        fmFreq += 500;
        if (fmFreq > FM_FREQ_MAX) {
            fmFreq = FM_FREQ_MAX;
        }
        connector->soundSettings.SetFMFreq(fmFreq);
        sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
    }

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
    ImGui::Separator();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 40.0f);
    ImGui::BeginChild("##fmpresets", ImVec2(windowSize.x - indent * 2 - ImGui::GetStyle().FramePadding.x * 2.0f, 145), ImGuiChildFlags_None, window_flags);
    ImVec2 button_sz(88, 60);
    float window_visible_x2 = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
    for (int i = 0; i < config->fmPresets.size(); i++) {
        float last_button_x2 = ImGui::GetItemRectMax().x;
        float next_button_x2 = last_button_x2 + ImGui::GetStyle().ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
        if (i < config->fmPresets.size() && next_button_x2 < window_visible_x2)
            ImGui::SameLine();
        char label[20];
        sprintf(label, "%.1f", (float)(config->fmPresets.at(i)) / 1000);
        if (ImGui::Button(label, button_sz)) {
            fmFreq = config->fmPresets.at(i);
            connector->soundSettings.SetFMFreq(config->fmPresets.at(i));
            sprintf(fmFreqFormat, "%.1fMHz", float(fmFreq) / 1000);
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void Skin::calcFMStorageLabels() {
#ifdef DESKTOP
    fm_storage_internal_free = GetFreeSpace("/");
    fm_storage_sd_card_free = GetFreeSpace("/dev/shm");
#else
    fm_storage_internal_free = GetFreeSpace("/contents");
    fm_storage_sd_card_free = GetFreeSpace("/contents_ext");
#endif

    fm_storage_internal_free_label = "Internal (" + std::to_string(fm_storage_internal_free / 1024 / 1024) + " MB)\n";
    fm_storage_sd_card_free_label = "SD Card (" + std::to_string(fm_storage_sd_card_free / 1024 / 1024) + " MB)\n";

    unsigned int one_second_bytes = 0;
    if (fm_codec == RecordCodec::WAV) {
        one_second_bytes = 44100 * 16 / 8 * 2;
    } else {
        one_second_bytes = 320 / 8 * 1024;
    }
    const auto minutes_internal = fm_storage_internal_free / (one_second_bytes * 60);
    if (minutes_internal > 60) {
        auto hours = 0;
        hours = std::floor(minutes_internal / 60);
        if (hours == 1) {
            fm_storage_internal_free_label += "~" + std::to_string(hours) + " hour";
        } else {
            fm_storage_internal_free_label += "~" + std::to_string(hours) + " hours";
        }
    } else {
        if (minutes_internal == 1) {
            fm_storage_internal_free_label += "~" + std::to_string(minutes_internal) + " minute";
        } else {
            fm_storage_internal_free_label += "~" + std::to_string(minutes_internal) + " minutes";
        }
    }

    const auto minutes_sd_card = fm_storage_sd_card_free / (one_second_bytes * 60);
    if (minutes_sd_card > 60) {
        auto hours = 0;
        hours = std::floor(minutes_sd_card / 60);
        if (hours == 1) {
            fm_storage_sd_card_free_label += "~" + std::to_string(hours) + " hour";
        } else {
            fm_storage_sd_card_free_label += "~" + std::to_string(hours) + " hours";
        }
    } else {
        if (minutes_sd_card == 1) {
            fm_storage_sd_card_free_label += "~" + std::to_string(minutes_sd_card) + " minute";
        } else {
            fm_storage_sd_card_free_label += "~" + std::to_string(minutes_sd_card) + " minutes";
        }
    }
}
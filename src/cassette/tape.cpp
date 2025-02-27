#include "tape.h"
#include "../skin.h"
#include <fstream>
#include <map>
#include <sstream>

namespace Tape {
    int Tape::LoadFromJPEGS(const std::string &directoryPath) {
        SkinList files{};
        listdir((directoryPath + "/").c_str(), &files, ".jpg");

        if (files.empty()) {
            DLOG("no images in %s, skipping\n", directoryPath.c_str());
            return ERR_NO_FILES;
        }

        std::ifstream f;
        f.open(files[0].fullPath);
        std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        Elements.Main.WithMagick("JPEG")->FromData((char *)contents.c_str(), contents.size())->WithRatio(1.0f)->Load();
        f.close();

        return 0;
    }

    int Tape::Load(const std::string &directoryPath) {
        DLOG("loading tape from %s\n", directoryPath.c_str());

        if (exists(directoryPath + "/tape.pkm")) {
            DLOG("loading ETC from %s\n", (directoryPath + "/tape.pkm").c_str());
            if (Elements.Main.FromPKM(directoryPath + "/tape.pkm") != 0) {
                DLOG("failed to load tape from pkm, back to jpegs\n");
                if (LoadFromJPEGS(directoryPath) != 0) {
                    return ERR_NO_FILES;
                }
            }
        } else {
            DLOG("loading tape from regular files from %s\n", directoryPath.c_str());
            if (LoadFromJPEGS(directoryPath) != 0) {
                return ERR_NO_FILES;
            }
        }

        std::ifstream f;
        f.open(directoryPath + "/config.txt");
        if (!f.is_open()) {
            DLOG("using default config for %s\n", directoryPath.c_str());
            return 0;
        }

        std::stringstream buf;
        buf << f.rdbuf();
        f.close();

        std::string contents = buf.str();
        ParseConfig(contents);

        this->InitializeButtons();

        return 0;
    }

    void Tape::ParseConfig(const std::string &text) {
        for (const auto &line : split(text, "\n")) {
            auto parts = split(line, ": ");
            switch (hash(parts[0].c_str())) {
            case hash("reel"): {
                this->reel = parts[1];
                break;
            }
            case hash("reelx"): {
                this->reelCoords.x = std::stof(parts[1]);
                break;
            }
            case hash("reely"): {
                this->reelCoords.y = std::stof(parts[1]);
                break;
            }
            case hash("artistx"): {
                this->artistCoords.x = std::stof(parts[1]);
                break;
            }
            case hash("artisty"): {
                this->artistCoords.y = std::stof(parts[1]);
                break;
            }
            case hash("artistformat"): {
                this->formatArtist = parts[1];
                break;
            }
            case hash("titlex"): {
                this->titleCoords.x = std::stof(parts[1]);
                break;
            }
            case hash("titley"): {
                this->titleCoords.y = std::stof(parts[1]);
                break;
            }
            case hash("titleformat"): {
                this->formatTitle = parts[1];
                break;
            }
            case hash("albumx"): {
                this->albumCoords.x = std::stof(parts[1]);
                break;
            }
            case hash("albumy"): {
                this->albumCoords.y = std::stof(parts[1]);
                break;
            }
            case hash("albumformat"): {
                this->formatAlbum = parts[1];
                break;
            }
            case hash("titlewidth"): {
                this->titleWidth = std::stof(parts[1]);
                break;
            }
            case hash("durationformat"): {
                this->formatDuration = parts[1];
                break;
            }
            case hash("textcolor"): {
                this->textColor = colorToImCol32(parts[1]);
                break;
            }
            default:
                break;
            }
        }

        if (this->textColor == 0) {
            this->textColor = IM_COL32_BLACK;
        }
    }

    int Tape::InitializeButtons() {
        FlatTexture flatTexture;
        ImGui::ButtonTexture bt;
        ImGui::ButtonTextures bts;

        bt.active = flatTexture.FromColor({80, 80}, {255.0f, 0.0f, 0.0f, 0.0f});
        bt.size = flatTexture.GetSize();
        bt.pressed = bt.active;
        bts[0] = bt;
        Elements.ToggleSettings.WithID("toggleSettings")
            ->WithPosition(800 - bt.size.x, 0.0f)
            ->WithTextures(bts)
            ->WithCallback(Skin::Skin::ToggleDrawSettings, skin, nullptr);
        Elements.RandomizeTape.WithID("randomizeTape")
            ->WithPosition(400 - bt.size.x / 2, 240 - bt.size.y / 2)
            ->WithTextures(bts)
            ->WithCallback(Skin::Skin::RandomizeTape, skin, nullptr);

        return 0;
    }

    void Tape::Draw() {
        Elements.Main.Draw();
        Elements.ToggleSettings.Draw();
        Elements.RandomizeTape.Draw();
    }

    void Tape::DrawSongInfo() const {
        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        if (titleCoords.x > 0) {
            ImGui::SetCursorPos(titleCoords);
            ImGui::Text("%s", title);
        }

        if (artistCoords.x > 0) {
            ImGui::SetCursorPos(artistCoords);
            ImGui::Text("%s", artist);
        }

        if (albumCoords.x > 0) {
            ImGui::SetCursorPos(albumCoords);
            ImGui::Text("%s", album);
        }

        ImGui::PopStyleColor();
    }

    void Tape::Unload() {
        DLOG("unload tape %s\n", name.c_str());
        Elements.Main.Unload();
        DLOG("unload button\n");
        Elements.ToggleSettings.Unload();
        Elements.RandomizeTape.Unload();
        valid = false;
    }
} // namespace Tape
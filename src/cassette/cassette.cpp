#include "cassette.h"
#include "imgui_impl_opengl3.h"
#include <fstream>
#include <thread>

static const ImWchar rangesPunctuation[] = {
    0x2000, 0x206F, // General Punctuation
};

namespace Cassette {

#ifdef DESKTOP
    //    static const std::string FontPath = "../SSTJpPro-Regular.otf";
    static const std::string FontPath = "../NotoSansKR-Regular.otf";
#else
    static const std::string FontPath = "/system/vendor/sony/lib/fonts/NotoSansKR-Regular.otf";
#endif

    static const int ttfFontSize = 34;
    static const int reelDelayMs = 55;

    std::vector<Tape::TapeType> tapeTypes = {
        Tape::MP3_128,
        Tape::MP3_160,
        Tape::MP3_256,
        Tape::MP3_320,
        Tape::FLAC_ALAC_APE_MQA,
        Tape::AIFF,
        Tape::PCM,
        Tape::FLAC_MQA_ALAC_PCM_AIFF_APE_HIRES,
        Tape::DSD};

    void Config::Default() {
        auto d = GetDefault();
        for (const auto &kv : d) {
            data[kv.first] = kv.second;
        }
    }

    std::map<Tape::TapeType, ConfigEntry> Config::GetDefault() {
        std::map<Tape::TapeType, ConfigEntry> ret;
        ret[Tape::MP3_128] = {"chf", "chf", "MP3 128kbps"};
        ret[Tape::MP3_160] = {"bhf", "other", "MP3 160kbps"};
        ret[Tape::MP3_256] = {"ahf", "other", "MP3 256kbps"};
        ret[Tape::MP3_320] = {"jhf", "other", "MP3 320kbps"};
        ret[Tape::FLAC_ALAC_APE_MQA] = {"ucx", "other", "FLAC"};
        ret[Tape::AIFF] = {"ucx_s", "other", "AIFF"};
        ret[Tape::PCM] = {"duad", "other", "PCM"};
        ret[Tape::FLAC_MQA_ALAC_PCM_AIFF_APE_HIRES] = {"metal", "other", "Hi-Res"};
        ret[Tape::DSD] = {"metal_master", "metal_master", "DSD"};
        return ret;
    }

    int Cassette::AddFonts(ImFont *fontRegular) {
        auto io = ImGui::GetIO();
        io.Fonts->Clear();

        static ImFontGlyphRangesBuilder range;
        range.Clear();
        static ImVector<ImWchar> gr;
        gr.clear();

        range.AddRanges(io.Fonts->GetGlyphRangesDefault());
        range.AddRanges(&rangesPunctuation[0]);

        if (fontRanges) {
            if (fontRanges->Japanese)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
            if (fontRanges->ChineseFull)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
            if (fontRanges->Cyrillic)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
            if (fontRanges->Greek)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesGreek());
            if (fontRanges->Korean)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesKorean());
            if (fontRanges->Thai)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesThai());
            if (fontRanges->Vietnamese)
                range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesVietnamese());
        }

        range.BuildRanges(&gr);
        fontRegular = io.Fonts->AddFontFromFileTTF(FontPath.c_str(), ttfFontSize, nullptr, gr.Data);

        ImGui_ImplOpenGL3_DestroyFontsTexture();
        ImGui_ImplOpenGL3_CreateFontsTexture();

        fontRegular->FontSize = 25; // this will scale down elements in settings

        return 0;
    }

    auto comp = [](const directoryEntry &a, const directoryEntry &b) { return a.name < b.name; };

    void Cassette::Notify() {}

    int Cassette::LoadReel(const std::string &path) {
        auto reelFiles = std::vector<directoryEntry>{};
        if (!Reels[path].empty()) {
            DLOG("reel %s already loaded, %zu textures\n", path.c_str(), Reels[path].size());
            return Tape::ERR_OK;
        }

        for (auto &entry : *reelList) {
            if (entry.name != path) {
                continue;
            }

            listdir((entry.fullPath + "/").c_str(), &reelFiles, ".jpg");
            if (reelFiles.empty()) {
                entry.valid = false;
                break;
            }

            std::sort(reelFiles.begin(), reelFiles.end(), comp);

            break;
        }

        if (reelFiles.empty()) {
            DLOG("reel %s specified in config but not found\n", path.c_str());
            return Tape::ERR_NO_FILES;
        }

        std::ifstream f;
        DLOG("loading reel %s\n", path.c_str());
        for (const auto &kv : reelFiles) {
            f.open(kv.fullPath);
            std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

            auto ft = FlatTexture();

            ft.WithMagick("JPEG")->FromData((char *)contents.c_str(), contents.size())->WithRatio(1.0f)->Load();
            Reels[path].emplace_back(ft);
            f.close();
        }

        DLOG("reel images: %zu\n", Reels[path].size());

        return Tape::ERR_OK;
    }

    int Cassette::LoadTape(const std::string &path) {
        DLOG("loading tape %s\n", path.c_str());
        assert(!path.empty());

        for (const auto &e : *tapeList) {
            if (e.name != path) {
                continue;
            }

            if (Tapes[path].valid && Tapes[path].Elements.Main.textureID != 0) {
                DLOG("tape %s already loaded\n", path.c_str());
                break;
            }

            Tapes[path].skin = skin;
            Tapes[path].song = &song;
            Tapes[path].connector = connector;

            if (Tapes[path].Load(e.fullPath) == Tape::ERR_NO_FILES) {
                DLOG("failed to load %s\n", e.fullPath.c_str());
                Tapes[path].valid = false;
                return Tape::ERR_NO_FILES;
            }

            Tapes[path].name = e.name;
            Tapes[path].valid = true;

            break;
        }

        return Tape::ERR_OK;
    }

    void Cassette::Unload() {
        for (auto &v : Tapes) {
            v.second.Unload();
        }

        Tapes.clear();

        for (auto &v : Reels) {
            DLOG("unload reel %s\n", v.first.c_str());
            for (auto &tex : v.second) {
                tex.Unload();
            }
        }

        Reels.clear();
        Track = "";

        reelThreadStop = true;
    }

    void Cassette::UnloadUnused() {
        for (auto &t : Tapes) {
            bool used = false;
            for (const auto &v : config->data) {
                if (v.second.tape == t.first) {
                    used = true;
                    break;
                }
            }

            if (used) {
                continue;
            } else {
                DLOG("tape %s is unused, unloading\n", t.first.c_str());
                t.second.Unload();
            }
        }

        for (auto &r : Reels) {
            bool used = false;
            for (const auto &v : config->data) {
                if (v.second.reel == r.first) {
                    used = true;
                    break;
                }
            }

            if (used) {
                continue;
            } else {
                DLOG("reel %s is unused, unloading\n", r.first.c_str());
                for (auto &tex : r.second) {
                    tex.Unload();
                }
                r.second.clear();
            }
        }
    }

    void Cassette::WithConfig(Config *c) { this->config = c; }

    // replaces invalid config entries with first valid tape/reel occurrences
    void Cassette::validateConfig() {
        Tape::Tape firstValidTape{};
        for (const auto &v : Tapes) {
            if (v.second.valid) {
                firstValidTape = v.second;
                break;
            }
        }

        Tape::Reel firstValidReel{};
        std::string firstValidReelName;
        for (const auto &v : Reels) {
            if (!v.second.empty()) {
                firstValidReel = v.second;
                firstValidReelName = v.first;
                break;
            }
        }

        for (auto const t : tapeTypes) {
            auto tape = config->Get(t)->tape;
            auto tapeName = tape;
            if (Tapes[tape].valid == false) {
                if (!firstValidTape.valid) {
                    DLOG("tape %s invalid, no suitable replacement found, exiting\n", tape.c_str());
                    exit(1);
                }

                DLOG("tape %s invalid, using first valid tape %s\n", tape.c_str(), firstValidTape.name.c_str());
                tapeName = firstValidTape.name;
            }

            auto reel = config->Get(t)->reel;
            auto reelName = reel;
            if (Reels[reel].empty()) {
                if (firstValidReel.empty()) {
                    DLOG("reel %s invalid, no suitable replacement found, exiting\n", reel.c_str());
                    exit(1);
                }

                DLOG("reel %s invalid, using first valid reel %s\n", reel.c_str(), firstValidReelName.c_str());
                reelName = firstValidReelName;
            }

            config->Set(t, {tapeName, reelName, config->Get(t)->name});
            //            DLOG("set %u %s %s %s\n", t, tapeName.c_str(), reelName.c_str(), config->Get(t)->name.c_str());
        }
    }

    void Cassette::LoadImages() {
        for (auto const t : tapeTypes) {
            LoadReel(config->Get(t)->reel);
            LoadTape(config->Get(t)->tape);
        }

        validateConfig();
    }

    int Cassette::Load(std::string filename, ImFont *FontRegular) {
        loading = true;

        SelectTape(true);
        LoadImages();

        AddFonts(FontRegular);

        loading = false;

        this->ReelThread();

        return 0;
    }

    void Cassette::randomizeTape() {
        auto index = std::rand() % tapeTypes.size();
        tapeType = tapeTypes[index];

        ActiveTape = &Tapes[config->Get(tapeType)->tape];
        assert(ActiveTape);
        ActiveReel = &Reels[config->Get(tapeType)->reel];
        assert(ActiveReel);

        ActiveTape->name = config->Get(tapeType)->tape;
        DLOG("tape: %s, reel %s\n", config->Get(tapeType)->tape.c_str(), config->Get(tapeType)->reel.c_str());
    }

    void Cassette::SelectTape(bool force) {
        if (connector->playlist.empty()) {
            DLOG("no songs in playlist\n");
            defaultTape();
            return;
        }

        if (Track == connector->playlist.at(0).File && force == false) {
            if (ActiveTape == nullptr || ActiveReel == nullptr) {
                defaultTape();
            }

            return;
        }

        if (connector->status.pollRunning) {
            if (ActiveTape == nullptr || ActiveReel == nullptr) {
                defaultTape();
            }

            return;
        }

        DLOG("track is %s, file is %s\n", Track.c_str(), connector->playlist.at(0).File.c_str());

        Track = connector->playlist.at(0).File;

        song = connector->playlist.at(0);

        if (!song.PlaylistStringsCalculated) {
            for (auto &c : song.Artist) {
                c = std::toupper(c);
            }
            for (auto &c : song.Title) {
                c = std::toupper(c);
            }

            // TODO replace with pure imgui
            song.Artist = CalculateTextWidth(song.Artist, Tapes[config->Get(tapeType)->tape].titleWidth);
            song.Title = CalculateTextWidth(song.Title, Tapes[config->Get(tapeType)->tape].titleWidth);
            song.PlaylistStringsCalculated = true;
        }

        if (config->randomize) {
            randomizeTape();
            return;
        }

        std::string codec;
#ifdef DESKTOP
        codec = split(connector->playlist.at(0).File, ".").back();
#else
        codec = connector->status.Codec;
#endif
        for (auto &c : codec) {
            c = std::tolower(c);
        }

        auto bitrate = connector->status.Bitrate;

        switch (hash(codec.c_str())) {
        case hash("wma"):
        case hash("aac"):
        case hash("mp3"):
            tapeType = Tape::MP3_320;
            if (bitrate < 129) {
                tapeType = Tape::MP3_128;
                break;
            }

            if (bitrate < 161) {
                tapeType = Tape::MP3_160;
                break;
            }

            if (bitrate < 257) {
                tapeType = Tape::MP3_256;
                break;
            }

            break;
        case hash("alac"):
        case hash("mqa"):
        case hash("flac"):
        case hash("ape"):
            tapeType = Tape::FLAC_ALAC_APE_MQA;
            if (connector->status.SampleRate > 44100 && connector->status.Bits > 16) {
                tapeType = Tape::FLAC_MQA_ALAC_PCM_AIFF_APE_HIRES;
            }
            break;
        case hash("pcm"):
            tapeType = Tape::PCM;
            if (connector->status.SampleRate > 44100 && connector->status.Bits > 16) {
                tapeType = Tape::FLAC_MQA_ALAC_PCM_AIFF_APE_HIRES;
            }
            break;
        case hash("dsd"):
            tapeType = Tape::DSD;
            break;
        case hash("aiff"):
            tapeType = Tape::FLAC_MQA_ALAC_PCM_AIFF_APE_HIRES;
            break;
        default:
            tapeType = Tape::MP3_320;
            break;
        }

        DLOG("tape: %s, reel %s\n", config->Get(tapeType)->tape.c_str(), config->Get(tapeType)->reel.c_str());
        ActiveTape = &Tapes[config->Get(tapeType)->tape];
        assert(ActiveTape);
        ActiveReel = &Reels[config->Get(tapeType)->reel];
        assert(ActiveReel);

        ActiveTape->name = config->Get(tapeType)->tape;

        if (debug) {
            DLOG("%d %s %d %d %d\n", bitrate, codec.c_str(), tapeType, connector->status.SampleRate, connector->status.Bits);
        }
    }

    void Cassette::defaultTape() {
        DLOG("\n");
        tapeType = Tape::MP3_320;
        ActiveTape = &Tapes[config->Get(tapeType)->tape];
        assert(ActiveTape);
        ActiveReel = &Reels[config->Get(tapeType)->reel];
        assert(ActiveReel);

        ActiveTape->name = config->Get(tapeType)->tape;
    }

    void Cassette::Draw() {
        if (loading) {
            return;
        }

        SelectTape();

        ActiveTape->Draw();

        if (!ActiveReel->empty()) {
            if (reelID > (ActiveReel->size() - 1)) {
                ActiveReel->at(0).DrawAt(ActiveTape->reelCoords);
            } else {
                ActiveReel->at(reelID).DrawAt(ActiveTape->reelCoords);
            }
        }

        if (debug) {
            drawCodecInfo();
        }
    }

    void Cassette::ReelThread() {
        reelThreadStop = false;
        auto exec = [this]() { this->ReelLoop(); };
        std::thread t(exec);
        t.detach();
    }

    void Cassette::ReelLoop() {
        for (;;) {
            std::this_thread::sleep_for(std::chrono::milliseconds(reelDelayMs));
            if (reelThreadStop) {
                break;
            }

            if (connector->status.State != "play") {
                continue;
            }

            if (this->reelID >= (this->Reels[config->Get(tapeType)->reel].size() - 1)) {
                this->reelID = 0;
            } else {
                this->reelID++;
            }
        }
    }

    void Cassette::drawCodecInfo() const {
        ImGui::SetCursorPos({600, 40});
        std::string codec;
#ifdef DESKTOP
        codec = split(connector->playlist.at(0).File, ".").back();
#else
        codec = connector->status.Codec;
#endif
        ImGui::Text("%s, %d", codec.c_str(), connector->status.Bitrate);
    }
} // namespace Cassette

#include "config.h"
#include "connector/mpd.h"
#include "util/util.h"
#include <libgen.h>
#include <sys/stat.h>

namespace AppConfig {

    enum ConfigErrors {
        FS_MOUNTED = 1,
        FILE_NOT_FOUND,
        SAVE_FAIL,
    };

    std::map<ESkinVariant, std::string> ESkinToName{{WINAMP, "winamp"}, {CASSETTE, "cassette"}};
    std::map<std::string, ESkinVariant> NameToESkin{{"winamp", WINAMP}, {"cassette", CASSETTE}};

#ifdef DESKTOP
    const char *defaultPath = "./config.ini";
#else
    const char *defaultPath = "/contents/wampy/config.ini";
#endif
    //    const char *defaultPath = "/tmp/test.ini";

    const char *configPaths[] = {
        "./config.ini",
        "../config.ini",
        "/contents_ext/wampy/config.ini",
        "/contents/wampy/config.ini",
        defaultPath,
    };

    void AppConfig::ToIni() {
        ini["wampy"]["badBoots"] = std::to_string(badBoots);
        ini["cassette:mp3_128"].set({{"tape", cassette.Get(Tape::MP3_128)->tape}, {"reel", cassette.Get(Tape::MP3_128)->reel}});

        ini["cassette:mp3_160"].set({{"tape", cassette.Get(Tape::MP3_160)->tape}, {"reel", cassette.Get(Tape::MP3_160)->reel}});

        ini["cassette:mp3_256"].set({{"tape", cassette.Get(Tape::MP3_256)->tape}, {"reel", cassette.Get(Tape::MP3_256)->reel}});
        ini["cassette:mp3_320"].set({{"tape", cassette.Get(Tape::MP3_320)->tape}, {"reel", cassette.Get(Tape::MP3_320)->reel}});

        ini["cassette:flac"].set(
            {{"tape", cassette.Get(Tape::FLAC_ALAC_APE_MQA)->tape}, {"reel", cassette.Get(Tape::FLAC_ALAC_APE_MQA)->reel}}
        );

        ini["cassette:aiff"].set({{"tape", cassette.Get(Tape::AIFF)->tape}, {"reel", cassette.Get(Tape::AIFF)->reel}});

        ini["cassette:pcm"].set({{"tape", cassette.Get(Tape::PCM)->tape}, {"reel", cassette.Get(Tape::PCM)->reel}});

        ini["cassette:hires"].set(
            {{"tape", cassette.Get(Tape::FLAC_MQA_ALAC_PCM_AIFF_APE_HIRES)->tape},
             {"reel", cassette.Get(Tape::FLAC_MQA_ALAC_PCM_AIFF_APE_HIRES)->reel}}
        );

        ini["cassette:dsd"].set({{"tape", cassette.Get(Tape::DSD)->tape}, {"reel", cassette.Get(Tape::DSD)->reel}});

        ini["cassette"]["randomize"] = std::to_string(cassette.randomize);

        ini["winamp"]["filename"] = winamp.filename;
        ini["winamp"]["bitmapFont"] = std::to_string(winamp.useBitmapFont);
        ini["winamp"]["bitmapFontInPlaylist"] = std::to_string(winamp.useBitmapFontInPlaylist);
        ini["winamp"]["preferTimeRemaining"] = std::to_string(winamp.preferTimeRemaining);

        ini["wampy"]["activeSkin"] = ESkinToName[activeSkin];

        ini["misc"]["swapTrackButtons"] = std::to_string(misc.swapTrackButtons);

        ini["features"]["bigCover"] = std::to_string(features.bigCover);
        ini["features"]["showTime"] = std::to_string(features.showTime);
        ini["debug"]["enabled"] = std::to_string(debug.enabled);
        ini["mpd"]["socketPath"] = MPDSocketPath;

#ifdef DESKTOP
        ini["fonts"]["chineseFull"] = std::to_string(fontRanges.ChineseFull);
#else
        ini["fonts"]["chineseFull"] = std::to_string(false);
#endif
        ini["fonts"]["cyrillic"] = std::to_string(fontRanges.Cyrillic);
        ini["fonts"]["greek"] = std::to_string(fontRanges.Greek);
        ini["fonts"]["japanese"] = std::to_string(fontRanges.Japanese);
        ini["fonts"]["korean"] = std::to_string(fontRanges.Korean);
        ini["fonts"]["thai"] = std::to_string(fontRanges.Thai);
        ini["fonts"]["vietnamese"] = std::to_string(fontRanges.Vietnamese);
    }

    void AppConfig::Default() {
        cassette.Default();
        winamp.Default();
        fontRanges = FontRanges{};

        activeSkin = WINAMP;
        MPDSocketPath = MPDDefaultAddress;

        ToIni();
    }

    int AppConfig::Load() {
        if (FindConfig() != 0) {
            filePath = defaultPath;
            if (Create() != 0) {
                DLOG("failed to create ini %s\n", filePath);
                exit(1);
            }
        }

        DLOG("using config %s\n", filePath);
        auto f = mINI::INIFile(filePath);
        f.read(ini);

        badBoots = std::atoi(ini["wampy"]["badBoots"].c_str());

        cassette.SetOrDefault(Tape::MP3_128, {ini["cassette:mp3_128"]["tape"], ini["cassette:mp3_128"]["reel"], "MP3 128kbps"});
        cassette.SetOrDefault(Tape::MP3_160, {ini["cassette:mp3_160"]["tape"], ini["cassette:mp3_160"]["reel"], "MP3 160kbps"});
        cassette.SetOrDefault(Tape::MP3_256, {ini["cassette:mp3_256"]["tape"], ini["cassette:mp3_256"]["reel"], "MP3 256kbps"});
        cassette.SetOrDefault(Tape::MP3_320, {ini["cassette:mp3_320"]["tape"], ini["cassette:mp3_320"]["reel"], "MP3 320kbps"});
        cassette.SetOrDefault(Tape::FLAC_ALAC_APE_MQA, {ini["cassette:flac"]["tape"], ini["cassette:flac"]["reel"], "FLAC"});
        cassette.SetOrDefault(Tape::AIFF, {ini["cassette:aiff"]["tape"], ini["cassette:aiff"]["reel"], "AIFF"});
        cassette.SetOrDefault(Tape::PCM, {ini["cassette:pcm"]["tape"], ini["cassette:pcm"]["reel"], "PCM"});
        cassette.SetOrDefault(
            Tape::FLAC_MQA_ALAC_PCM_AIFF_APE_HIRES, {ini["cassette:hires"]["tape"], ini["cassette:hires"]["reel"], "Hi-Res"}
        );
        cassette.SetOrDefault(Tape::DSD, {ini["cassette:dsd"]["tape"], ini["cassette:dsd"]["reel"], "DSD"});

        winamp.filename = ini["winamp"]["filename"];
        auto winampDefault = Winamp::Config::GetDefault();
        if (winamp.filename.empty()) {
            winamp.filename = winampDefault.filename;
        }

        // NOLINTBEGIN
        winamp.useBitmapFont = (bool)std::atoi(ini["winamp"]["bitmapFont"].c_str());
        if (ini["winamp"]["bitmapFont"].empty()) {
            winamp.useBitmapFont = winampDefault.useBitmapFont;
        }

        winamp.useBitmapFontInPlaylist = (bool)std::atoi(ini["winamp"]["bitmapFontInPlaylist"].c_str());
        winamp.preferTimeRemaining = (bool)std::atoi(ini["winamp"]["preferTimeRemaining"].c_str());
        cassette.randomize = (bool)std::atoi(ini["cassette"]["randomize"].c_str());
        // NOLINTEND

        auto activeSkinString = ini["wampy"]["activeSkin"];
        if (NameToESkin.count(activeSkinString) == 0) {
            activeSkin = WINAMP;
        } else {
            activeSkin = NameToESkin[activeSkinString];
        }

        // NOLINTBEGIN
        misc.swapTrackButtons = (bool)std::atoi(ini["misc"]["swapTrackButtons"].c_str());
        features.bigCover = (bool)std::atoi(ini["features"]["bigCover"].c_str());
        features.showTime = (bool)std::atoi(ini["features"]["showTime"].c_str());

        debug.enabled = (bool)std::atoi(ini["debug"]["enabled"].c_str());

#ifdef DESKTOP
        fontRanges.ChineseFull = (bool)std::atoi(ini["fonts"]["chineseFull"].c_str());
#else
        // device cannot handle full chinese range
        fontRanges.ChineseFull = false;
#endif
        fontRanges.Cyrillic = (bool)std::atoi(ini["fonts"]["cyrillic"].c_str());
        fontRanges.Greek = (bool)std::atoi(ini["fonts"]["greek"].c_str());
        fontRanges.Japanese = (bool)std::atoi(ini["fonts"]["japanese"].c_str());
        fontRanges.Korean = (bool)std::atoi(ini["fonts"]["korean"].c_str());
        fontRanges.Thai = (bool)std::atoi(ini["fonts"]["thai"].c_str());
        fontRanges.Vietnamese = (bool)std::atoi(ini["fonts"]["vietnamese"].c_str());
        // NOLINTEND

        MPDSocketPath = ini["mpd"]["socketPath"];

        return 0;
    }

    int AppConfig::Save() {
        ToIni();
        auto f = mINI::INIFile(filePath);
        if (!f.write(ini, true)) {
            return SAVE_FAIL;
        }

        return 0;
    }

    // no config file found, create one using filePath
    int AppConfig::Create() {
        Default();
        char *c = (char *)malloc(strlen(filePath) + 1);
        strcpy(c, filePath);
        mkpath(dirname(c), 0755);

        auto f = mINI::INIFile(filePath);
        if (!f.write(ini, true)) {
            return SAVE_FAIL;
        }

        return 0;
    }

    int AppConfig::FindConfig() {
        if (IsMounted()) {
            return FS_MOUNTED;
        }

        struct stat sb {};

        for (const auto &f : configPaths) {
            if (stat(f, &sb) == 0) {
                filePath = f;
                return 0;
            }
        }

        return FILE_NOT_FOUND;
    }
} // namespace AppConfig
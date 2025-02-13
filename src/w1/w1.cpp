#include "w1.h"
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>

namespace W1 {
    std::map<std::string, uint> colorByName = {{"Default", 0}, {"Peach", 3}, {"Red", 5}, {"Blue", 7}, {"Green", 9}};
    std::map<uint, std::string> colorByValue = {{0, "Default"}, {3, "Peach"}, {5, "Red"}, {7, "Blue"}, {9, "Green"}};

    std::map<std::string, uint> colorByNameWalkmanOne = {{"Default", 0}, {"Peach", 1}, {"Red", 2}, {"Blue", 3}, {"Green", 4}};
    std::map<uint, std::string> colorByValueWalkmanOne = {{0, "Default"}, {1, "Peach"}, {2, "Red"}, {3, "Blue"}, {4, "Green"}};

    std::map<std::string, uint> signatureByNameWalkmanOne = {
        {"Neutral", 0}, {"Warm (Midnight v2)", 1}, {"Bright (Dawn v2.1)", 2}, {"WM1Z", 3}};
    std::map<uint, std::string> signatureByValueWalkmanOne = {
        {0, "Neutral"}, {1, "Warm (Midnight v2)"}, {2, "Bright (Dawn v2.1)"}, {3, "WM1Z"}};

    std::vector<std::string> regionWalkmanOne = {
        "J",    // 0x00000000
        "U",    // 0x00000001, US, CND : Canadian model
        "U2",   // 0x00000101
        "U3",   // 0x00000201
        "CA",   // 0x00000301
        "CEV",  // 0x00000002, EE : Russian model (CIS area: Except for Moldova)
        "CE7",  // 0x00000102
        "CEW",  // 0x00000003
        "CEW2", // 0x00000103, AEP : European, East European and Moldova models, aka UK,
        "CN",   // 0x00000004, CH : Chinese model
        "KR",   // 0x00000005
        "E",    // 0x00000006, AUS : Australian model, JE : Tourist model
        "MX",   // 0x00000203 is actually a KR3, typo in walkman one
        "KR3",  // 0x00000203
        "E2",   // 0x00000206
        "MX3",  // 0x00000306
        "TW",   // 0x00000007
    };

    std::map<uint, std::string> signatureToPathWalkmanOne = {
        {0, "/contents/CFW/External_Tunings/Neutral_&_Warm_external_tuning/"},
        {1, "/contents/CFW/External_Tunings/Neutral_&_Warm_external_tuning/"},
        {2, "/contents/CFW/External_Tunings/Bright_external_tuning/"},
        {3, "/contents/CFW/External_Tunings/WM1Z_external_tuning/"}};

    std::map<uint, std::string> signatureToSigNameWalkmanOne = {{0, "neutral"}, {1, "warm"}, {2, "bright"}, {3, "wm1z"}};

    std::map<uint, std::string> modelByID = {{0x21000004, "NW-A50Series"}, {0x21000008, "NW-WM1Z"}};

#ifndef DESKTOP
    const char *settingsPath = "/contents/CFW/settings.txt";
#else
    const char *settingsPath = "../settings.txt";
#endif

    uint defaultColor = 0;

    void SetColor(uint color) {
        char command[30];
        if (colorByValue.find(color) == colorByValue.end()) {
            DLOG("unknown color 0x%.8x\n", color);
            return;
        }

        snprintf(command, 30, "nvpflag -x clv 0x%.8x", color);
        DLOG("command %s\n", command);
        auto code = system(command);
        if (code != 0) {
            DLOG("failure, code %d, setting color to default\n", code);
            system("nvpflag clv 0x00000000");
            return;
        }

        DLOG("new color is %s (0x%.8x)\n", colorByValue.at(color).c_str(), color);
    }

    int ParseSettings(WalkmanOneOptions *w) {
        struct stat info {};

        if (stat(settingsPath, &info) == 0) {
            w->configFound = true;
        } else {
            DLOG("Walkman One config not found\n");
            return -1;
        }

        std::ifstream infile(settingsPath);
        std::string line;

        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            if (line.empty()) {
                continue;
            }

            if (line.at(0) == '#') {
                continue;
            }

            std::string key;
            auto kv = split(line, "=");
            if (kv.size() != 2) {
                continue;
            }

            //            DLOG("key %s, value %s\n", kv.at(0).c_str(), kv.at(1).c_str());
            switch (hash(kv.at(0).c_str())) {
            case hash("SIG"):
                w->signature = std::atoi(kv.at(1).c_str());
                break;
            case hash("REG"):
                strncpy(w->region, kv.at(1).c_str(), sizeof w->region);
                break;
            case hash("REM"):
                w->remote = (bool)std::atoi(kv.at(1).c_str());
                break;
            case hash("PMV"):
                w->plusModeVersion = std::atoi(kv.at(1).c_str());
                break;
            case hash("PMD"):
                w->plusModeByDefault = (bool)std::atoi(kv.at(1).c_str());
                break;
            case hash("GMD"):
                w->gainMode = (bool)std::atoi(kv.at(1).c_str());
                break;
            case hash("DIM"):
                w->dacInitializationMode = (bool)std::atoi(kv.at(1).c_str());
                break;
            case hash("COL"):
                w->color = std::atoi(kv.at(1).c_str());
                break;
            default:
                DLOG("unexpected key %s\n", kv.at(0).c_str());
                break;
            }
        }

        w->plusModeVersionBOOL = false;
        if (w->plusModeVersion == 2) {
            w->plusModeVersionBOOL = true;
        }

        return 0;
    }

    void WalkmanOneOptions::Save() {
        std::ifstream infile(settingsPath);
        std::string tempConfig = std::string(settingsPath) + ".delme";
        std::ofstream outfile(tempConfig);
        std::string line;

        plusModeVersion = 1;
        if (plusModeVersionBOOL) {
            plusModeVersion = 2;
        }

        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            if (line == "\n" || line == "\r\n") {
                outfile << line << "\n";
                continue;
            }

            if (line.empty()) {
                outfile << line << "\n";
                continue;
            }

            if (line.at(0) == '#') {
                outfile << line << "\n";
                continue;
            }

            std::string key;
            auto kv = split(line, "=");
            if (kv.size() != 2) {
                outfile << line << "\n";
                continue;
            }

            //            DLOG("key %s, value %s\n", kv.at(0).c_str(), kv.at(1).c_str());
            switch (hash(kv.at(0).c_str())) {
            case hash("SIG"):
                outfile << kv.at(0) << "=" << signature << "\n";
                break;
            case hash("REG"):
                outfile << kv.at(0) << "=" << region << "\n";
                break;
            case hash("REM"):
                outfile << kv.at(0) << "=" << remote << "\n";
                break;
            case hash("PMV"):
                outfile << kv.at(0) << "=" << plusModeVersion << "\n";
                break;
            case hash("PMD"):
                outfile << kv.at(0) << "=" << plusModeByDefault << "\n";
                break;
            case hash("GMD"):
                outfile << kv.at(0) << "=" << gainMode << "\n";
                break;
            case hash("DIM"):
                outfile << kv.at(0) << "=" << dacInitializationMode << "\n";
                break;
            case hash("COL"):
                outfile << kv.at(0) << "=" << color << "\n";
                break;
            default:
                outfile << line << "\n";
                break;
            }
        }

        std::remove(settingsPath);
        int err = std::rename(tempConfig.c_str(), settingsPath);
        if (err != 0) {
            std::perror("Error renaming");
            DLOG("failed to save config\n");
        }
    }

    /*
     * if signature is 0 or 2 (bright/neutral), walkman one boot script restores nvp config from backup
     * after that you are expected to run exe, which is not user-friendly
     * it is possible to circumvent some checks, but I want to keep stuff as straightforward as possible
    bool WalkmanOneOptions::CanApplyTuning() {
        int code = system("busybox diff /opt2/stock/nv_bk /dev/block/mmcblk0p3");
        if (code == 0) {
            DLOG("settings were restored from backup file, can apply tuning\n");
            return true;
        }

        return false;
    }
     */

    // system() sucks, but it's faster than dealing with fork()
    void WalkmanOneOptions::ApplyTuning() const {
        struct stat info {};

        if (signatureToPathWalkmanOne.count(signature) == 0) {
            DLOG("invalid signature %d\n", signature);
            return;
        }

        std::string upg = signatureToPathWalkmanOne.at(signature) + "/Data/Device/NW_WM_FW.UPG";

        if (stat(upg.c_str(), &info) != 0) {
            DLOG("UPG %s not found\n", upg.c_str());
            return;
        }

        std::string workdir = "/opt2/wampy-tuning-workdir/";
        system("mount -o remount,rw /emmc@option2 /opt2");
        int err = mkpath(workdir.c_str(), 0755);
        if (err != 0) {
            DLOG("failed to make path %s\n", workdir.c_str());
            return;
        }

        std::string command =
            "/system/vendor/unknown321/bin/upgtool-linux-arm5 -w -m nw-wm1a -z 2 -z 3 -e -o " + workdir + " '" + upg + "'";
        DLOG("command %s\n", command.c_str());
        err = system(command.c_str());
        if (err != 0) {
            DLOG("failed to unpack, code %d\n", err);
            return;
        }

        system("hagodaemons.sh stop");

        std::ofstream outfile("/opt2/sig");
        outfile << signatureToSigNameWalkmanOne.at(signature) << "\n";
        outfile.close();

        command = "dd if=" + workdir + "/2.bin of=/dev/block/mmcblk0p3";
        DLOG("command %s\n", command.c_str());
        err = system(command.c_str());
        if (err != 0) {
            DLOG("failed to flash part A, code %d\n", err);
            return;
        }

        command = "dd if=" + workdir + "/3.bin of=/dev/block/mmcblk0p7";
        DLOG("command %s\n", command.c_str());
        err = system(command.c_str());
        if (err != 0) {
            DLOG("failed to flash part B, code %d\n", err);
            return;
        }
    }

    void WalkmanOneOptions::Reboot() const {
        if (tuningChanged) {
            ApplyTuning();
        }

        system("sync");
        system("reboot");
    }

    void SetModelName() {
        // nvpstr fpi
    }

    void SetModelID() {
        // nvpflag mid
    }

} // namespace W1
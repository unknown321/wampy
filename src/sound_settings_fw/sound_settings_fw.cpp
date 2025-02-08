#include "sound_settings_fw.h"
#include "../util/util.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

void SoundSettingsFww::Start() {
#ifdef DESKTOP
    s = new sound_settings_fw();
    char a[11][50] = {
        "dynamicnormalizer", "dseehxlegacy", "attn", "vpt", "eq6band", "eq10band", "eqtone", "dcphaselinear", "vinylizer", "clearphase"};
    int i = 0;
    for (auto v : a) {
        strcpy(s->FilterStatus[i].name, v);
        s->FilterStatus[i].is_proc = true;
        i++;
    }
    return;
#endif
    auto fullpath = std::string("/dev/shm") + SSFW_SHMPATH;
    while (!exists(fullpath)) {
        DLOG("waiting for %s\n", fullpath.c_str());
        sleep(1);
    }

    auto fd = shm_open(SSFW_SHMPATH, O_RDWR, 0);
    if (fd == -1)
        errExit("shm_open");

    s = static_cast<sound_settings_fw *>(mmap(nullptr, sizeof(sound_settings_fw), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (s == MAP_FAILED)
        errExit("mmap");

    DLOG("SoundSettingsFww started\n");
}

void SoundSettingsFww::Send() const {
#ifdef DESKTOP
    return;
#endif
    if (sem_post(&s->sem1) == -1)
        errExit("sem_post");
    if (sem_wait(&s->sem2) == -1)
        errExit("sem_wait");
}

void SoundSettingsFww::Print() {
    Update();
    for (auto v : s->FilterStatus) {
        DLOG("%s %d\n", v.name, v.is_proc);
    }
}

void SoundSettingsFww::Update() {
    DLOG("\n");
    s->command.id = SSFW_UPDATE;
    Send();

    filters.clear();
    for (auto v : s->FilterStatus) {
        if (v.name[0] == '\0') {
            continue;
        }
        filters[v.name] = v.is_proc;
    }
    filters[filterInvalid] = false;

    char *endptr;
    for (int i = 0; i < eq10bands.size(); i++) {
        s->command.id = SSFW_GET_PARAM;
        memset(s->command.resultChar, 0, sizeof s->command.resultChar);
        sprintf(s->command.valueChar, "eq10band,band=%d", i);
        Send();
#ifdef DESKTOP
        strcpy(s->command.resultChar, "1");
#endif
        eq10bands.at(i).second = std::strtol(s->command.resultChar, &endptr, 10);
    }

    for (int i = 0; i < eq6bands.size(); i++) {
        s->command.id = SSFW_GET_PARAM;
        memset(s->command.resultChar, 0, sizeof s->command.resultChar);
        sprintf(s->command.valueChar, "eq6band,band=%d", i);
        Send();
#ifdef DESKTOP
        strcpy(s->command.resultChar, "1");
#endif
        eq6bands.at(i).second = std::strtol(s->command.resultChar, &endptr, 10);
    }

    strcpy(s->command.valueChar, "vinylizer,type");
    s->command.id = SSFW_GET_PARAM;
    Send();
#ifdef DESKTOP
    strcpy(s->command.resultChar, "1");
#endif
    vinylizerValue = std::strtol(s->command.resultChar, &endptr, 10);

    strcpy(s->command.valueChar, "vpt,mode");
    s->command.id = SSFW_GET_PARAM;
    Send();
#ifdef DESKTOP
    strcpy(s->command.resultChar, "1");
#endif
    vptValue = std::strtol(s->command.resultChar, &endptr, 10);

    strcpy(s->command.valueChar, "dcphaselinear,filtertype");
    s->command.id = SSFW_GET_PARAM;
    Send();
#ifdef DESKTOP
    strcpy(s->command.resultChar, "1");
#endif
    dcValue = std::strtol(s->command.resultChar, &endptr, 10);

    strcpy(s->command.valueChar, "eq6band,preset");
    s->command.id = SSFW_GET_PARAM;
    Send();
#ifdef DESKTOP
    strcpy(s->command.resultChar, "2");
#endif
    eq6Value = std::strtol(s->command.resultChar, &endptr, 10);
}

void SoundSettingsFww::SetEq10Band(int band, int value) const {
    s->command.id = SSFW_SET_PARAM;
    sprintf(s->command.valueChar, "eq10band,band=%d,value=%d", band, value);
    DLOG("%s\n", s->command.valueChar);
    Send();
}

void SoundSettingsFww::SetEq6Band(int band, int value) const {
    s->command.id = SSFW_SET_PARAM;
    sprintf(s->command.valueChar, "eq6band,band=%d,value=%d", band, value);
    DLOG("%s\n", s->command.valueChar);
    Send();
}

void SoundSettingsFww::SetFilter(const std::string &name, bool value) const {
    s->command.id = SSFW_SET_FILTER;
    strncpy(s->command.valueChar, name.c_str(), sizeof s->command.valueChar);
    s->command.valueBool = value;
    DLOG("filter %s, value %d\n", name.c_str(), value);
    Send();
}

void SoundSettingsFww::SetEqTone(int type, int value) const {
    s->command.id = SSFW_SET_PARAM;
    sprintf(s->command.valueChar, "eqtone,type=%d,value=%d", type, value);
    DLOG("%s\n", s->command.valueChar);
    Send();
}

void SoundSettingsFww::SetEqToneFreq(int type, int value) const {
    s->command.id = SSFW_SET_PARAM;
    sprintf(s->command.valueChar, "eqtone,type=%d,centerfreq=%d", type, value);
    DLOG("%s\n", s->command.valueChar);
    Send();
}

void SoundSettingsFww::SetVinylizerType(int value) const {
    s->command.id = SSFW_SET_PARAM;
    sprintf(s->command.valueChar, "vinylizer,type=%d", value);
    DLOG("%s\n", s->command.valueChar);
    Send();
}

void SoundSettingsFww::SetVptMode(int value) const {
    s->command.id = SSFW_SET_PARAM;
    sprintf(s->command.valueChar, "vpt,mode=%d", value);
    DLOG("%s\n", s->command.valueChar);
    Send();
}

void SoundSettingsFww::SetDcFilterType(int value) const {
    s->command.id = SSFW_SET_PARAM;
    sprintf(s->command.valueChar, "dcphaselinear,filtertype=%d", value);
    DLOG("%s\n", s->command.valueChar);
    Send();
}

void SoundSettingsFww::SetEq6Preset(int value) const {
    s->command.id = SSFW_SET_PARAM;
    sprintf(s->command.valueChar, "eq6band,preset=%d", value);
    DLOG("%s\n", s->command.valueChar);
    Send();
}

void SoundSettingsFww::SetDirectSource(bool value) const {
    if (!s->directSourcePresent) {
        s->directSourceOn = value;
    }

    s->command.id = SSFW_SET_PARAM;
    sprintf(s->command.valueChar, "sourcedirect=%s", value ? "on" : "off");
    DLOG("%s\n", s->command.valueChar);
    Send();
}

void SoundSettingsFww::SetClearAudioPlus(bool value) const {
    if (!s->clearAudioPlusPresent) {
        s->clearAudioPlusOn = value;
    }
    s->command.id = SSFW_SET_PARAM;
    sprintf(s->command.valueChar, "clearaudioplus=%s", value ? "on" : "off");
    DLOG("%s\n", s->command.valueChar);
    Send();
}

#ifndef PSHM_UCASE_H
#define PSHM_UCASE_H

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <semaphore.h>

#define errExit(msg)                                                                                                                       \
    do {                                                                                                                                   \
        perror(msg);                                                                                                                       \
        exit(EXIT_FAILURE);                                                                                                                \
    } while (0)

#define SHMPATH "/sound_settings"

struct sound_settings_status {
    int vptOn;
    int vptMode;
    int clearPhaseOn;
    int DNOn;   // ?
    int dseeOn; // is dsee featured?
    int dseeCustOn;
    int dseeCustMode;
    int eq6On;
    int eq6Preset;
    int eq6Bands[6];
    int eq10On;
    int eq10Preset;
    int eq10Bands[10];
    int toneControlOn;
    int toneControlLow;
    int toneControlMid;
    int toneControlHigh;
    int toneControlLowFreq;
    int toneControlMidFreq;
    int toneControlHighFreq;
    int eqUse; // 2 == 10band, 3 == toneControl / 6band
    int dcLinearOn;
    int dcLinearFilter;
    int clearAudioOn;
    int directSourceOn;
    int masterVolume;
    int dseeHXOn; // dsee hx on nw-a50
    int vinylOn;
    int vinylType;

    int clearAudioAvailable;
    int directSourceAvailable;
};

enum EPstServerCommand {
    PSC_UNKNOWN = 0,
    PSC_UPDATE = 1,
    PSC_SET_FM = 2,
    PSC_SET_FM_FREQ = 3,
    PSC_SET_FM_STEREO = 4,
};

struct PstServerCommand {
    EPstServerCommand id = PSC_UNKNOWN;
    int valueInt = 0;
};

#define FM_FREQ_MIN 76000
#define FM_FREQ_MAX 108000

struct FmStatus {
    int state;
    int freq = FM_FREQ_MIN;
    bool stereo = false;
};

struct sound_settings {
    sem_t sem1{}; /* POSIX unnamed semaphore */
    sem_t sem2{}; /* POSIX unnamed semaphore */

    PstServerCommand command{};
    sound_settings_status status{};
    FmStatus fmStatus{};

    void Print() {
        printf("clearAudioAvailable: %d\n", status.clearAudioAvailable);
        printf("directSourceAvailable: %d\n", status.directSourceAvailable);
        printf("vptOn: %d\n", status.vptOn);
        printf("vptMode: %d\n", status.vptMode);
        printf("clearPhaseOn: %d\n", status.clearPhaseOn);
        printf("DNOn: %d\n", status.DNOn);
        printf("dseeOn: %d\n", status.dseeOn);
        printf("dseeCustOn: %d\n", status.dseeCustOn);
        printf("dseeCustMode: %d\n", status.dseeCustMode);
        printf("eq6On: %d\n", status.eq6On);
        printf("eq6Preset: %d\n", status.eq6Preset);
        for (const auto i : status.eq6Bands) {
            printf("eq6Bands: %d\n", i);
        }
        printf("eq10On: %d\n", status.eq10On);
        printf("eq10Preset: %d\n", status.eq10Preset);
        for (const auto i : status.eq10Bands) {
            printf("eq10Bands: %d\n", i);
        }

        printf("toneControlOn: %d\n", status.toneControlOn);
        printf("toneControlLow: %d\n", status.toneControlLow);
        printf("toneControlMid: %d\n", status.toneControlMid);
        printf("toneControlHigh: %d\n", status.toneControlHigh);
        printf("toneControlLowFreq: %d\n", status.toneControlLowFreq);
        printf("toneControlMidFreq: %d\n", status.toneControlMidFreq);
        printf("toneControlHighFreq: %d\n", status.toneControlHighFreq);
        printf("eqUse: %d\n", status.eqUse);
        printf("dcLinearOn: %d\n", status.dcLinearOn);
        printf("dcLinearFilter: %d\n", status.dcLinearFilter);
        printf("clearAudioOn: %d\n", status.clearAudioOn);
        printf("directSourceOn: %d\n", status.directSourceOn);
        printf("masterVolume: %d\n", status.masterVolume);
        printf("dseeHXOn: %d\n", status.dseeHXOn);
        printf("vinylOn: %d\n", status.vinylOn);
        printf("vinylType: %d\n", status.vinylType);
    }
};

#endif

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

struct sound_settings {
    sem_t sem1; /* POSIX unnamed semaphore */
    sem_t sem2; /* POSIX unnamed semaphore */

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

    void Print() {
        printf("clearAudioAvailable: %d\n", clearAudioAvailable);
        printf("directSourceAvailable: %d\n", directSourceAvailable);
        printf("vptOn: %d\n", vptOn);
        printf("vptMode: %d\n", vptMode);
        printf("clearPhaseOn: %d\n", clearPhaseOn);
        printf("DNOn: %d\n", DNOn);
        printf("dseeOn: %d\n", dseeOn);
        printf("dseeCustOn: %d\n", dseeCustOn);
        printf("dseeCustMode: %d\n", dseeCustMode);
        printf("eq6On: %d\n", eq6On);
        printf("eq6Preset: %d\n", eq6Preset);
        for (const auto i : eq6Bands) {
            printf("eq6Bands: %d\n", i);
        }
        printf("eq10On: %d\n", eq10On);
        printf("eq10Preset: %d\n", eq10Preset);
        for (const auto i : eq10Bands) {
            printf("eq10Bands: %d\n", i);
        }

        printf("toneControlOn: %d\n", toneControlOn);
        printf("toneControlLow: %d\n", toneControlLow);
        printf("toneControlMid: %d\n", toneControlMid);
        printf("toneControlHigh: %d\n", toneControlHigh);
        printf("toneControlLowFreq: %d\n", toneControlLowFreq);
        printf("toneControlMidFreq: %d\n", toneControlMidFreq);
        printf("toneControlHighFreq: %d\n", toneControlHighFreq);
        printf("eqUse: %d\n", eqUse);
        printf("dcLinearOn: %d\n", dcLinearOn);
        printf("dcLinearFilter: %d\n", dcLinearFilter);
        printf("clearAudioOn: %d\n", clearAudioOn);
        printf("directSourceOn: %d\n", directSourceOn);
        printf("masterVolume: %d\n", masterVolume);
        printf("dseeHXOn: %d\n", dseeHXOn);
        printf("vinylOn: %d\n", vinylOn);
        printf("vinylType: %d\n", vinylType);
    }
};

#endif

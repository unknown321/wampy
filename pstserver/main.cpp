#include "dmpclient.h"
#include "dmpconfig.h"
#include "pshm_ucase.h"
#include <cstdio>
#include <map>
#include <unistd.h>

#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

#include "thread"

void healthcheck() {
    // do nothing
}

void dumpSoundSettings(
    pst::dmpconfig::DmpConfig *dmpConfig, pst::dmpconfig::Key *k, pst::dmpfeature::DmpFeature *dmpFeature, sound_settings *a
) {
    k->value = DMP_CONFIG_SS_VPT_ONOFF;
    dmpConfig->Get(*k, a->vptOn);

    k->value = DMP_CONFIG_SS_VPT_MODE;
    dmpConfig->Get(*k, a->vptMode);

    k->value = DMP_CONFIG_SS_CPHP_ONOFF;
    dmpConfig->Get(*k, a->clearPhaseOn);

    k->value = DMP_CONFIG_SS_DSEE_ONOFF;
    dmpConfig->Get(*k, a->dseeOn);

    k->value = DMP_CONFIG_SS_DSEECUST_ONOFF;
    dmpConfig->Get(*k, a->dseeCustOn);

    k->value = DMP_CONFIG_SS_DSEECUST_MODE;
    dmpConfig->Get(*k, a->dseeCustMode);

    k->value = DMP_CONFIG_SS_DN_ONOFF;
    dmpConfig->Get(*k, a->DNOn);

    k->value = DMP_CONFIG_SS_EQ6_ONOFF;
    dmpConfig->Get(*k, a->eq6On);

    k->value = DMP_CONFIG_SS_EQ6_PRESET;
    dmpConfig->Get(*k, a->eq6Preset);

    k->value = DMP_CONFIG_SS_EQ_USE;
    dmpConfig->Get(*k, a->eqUse);

    std::vector<int> bands6 = {
        DMP_CONFIG_SS_EQ6_BAND1,
        DMP_CONFIG_SS_EQ6_BAND2,
        DMP_CONFIG_SS_EQ6_BAND3,
        DMP_CONFIG_SS_EQ6_BAND4,
        DMP_CONFIG_SS_EQ6_BAND5,
        DMP_CONFIG_SS_EQ6_BAND6,
    };

    for (int i = 0; i < bands6.size(); i++) {
        k->value = bands6[i];
        dmpConfig->Get(*k, a->eq6Bands[i]);
    }

    k->value = DMP_CONFIG_SS_EQ10_ONOFF;
    dmpConfig->Get(*k, a->eq10On);

    k->value = DMP_CONFIG_SS_EQ10_PRESET;
    dmpConfig->Get(*k, a->eq10Preset);

    std::vector<int> bands10 = {
        DMP_CONFIG_SS_EQ10_BAND01,
        DMP_CONFIG_SS_EQ10_BAND02,
        DMP_CONFIG_SS_EQ10_BAND03,
        DMP_CONFIG_SS_EQ10_BAND04,
        DMP_CONFIG_SS_EQ10_BAND05,
        DMP_CONFIG_SS_EQ10_BAND06,
        DMP_CONFIG_SS_EQ10_BAND07,
        DMP_CONFIG_SS_EQ10_BAND08,
        DMP_CONFIG_SS_EQ10_BAND09,
        DMP_CONFIG_SS_EQ10_BAND10,
    };

    for (int i = 0; i < bands10.size(); i++) {
        k->value = bands10[i];
        dmpConfig->Get(*k, a->eq10Bands[i]);
    }

    k->value = DMP_CONFIG_SS_TONE_ONOFF;
    dmpConfig->Get(*k, a->toneControlOn);

    k->value = DMP_CONFIG_SS_TONE_LOW;
    dmpConfig->Get(*k, a->toneControlLow);

    k->value = DMP_CONFIG_SS_TONE_MID;
    dmpConfig->Get(*k, a->toneControlMid);

    k->value = DMP_CONFIG_SS_TONE_HIGH;
    dmpConfig->Get(*k, a->toneControlHigh);

    k->value = DMP_CONFIG_SS_TONE_LOW_FREQ;
    dmpConfig->Get(*k, a->toneControlLowFreq);

    k->value = DMP_CONFIG_SS_TONE_MID_FREQ;
    dmpConfig->Get(*k, a->toneControlMidFreq);

    k->value = DMP_CONFIG_SS_TONE_HIGH_FREQ;
    dmpConfig->Get(*k, a->toneControlHighFreq);

    k->value = DMP_CONFIG_SS_DCLINEAR_ONOFF;
    dmpConfig->Get(*k, a->dcLinearOn);

    k->value = DMP_CONFIG_SS_DCLINEAR_FILTER;
    dmpConfig->Get(*k, a->dcLinearFilter);

    k->value = DMP_CONFIG_SS_CAPLUS_ONOFF;
    dmpConfig->Get(*k, a->clearAudioOn);

    k->value = DMP_CONFIG_SS_SRCDIRECT_ONOFF;
    dmpConfig->Get(*k, a->directSourceOn);

    k->value = DMP_CONFIG_VOL_MASTERVOL;
    dmpConfig->Get(*k, a->masterVolume);

    k->value = DMP_CONFIG_DSEEAI_ONOFF;
    dmpConfig->Get(*k, a->dseeHXOn);

    k->value = DMP_CONFIG_VINYL_ONOFF;
    dmpConfig->Get(*k, a->vinylOn);

    k->value = DMP_CONFIG_VINYL_TYPE;
    dmpConfig->Get(*k, a->vinylType);

    a->clearAudioAvailable = dmpFeature->IsFeaturedClearAudioPlus();
    a->directSourceAvailable = dmpFeature->IsFeaturedSourceDirect();
}

int main(int argc, char **argv) {
    if (argc > 1) {
        auto delay = strtol(argv[1], nullptr, 0);
        printf("sleeping for %d\n", delay);
        sleep(delay);
    }

    int fd;
    struct sound_settings *shmp;

    printf("starting\n");

    shm_unlink(SHMPATH);

    fd = shm_open(SHMPATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1)
        errExit("shm_open");

    if (ftruncate(fd, sizeof(struct sound_settings)) == -1)
        errExit("ftruncate");

    /* Map the object into the caller's address space. */

    shmp = static_cast<sound_settings *>(mmap(nullptr, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (shmp == MAP_FAILED)
        errExit("mmap");

    /* Initialize semaphores as process-shared, with value 0. */

    if (sem_init(&shmp->sem1, 1, 0) == -1)
        errExit("sem_init-sem1");
    if (sem_init(&shmp->sem2, 1, 0) == -1)
        errExit("sem_init-sem2");

    /* Wait for 'sem1' to be posted by peer before touching
       shared memory. */

    auto pump = []() {
        auto framework = pst::core::Framework::GetReference();
        framework->StartForApplication(healthcheck, true);
        while (true) {
            framework->Pump(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        framework->Stop();
    };

    std::thread ttt(pump);
    ttt.detach();

    sleep(1);

    auto dmpconfig = new pst::dmpconfig::DmpConfig();
    auto k = pst::dmpconfig::Key();
    auto dmpFeature = pst::dmpfeature::DmpFeature();

    while (true) {
        if (sem_wait(&shmp->sem1) == -1)
            errExit("sem_wait");

        printf("request\n");
        dumpSoundSettings(dmpconfig, &k, &dmpFeature, shmp);

        /* Post 'sem2' to tell the peer that it can now
           access the modified data in shared memory. */

        if (sem_post(&shmp->sem2) == -1)
            errExit("sem_post");
    }

    /* Unlink the shared memory object. Even if the peer process
       is still using the object, this is okay. The object will
       be removed only after all open references are closed. */

    shm_unlink(SHMPATH);
}
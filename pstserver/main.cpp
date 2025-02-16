#include "dmpclient.h"
#include "dmpconfig.h"
#include "pshm_ucase.h"
#include <cstdio>
#include <unistd.h>

#include <fcntl.h>
#include <fstream>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "string.h"
#include "thread"

void healthcheck() {
    // do nothing
}

pst::services::TunerPlayerServiceClient *tunerClient;
pst::services::audioanalyzerservice::AudioAnalyzerService *analyzer;
pst::services::audioanalyzerservice::EventListener analyzerEventListener;
struct sound_settings *shmp;

void Update(pst::dmpconfig::DmpConfig *dmpConfig, pst::dmpconfig::Key *k, pst::dmpfeature::DmpFeature *dmpFeature, sound_settings *a) {
    k->value = DMP_CONFIG_SS_VPT_ONOFF;
    dmpConfig->Get(*k, a->status.vptOn);

    k->value = DMP_CONFIG_SS_VPT_MODE;
    dmpConfig->Get(*k, a->status.vptMode);

    k->value = DMP_CONFIG_SS_CPHP_ONOFF;
    dmpConfig->Get(*k, a->status.clearPhaseOn);

    k->value = DMP_CONFIG_SS_DSEE_ONOFF;
    dmpConfig->Get(*k, a->status.dseeOn);

    k->value = DMP_CONFIG_SS_DSEECUST_ONOFF;
    dmpConfig->Get(*k, a->status.dseeCustOn);

    k->value = DMP_CONFIG_SS_DSEECUST_MODE;
    dmpConfig->Get(*k, a->status.dseeCustMode);

    k->value = DMP_CONFIG_SS_DN_ONOFF;
    dmpConfig->Get(*k, a->status.DNOn);

    k->value = DMP_CONFIG_SS_EQ6_ONOFF;
    dmpConfig->Get(*k, a->status.eq6On);

    k->value = DMP_CONFIG_SS_EQ6_PRESET;
    dmpConfig->Get(*k, a->status.eq6Preset);

    k->value = DMP_CONFIG_SS_EQ_USE;
    dmpConfig->Get(*k, a->status.eqUse);

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
        dmpConfig->Get(*k, a->status.eq6Bands[i]);
    }

    k->value = DMP_CONFIG_SS_EQ10_ONOFF;
    dmpConfig->Get(*k, a->status.eq10On);

    k->value = DMP_CONFIG_SS_EQ10_PRESET;
    dmpConfig->Get(*k, a->status.eq10Preset);

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
        dmpConfig->Get(*k, a->status.eq10Bands[i]);
    }

    k->value = DMP_CONFIG_SS_TONE_ONOFF;
    dmpConfig->Get(*k, a->status.toneControlOn);

    k->value = DMP_CONFIG_SS_TONE_LOW;
    dmpConfig->Get(*k, a->status.toneControlLow);

    k->value = DMP_CONFIG_SS_TONE_MID;
    dmpConfig->Get(*k, a->status.toneControlMid);

    k->value = DMP_CONFIG_SS_TONE_HIGH;
    dmpConfig->Get(*k, a->status.toneControlHigh);

    k->value = DMP_CONFIG_SS_TONE_LOW_FREQ;
    dmpConfig->Get(*k, a->status.toneControlLowFreq);

    k->value = DMP_CONFIG_SS_TONE_MID_FREQ;
    dmpConfig->Get(*k, a->status.toneControlMidFreq);

    k->value = DMP_CONFIG_SS_TONE_HIGH_FREQ;
    dmpConfig->Get(*k, a->status.toneControlHighFreq);

    k->value = DMP_CONFIG_SS_DCLINEAR_ONOFF;
    dmpConfig->Get(*k, a->status.dcLinearOn);

    k->value = DMP_CONFIG_SS_DCLINEAR_FILTER;
    dmpConfig->Get(*k, a->status.dcLinearFilter);

    k->value = DMP_CONFIG_SS_CAPLUS_ONOFF;
    dmpConfig->Get(*k, a->status.clearAudioOn);

    k->value = DMP_CONFIG_SS_SRCDIRECT_ONOFF;
    dmpConfig->Get(*k, a->status.directSourceOn);

    k->value = DMP_CONFIG_VOL_MASTERVOL;
    dmpConfig->Get(*k, a->status.masterVolume);

    k->value = DMP_CONFIG_DSEEAI_ONOFF;
    dmpConfig->Get(*k, a->status.dseeHXOn);

    k->value = DMP_CONFIG_VINYL_ONOFF;
    dmpConfig->Get(*k, a->status.vinylOn);

    k->value = DMP_CONFIG_VINYL_TYPE;
    dmpConfig->Get(*k, a->status.vinylType);

    a->status.clearAudioAvailable = dmpFeature->IsFeaturedClearAudioPlus();
    a->status.directSourceAvailable = dmpFeature->IsFeaturedSourceDirect();

    if (tunerClient == nullptr) {
        auto f = pst::services::TunerPlayerServiceClientFactory();
        tunerClient = f.CreateInstance();
    }

    uint freq = 0;
    tunerClient->GetFrequency(freq);
    a->fmStatus.freq = (int)freq;
    a->fmStatus.state = tunerClient->GetTunerState();
}

void SetFmStereoMode(int value) {
    auto stereoMode = pst::services::ITunerPlayerService::StereoMode();
    tunerClient->GetStereoMode(stereoMode);
    printf("stereo is %d, setting to %d\n", stereoMode.value, value);
    stereoMode.value = value;
    tunerClient->SetStereoMode(stereoMode);
    shmp->fmStatus.stereo = value;
}

void SetAudioAnalyzer(int value) {
    if (analyzer == nullptr) {
        printf("attempt to SetAudioAnalyzer while analyzer is nullptr\n");
        return;
    }
    if (value == 1) {
        analyzer->Start(&analyzerEventListener);
    } else {
        analyzer->Stop();
    }
}

void SetAudioAnalyzerBands(const int values[50], int count) {
    if (analyzer == nullptr) {
        printf("attempt to SetAudioAnalyzerBands while analyzer is nullptr\n");
        return;
    }

    if (count > 12) {
        printf("got %d band values, set to 12\n", count);
        count = 12;
    }

    auto bands = std::vector<pst::services::audioanalyzerservice::Passband>{};
    for (int i = 0; i < count * 2; i = i + 2) {
        auto v = pst::services::audioanalyzerservice::Passband{values[i], float(values[i + 1])};
        printf("new band: %d\t%.1f\n", v.value, v.mean);
        bands.push_back(v);
    }
    analyzer->SetPassband(bands);
}

void SetFM(int value) {
    if (tunerClient == nullptr) {
        auto f = pst::services::TunerPlayerServiceClientFactory();
        tunerClient = f.CreateInstance();
    }

    auto res = tunerClient->GetTunerState();
    printf("tuner status %d, action %d\n", res, value);

    if (value == 1) {
        tunerClient->Open();
        tunerClient->Play();

        uint tunerMin, tunerMax, tunerStep = 0;
        tunerClient->GetBandwidth(tunerMin, tunerMax, tunerStep);
        printf("Bandwidth: %d %d %d\n", tunerMin, tunerMax, tunerStep);

        tunerClient->GetSoftBandwidth(tunerMin, tunerMax, tunerStep);
        printf("Soft bandwidth: %d %d %d\n", tunerMin, tunerMax, tunerStep);

        tunerClient->SetSoftBandwidth(FM_FREQ_MIN, FM_FREQ_MAX, tunerStep);
        auto stereoMode = pst::services::ITunerPlayerService::StereoMode();
        tunerClient->GetStereoMode(stereoMode);
        shmp->fmStatus.stereo = (bool)stereoMode.value;
    } else {
        tunerClient->Stop();
        tunerClient->Close();
    }

    shmp->fmStatus.state = tunerClient->GetTunerState();
    printf("tuner status: %d\n", res);
    uint freq = 0;
    tunerClient->GetFrequency(freq);
    shmp->fmStatus.freq = (int)freq;
}

void SetFmFreq(int value) {
    if (tunerClient == nullptr) {
        printf("client is nullpo\n");
        return;
    }

    printf("tuner freq to %d\n", value);
    tunerClient->SetFrequency(value);
}

void pst::services::audioanalyzerservice::EventListener::OnSpectrumUpdate(std::vector<int> *values) {
    for (int i = 0; i < values->size(); i++) {
        shmp->peaks[i] = values->at(i);
        //        printf("%d ", shmp->peaks[i]);
    }
    //    printf("\n");
};

int main(int argc, char **argv) {
    if (argc > 1) {
        auto delay = strtol(argv[1], nullptr, 0);
        printf("sleeping for %ld\n", delay);
        sleep(delay);
    }

    int fd;

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

    printf("starting audio analyzer\n");
    analyzer = pst::services::audioanalyzerservice::AudioAnalyzerService::GetInstance();
    analyzerEventListener = pst::services::audioanalyzerservice::EventListener();
    pst::services::audioanalyzerservice::mode_t m{};
    m.value = 1;
    analyzer->SetMode(m);
    analyzer->Start(&analyzerEventListener);
    printf("started audio analyzer\n");

    while (true) {
        if (sem_wait(&shmp->sem1) == -1)
            errExit("sem_wait");

        printf("request %d\n", shmp->command.id);
        switch (shmp->command.id) {
        case PSC_UPDATE:
            Update(dmpconfig, &k, &dmpFeature, shmp);
            break;
        case PSC_SET_FM:
            SetFM(shmp->command.valueInt);
            break;
        case PSC_SET_FM_FREQ:
            SetFmFreq(shmp->command.valueInt);
            break;
        case PSC_SET_FM_STEREO:
            SetFmStereoMode(shmp->command.valueInt);
            break;
        case PSC_SET_AUDIO_ANALYZER:
            SetAudioAnalyzer(shmp->command.valueInt);
            break;
        case PSC_SET_AUDIO_ANALYZER_BANDS:
            SetAudioAnalyzerBands(shmp->command.valuesInt, shmp->command.valueInt);
            break;
        default:
        case PSC_UNKNOWN:
            printf("unknown command\n");
            break;
        }
        shmp->command.id = PSC_UNKNOWN;

        if (sem_post(&shmp->sem2) == -1)
            errExit("sem_post");
    }

    /* Unlink the shared memory object. Even if the peer process
       is still using the object, this is okay. The object will
       be removed only after all open references are closed. */

    shm_unlink(SHMPATH);
}
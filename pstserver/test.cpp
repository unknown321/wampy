#include "dmpclient.h"
#include "dmpconfig.h"
#include "string"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>

void healthcheck() {
    // do nothing
}

std::string a;

int main(int argc, char **argv) {
    printf("start\n");
    if (strcmp(argv[1], "1") == 0) {
        printf("ok\n");
    }

    a = argv[1];
    printf("a is %s\n", a.c_str());

    auto pump = []() {
        auto framework = pst::core::Framework::GetReference();
        framework->StartForApplication(healthcheck, true);
        while (true) {
            framework->Pump(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        framework->Stop();
    };

    std::thread t(pump);
    t.detach();

    sleep(1);

    auto effectCtrlDmp = pst::services::sound::EffectCtrlDmp();

    if (a == "1") {
        printf("UpdateEq6BandOnOff && SetEq10Band(true)\n");
        effectCtrlDmp.UpdateEq6BandOnOff();
        effectCtrlDmp.SetEq10Band(true);
    }

    if (a == "0") {
        printf("SetEq10Band(false)\n");
        effectCtrlDmp.SetEq10Band(false);
    }

    if (a == "2") {
        printf("SetEq10BandValue\n");
        auto band = pst::services::sound::Eq10Band();
        band.value = 1;
        effectCtrlDmp.SetEq10BandValue(band, 20);
        band.value = 2;
        effectCtrlDmp.SetEq10BandValue(band, 20);
        band.value = 3;
        effectCtrlDmp.SetEq10BandValue(band, 20);
        band.value = 4;
        effectCtrlDmp.SetEq10BandValue(band, 20);
        band.value = 5;
        effectCtrlDmp.SetEq10BandValue(band, -20);
        band.value = 6;
        effectCtrlDmp.SetEq10BandValue(band, -20);
    }

    if (a == "3") {
        printf("SetEq10BandValue\n");
        auto band = pst::services::sound::Eq10Band();
        band.value = 1;
        effectCtrlDmp.SetEq10BandValue(band, -20);
        band.value = 2;
        effectCtrlDmp.SetEq10BandValue(band, -20);
        band.value = 3;
        effectCtrlDmp.SetEq10BandValue(band, -20);
        band.value = 4;
        effectCtrlDmp.SetEq10BandValue(band, -20);
        band.value = 5;
        effectCtrlDmp.SetEq10BandValue(band, 20);
        band.value = 6;
        effectCtrlDmp.SetEq10BandValue(band, 20);
    }

    if (a == "4") {
        auto tv = pst::services::sound::ToneType();
        tv.value = 0;
        effectCtrlDmp.SetToneValue(tv, 10);
        auto res = effectCtrlDmp.GetToneValue(tv);
        printf("%d\n", res);
    }

    if (a == "5") {
        auto tv = pst::services::sound::ToneType();
        tv.value = 0;
        auto tvf = pst::services::sound::ToneCenterFreq();
        tvf.value = 1;

        effectCtrlDmp.SetToneCenterFreq(tv, tvf);
    }

    if (a == "6") {
        auto tv = pst::services::sound::DcPhaseFilterType();
        tv.value = 0;

        effectCtrlDmp.SetDcPhaseFilterType(tv);
    }

    printf("end\n");
}
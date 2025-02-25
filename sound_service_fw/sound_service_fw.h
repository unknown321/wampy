#ifndef SSFW_PRELOAD_H
#define SSFW_PRELOAD_H

#define DLOGG(fmt, ...) fprintf(stderr, "[wampySSFW] %s %s:%d " fmt, __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#include "unistd.h"
#include <cstring>
#include <map>
#include <semaphore.h>
#include <string>
#include <vector>

#define SSFW_SHMPATH "/sound_service_fw"

#define errExit(msg)                                                                                                                       \
    do {                                                                                                                                   \
        perror(msg);                                                                                                                       \
        exit(EXIT_FAILURE);                                                                                                                \
    } while (0)

void StartServer();

namespace pst {
    namespace services {
        namespace sound {
            class DynamicAllocPacketPool {
              public:
                int data;
                char data1[8];
                char data2[8];
                char data3[8];
                char data4[8];
                char data5[4];
                char data6[4];
                char data7[4];
            };
            class mobile {

              public:
                class PreFilters {
                  public:
                    int Create(const std::vector<std::string> &param_1);
                };
                class Filter {
                  public:
                    //                    char data[0x50];
                    char data[0x10];
                    bool is_proc;
                    char data2[0x39];
                };

                class Eq10band {
                    char data[0x10];
                    bool is_proc;
                    char data2[0x39];
                    int UpdateProcCond(bool, bool);
                };

                class Eq6band {
                    char data[0x10];
                    bool is_proc;
                    char data2[0x39];
                    int UpdateProcCond(bool, bool);
                };

                class EqTone {
                    char data[0x10];
                    bool is_proc;
                    char data2[0x39];
                    int UpdateProcCond(bool, bool);
                };

                class Vpt {
                    char data[0x10];
                    bool is_proc;
                    char data2[0x39];
                    int UpdateProcCond(bool, bool);
                };

                class DynamicNormalizer {
                    char data[0x10];
                    bool is_proc;
                    char data2[0x39];
                    int UpdateProcCond(bool, bool);
                };

                class DcPhaseLinearizer {
                    char data[0x10];
                    bool is_proc;
                    char data2[0x39];
                    int UpdateProcCond(bool, bool);
                };

                class ClearPhase {
                    char data[0x10];
                    bool is_proc;
                    char data2[0x39];
                    int UpdateProcCond(bool, bool);
                };

                class FilterChain {
                  public:
                    char data[0x54];
                    std::map<std::string, Filter *> filters;
                    std::vector<std::string> filterOrder;
                    int Create(pst::services::sound::DynamicAllocPacketPool const *, std::vector<std::string> const &);
                    int ExecEffectParam(const std::string &, bool const &);
                    int SetEnable(std::string const &, bool const &);
                    bool CheckNeeded(std::string const &);
                    void EnqueueEffectParam(std::string const &);
                    int GetParam(const std::string &, std::string &);
                };
            }; // namespace mobile
        }      // namespace sound
    }          // namespace services
} // namespace pst

enum ESoundServiceFwCommand {
    SSFW_UNKNOWN = 0,
    SSFW_PRINT = 1,
    SSFW_SET_FILTER = 2,
    SSFW_UPDATE = 3,
    SSFW_SET_PARAM = 4,
    SSFW_GET_PARAM = 5
};

struct SoundServiceFwCommand {
    ESoundServiceFwCommand id = SSFW_UNKNOWN;
    int valueInt = 0;
    bool valueBool = false;
    char valueChar[100];
    char resultChar[100];

    void Reset() {
        id = SSFW_UNKNOWN;
        valueInt = 0;
        valueBool = false;
        memset(valueChar, 0, sizeof(valueChar));
    }
};

extern std::map<std::string, bool> procDesired;

struct filterStatus {
    char name[50]{};
    bool is_proc{};
};

struct sound_settings_fw {
    sem_t sem1{}; /* POSIX unnamed semaphore */
    sem_t sem2{}; /* POSIX unnamed semaphore */

    SoundServiceFwCommand command{};

    filterStatus FilterStatus[15];

    bool clearAudioPlusPresent{}; // backend returns "" on GetParam
    bool clearAudioPlusOn{};
    bool directSourcePresent{};
    bool directSourceOn{};
    bool anyFilterEnabled = false;

    pst::services::sound::mobile::FilterChain *chain = nullptr; // pointer to filter chain. Do stupid things, win stupid prizes.

    void Update() {
        for (auto &FilterStatu : FilterStatus) {
            memset(FilterStatu.name, 0, sizeof FilterStatu.name);
            FilterStatu.is_proc = false;
        }

        anyFilterEnabled = false;
        for (int i = 0; i < chain->filterOrder.size(); i++) {
            auto v = chain->filters.find(chain->filterOrder.at(i));
            if (v == chain->filters.end()) {
                DLOGG("invalid filter\n");
                continue;
            }
            memcpy(FilterStatus[i].name, v->first.c_str(), v->first.size());
            FilterStatus[i].is_proc = v->second->is_proc;
            anyFilterEnabled |= v->second->is_proc;
        }
        std::string res;
        chain->GetParam("clearaudioplus", res);
        clearAudioPlusPresent = !res.empty();
        if (clearAudioPlusPresent) {
            clearAudioPlusOn = res == "on";
        }
        anyFilterEnabled |= clearAudioPlusOn;

        chain->GetParam("sourcedirect", res);
        directSourcePresent = !res.empty();
        if (directSourcePresent) {
            directSourceOn = res == "on";
        }
        anyFilterEnabled |= directSourceOn;
    }

    void PrintFilters() const {
        for (const auto &i : chain->filterOrder) {
            auto v = chain->filters.find(i);
            if (v == chain->filters.end()) {
                DLOGG("invalid filter\n");
                continue;
            }

            DLOGG("filter %s, enabled %d\n", v->first.c_str(), v->second->is_proc);
        }
    }

    void SetFilter(const std::string &name, bool proc) const {
        DLOGG("setting filter %s to %d\n", name.c_str(), proc);

        auto f = chain->filters.find(name);
        if (f == chain->filters.end()) {
            DLOGG("filter %s not found\n", name.c_str());
            return;
        }

        procDesired[name.c_str()] = proc;

        char format[50] = "%s=%s";
        char out[50]{};
        sprintf(out, format, name.c_str(), proc ? "on" : "off");
        DLOGG("enqueue %s\n", out);
        chain->EnqueueEffectParam(out);
    }

    void EnqueueParam(const char *param) const {
        DLOGG("enqueue %s\n", param);
        chain->EnqueueEffectParam(param);
    }

    void GetParam(const char *param, std::string *out) const {
        chain->GetParam(param, *out);
        //        DLOGG("param %s, res is %s\n", param, out->c_str());
    }
};

#endif // SSFW_PRELOAD_H

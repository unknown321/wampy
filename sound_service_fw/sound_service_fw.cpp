#include "sound_service_fw.h"
#include "algorithm"
#include <cassert>
#include <dlfcn.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>
#include <vector>

std::map<std::string, bool> procDesired = {};

using CreateFuncType = void (pst::services::sound::mobile::PreFilters::*)(const std::vector<std::string> &);
CreateFuncType original_Create = nullptr;

using CreateFilterFuncType = int (pst::services::sound::mobile::*)(const std::string &);
CreateFilterFuncType original_CreateFilter = nullptr;

using CreateFilterChainType =
    void (pst::services::sound::mobile::FilterChain::*)(pst::services::sound::DynamicAllocPacketPool *, const std::vector<std::string> &);
CreateFilterChainType original_FilterChainCreate = nullptr;

using FilterExecEffectParam = void (pst::services::sound::mobile::FilterChain::*)(const std::string &, const bool &);
FilterExecEffectParam original_FilterExecEffectParam = nullptr;

using FilterChainSetEnable = void (pst::services::sound::mobile::FilterChain::*)(const std::string &, const bool &);
FilterChainSetEnable original_FilterChainSetEnable = nullptr;

using FilterChainCheckNeeded = bool (pst::services::sound::mobile::FilterChain::*)(const std::string &);
FilterChainCheckNeeded original_FilterChainCheckNeeded = nullptr;

using FilterChainGetParam = bool (pst::services::sound::mobile::FilterChain::*)(const std::string &, std::string &);
FilterChainGetParam original_FilterChainGetParam = nullptr;

using Eq10bandUpdateProcCond = bool (pst::services::sound::mobile::Eq10band::*)(bool, bool);
Eq10bandUpdateProcCond original_eq10BandUpdateProcCond = nullptr;

using Eq6bandUpdateProcCond = bool (pst::services::sound::mobile::Eq6band::*)(bool, bool);
Eq6bandUpdateProcCond original_eq6BandUpdateProcCond = nullptr;

using EqToneUpdateProcCond = bool (pst::services::sound::mobile::EqTone::*)(bool, bool);
EqToneUpdateProcCond original_eqToneUpdateProcCond = nullptr;

using VptUpdateProcCond = bool (pst::services::sound::mobile::Vpt::*)(bool, bool);
VptUpdateProcCond original_vptUpdateProcCond = nullptr;

using DynamicNormalizerUpdateProcCond = bool (pst::services::sound::mobile::DynamicNormalizer::*)(bool, bool);
DynamicNormalizerUpdateProcCond original_DynamicNormalizerUpdateProcCond = nullptr;

using DcPhaseLinearizerUpdateProcCond = bool (pst::services::sound::mobile::DcPhaseLinearizer::*)(bool, bool);
DcPhaseLinearizerUpdateProcCond original_DcPhaseLinearizerUpdateProcCond = nullptr;

using ClearPhaseUpdateProcCond = bool (pst::services::sound::mobile::ClearPhase::*)(bool, bool);
ClearPhaseUpdateProcCond original_ClearPhaseUpdateProcCond = nullptr;

pst::services::sound::mobile::FilterChain *filterChain = nullptr;

std::vector<std::string> allFilters = {
    "alc",
    "attn",
    "clearphase",
    "dcphaselinear",
    "dseeai",
    "dseehxcustom",
    "dseehxlegacy",
    "dynamicnormalizer",
    "eq10band",
    "eq6band",
    "eqtone",
    "heq",
    "vinylizer",
    "vpt",
};

std::vector<std::string> a50Filters = {
    "i2f",
    "dseeai",
    "heq",
    "dynamicnormalizer",
    "attn",
    "eq6band",
    "vpt",
    "clearphase",
    "alc",
    "dcphaselinear",
    "vinylizer",
    "f2i",
};

std::vector<std::string> a50FullFilters = {
    "i2f",
    "dseeai",
    "heq",
    "dynamicnormalizer",
    "attn",
    "eq6band",
    "eq10band",
    "eqtone",
    "vpt",
    "clearphase",
    "alc",
    "dcphaselinear",
    "vinylizer",
    "f2i",
};

std::vector<std::string> WM1ZFilters = {
    "i2f",
    "dseehxcustom",
    "heq",
    "dynamicnormalizer",
    "attn",
    "eq10band",
    "eqtone",
    "alc",
    "dcphaselinear",
    "vinylizer",
    "f2i",
};

std::vector<std::string> WM1ZFullFilters = {
    "i2f",
    "dseehxcustom",
    "heq",
    "dynamicnormalizer",
    "attn",
    "eq6band",
    "eq10band",
    "eqtone",
    "vpt",
    "clearphase",
    "alc",
    "dcphaselinear",
    "vinylizer",
    "f2i",
};

void pst::services::sound::mobile::PreFilters::Create(const std::vector<std::string> &param_1) {
    if (!original_Create) {
        union h {
            void *void_ptr;
            CreateFuncType member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(
            RTLD_NEXT,
            "_ZN3pst8services5sound6mobile10PreFilters6CreateERKNSt3__16vectorINS4_12basic_stringIcNS4_11char_traitsIcEENS4_"
            "9allocatorIcEEEENS9_ISB_EEEE"
        );
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_Create = helper.member_func_ptr;
    }

    for (const auto &v : param_1) {
        DLOGG("prefilter ->> %s\n", v.c_str());
    }

    (this->*original_Create)(param_1);
}

void pst::services::sound::mobile::FilterChain::Create(
    const pst::services::sound::DynamicAllocPacketPool *param_1, const std::vector<std::string> &param_2
) {
    DLOGG("start\n");
    if (filterChain == nullptr) {
        filterChain = this;

        auto f = []() { StartServer(); };
        std::thread t(f);
        t.detach();
    }

    if (!original_FilterChainCreate) {
        union h {
            void *void_ptr;
            CreateFilterChainType member_func_ptr;
        };

        h helper{};
        helper.void_ptr = dlsym(
            RTLD_NEXT,
            "_ZN3pst8services5sound6mobile11FilterChain6CreateEPKNS1_22DynamicAllocPacketPoolERKNSt3__16vectorINS7_12basic_stringIcNS7_"
            "11char_traitsIcEENS7_9allocatorIcEEEENSC_ISE_EEEE"
        );
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_FilterChainCreate = helper.member_func_ptr;
    }

    auto pp = param_2;

    if (pp.empty()) {
        DLOGG("empty, ignoring\n");
    } else {
        if (std::find(pp.begin(), pp.end(), "dseeai") != pp.end()) {
            DLOGG("using A50 filters\n");
            pp = a50FullFilters;
        } else if (std::find(pp.begin(), pp.end(), "dseehxcustom") != pp.end()) {
            DLOGG("using WMZ1 filters\n");
            pp = WM1ZFullFilters;
        } else {
            DLOGG("filter chain not recognized, using default\n");
        }
    }

    for (const auto &v : pp) {
        DLOGG("create filter: ->> %s\n", v.c_str());
    }

    DLOGG("calling original\n");

    (this->*original_FilterChainCreate)(const_cast<DynamicAllocPacketPool *>(param_1), pp);

    DLOGG("end\n");
}

void pst::services::sound::mobile::FilterChain::ExecEffectParam(const std::string &s, const bool &b) {
    if (!original_FilterExecEffectParam) {
        union h {
            void *void_ptr;
            FilterExecEffectParam member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(
            RTLD_NEXT,
            "_ZN3pst8services5sound6mobile11FilterChain15ExecEffectParamERKNSt3__112basic_stringIcNS4_11char_traitsIcEENS4_"
            "9allocatorIcEEEERKb"
        );
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_FilterExecEffectParam = helper.member_func_ptr;
    }

    DLOGG("effect param %s\n", s.c_str());

    (this->*original_FilterExecEffectParam)(s, b);
}

void pst::services::sound::mobile::FilterChain::SetEnable(const std::string &s, const bool &b) {
    if (!original_FilterChainSetEnable) {
        union h {
            void *void_ptr;
            FilterChainSetEnable member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(
            RTLD_NEXT,
            "_ZN3pst8services5sound6mobile11FilterChain9SetEnableERKNSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEERKb"
        );
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_FilterChainSetEnable = helper.member_func_ptr;
    }

    //    DLOGG("set enable %s %d\n", s.c_str(), b);

    (this->*original_FilterChainSetEnable)(s, b);
}

bool pst::services::sound::mobile::FilterChain::CheckNeeded(const std::string &s) {
    if (!original_FilterChainCheckNeeded) {
        union h {
            void *void_ptr;
            FilterChainCheckNeeded member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(
            RTLD_NEXT,
            "_ZN3pst8services5sound6mobile11FilterChain11CheckNeededERKNSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEE"
        );
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return false;
        }
        original_FilterChainCheckNeeded = helper.member_func_ptr;
    }

    //    DLOGG("check needed %s\n", s.c_str());

    return (this->*original_FilterChainCheckNeeded)(s);
}

void pst::services::sound::mobile::FilterChain::GetParam(const std::string &s1, std::string &s2) {
    if (!original_FilterChainGetParam) {
        union h {
            void *void_ptr;
            FilterChainGetParam member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(
            RTLD_NEXT,
            "_ZN3pst8services5sound6mobile11FilterChain8GetParamERKNSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEERSA_"
        );
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_FilterChainGetParam = helper.member_func_ptr;
    }

    (this->*original_FilterChainGetParam)(s1, s2);

    DLOGG("Get param %s, result %s\n", s1.c_str(), s2.c_str());
}

void pst::services::sound::mobile::Eq10band::UpdateProcCond(bool b1, bool b2) {
    if (!original_eq10BandUpdateProcCond) {
        union h {
            void *void_ptr;
            Eq10bandUpdateProcCond member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(RTLD_NEXT, "_ZN3pst8services5sound6mobile8Eq10band14UpdateProcCondEbb");
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_eq10BandUpdateProcCond = helper.member_func_ptr;
    }

    (this->*original_eq10BandUpdateProcCond)(b1, b2);
    DLOGG("isproc is %d\n", this->is_proc);
    auto p = procDesired.find("eq10band");
    if (p == procDesired.end()) {
        DLOGG("no desired value, skip\n");
        return;
    }
    this->is_proc = p->second;
    DLOGG("isproc set to %d\n", this->is_proc);
}

void pst::services::sound::mobile::Eq6band::UpdateProcCond(bool b1, bool b2) {
    if (!original_eq6BandUpdateProcCond) {
        union h {
            void *void_ptr;
            Eq6bandUpdateProcCond member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(RTLD_NEXT, "_ZN3pst8services5sound6mobile7Eq6band14UpdateProcCondEbb");
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_eq6BandUpdateProcCond = helper.member_func_ptr;
    }

    (this->*original_eq6BandUpdateProcCond)(b1, b2);
    DLOGG("isproc is %d\n", this->is_proc);
    auto p = procDesired.find("eq6band");
    if (p == procDesired.end()) {
        DLOGG("no desired value, skip\n");
        return;
    }
    this->is_proc = p->second;
    DLOGG("isproc set to %d\n", this->is_proc);
}

void pst::services::sound::mobile::EqTone::UpdateProcCond(bool b1, bool b2) {
    if (!original_eqToneUpdateProcCond) {
        union h {
            void *void_ptr;
            EqToneUpdateProcCond member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(RTLD_NEXT, "_ZN3pst8services5sound6mobile6EqTone14UpdateProcCondEbb");
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_eqToneUpdateProcCond = helper.member_func_ptr;
    }

    (this->*original_eqToneUpdateProcCond)(b1, b2);
    DLOGG("isproc is %d\n", this->is_proc);
    auto p = procDesired.find("eqtone");
    if (p == procDesired.end()) {
        DLOGG("no desired value, skip\n");
        return;
    }
    this->is_proc = p->second;
    DLOGG("isproc set to %d\n", this->is_proc);
}

void pst::services::sound::mobile::Vpt::UpdateProcCond(bool b1, bool b2) {
    if (!original_vptUpdateProcCond) {
        union h {
            void *void_ptr;
            VptUpdateProcCond member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(RTLD_NEXT, "_ZN3pst8services5sound6mobile3Vpt14UpdateProcCondEbb");
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_vptUpdateProcCond = helper.member_func_ptr;
    }

    (this->*original_vptUpdateProcCond)(b1, b2);
    DLOGG("isproc is %d\n", this->is_proc);
    auto p = procDesired.find("vpt");
    if (p == procDesired.end()) {
        DLOGG("no desired value, skip\n");
        return;
    }
    this->is_proc = p->second;
    DLOGG("isproc set to %d\n", this->is_proc);
}

void pst::services::sound::mobile::DynamicNormalizer::UpdateProcCond(bool b1, bool b2) {
    if (!original_DynamicNormalizerUpdateProcCond) {
        union h {
            void *void_ptr;
            DynamicNormalizerUpdateProcCond member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(RTLD_NEXT, "_ZN3pst8services5sound6mobile17DynamicNormalizer14UpdateProcCondEbb");
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_DynamicNormalizerUpdateProcCond = helper.member_func_ptr;
    }

    (this->*original_DynamicNormalizerUpdateProcCond)(b1, b2);
    DLOGG("isproc is %d\n", this->is_proc);
    auto p = procDesired.find("dynamicnormalizer");
    if (p == procDesired.end()) {
        DLOGG("no desired value, skip\n");
        return;
    }
    this->is_proc = p->second;
    DLOGG("isproc set to %d\n", this->is_proc);
}

void pst::services::sound::mobile::DcPhaseLinearizer::UpdateProcCond(bool b1, bool b2) {
    if (!original_DcPhaseLinearizerUpdateProcCond) {
        union h {
            void *void_ptr;
            DcPhaseLinearizerUpdateProcCond member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(RTLD_NEXT, "_ZN3pst8services5sound6mobile17DcPhaseLinearizer14UpdateProcCondEbb");
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_DcPhaseLinearizerUpdateProcCond = helper.member_func_ptr;
    }

    (this->*original_DcPhaseLinearizerUpdateProcCond)(b1, b2);
    DLOGG("isproc is %d\n", this->is_proc);
    auto p = procDesired.find("dcphaselinear");
    if (p == procDesired.end()) {
        DLOGG("no desired value, skip\n");
        return;
    }
    this->is_proc = p->second;
    DLOGG("isproc set to %d\n", this->is_proc);
}

void pst::services::sound::mobile::ClearPhase::UpdateProcCond(bool b1, bool b2) {
    if (!original_ClearPhaseUpdateProcCond) {
        union h {
            void *void_ptr;
            ClearPhaseUpdateProcCond member_func_ptr;
        };
        h helper{};
        helper.void_ptr = dlsym(RTLD_NEXT, "_ZN3pst8services5sound6mobile10ClearPhase14UpdateProcCondEbb");
        if (!helper.void_ptr) {
            DLOGG("dlsym fail\n");
            return;
        }
        original_ClearPhaseUpdateProcCond = helper.member_func_ptr;
    }

    (this->*original_ClearPhaseUpdateProcCond)(b1, b2);
    DLOGG("isproc is %d\n", this->is_proc);
    auto p = procDesired.find("clearphase");
    if (p == procDesired.end()) {
        DLOGG("no desired value, skip\n");
        return;
    }
    this->is_proc = p->second;
    DLOGG("isproc set to %d\n", this->is_proc);
}

void StartServer() {
    DLOGG("\n");
    assert(filterChain);

    shm_unlink(SSFW_SHMPATH);

    int fd = shm_open(SSFW_SHMPATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1)
        errExit("shm_open");

    if (ftruncate(fd, sizeof(struct sound_settings_fw)) == -1)
        errExit("ftruncate");

    auto shmp = static_cast<sound_settings_fw *>(mmap(nullptr, sizeof(sound_settings_fw), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (shmp == MAP_FAILED)
        errExit("mmap");

    /* Initialize semaphores as process-shared, with value 0. */

    if (sem_init(&shmp->sem1, 1, 0) == -1)
        errExit("sem_init-sem1");
    if (sem_init(&shmp->sem2, 1, 0) == -1)
        errExit("sem_init-sem2");

    shmp->chain = filterChain;

    while (true) {
        if (sem_wait(&shmp->sem1) == -1)
            errExit("sem_wait");

        DLOGG("request, command %d\n", shmp->command.id);
        switch (shmp->command.id) {
        case SSFW_UNKNOWN:
            DLOGG("unknown command\n");
            break;
        case SSFW_PRINT:
            shmp->PrintFilters();
            break;
        case SSFW_SET_FILTER:
            shmp->SetFilter(shmp->command.valueChar, shmp->command.valueBool);
            break;
        case SSFW_UPDATE:
            shmp->Update();
            break;
        case SSFW_SET_PARAM:
            shmp->EnqueueParam(shmp->command.valueChar);
            break;
        case SSFW_GET_PARAM:
            std::string out;
            shmp->GetParam(shmp->command.valueChar, &out);
            memset(shmp->command.resultChar, 0, sizeof shmp->command.resultChar);
            out.copy(shmp->command.resultChar, sizeof shmp->command.resultChar, 0);
            break;
        }
        shmp->command.Reset();

        if (sem_post(&shmp->sem2) == -1)
            errExit("sem_post");
    }

    shm_unlink(SSFW_SHMPATH);
}

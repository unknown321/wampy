#include "wampy.h"
#include "helpers.h"

#if SECOND_PASS == 1

#include "/Version.h"
#include <future>

// clang-format off
#include "server.moc"
// clang-format on

void Start() {
    DLOG("ver %s\n", SOFTWARE_VERSION);

    auto exec = []() {
        auto server = new WampyServer();
        server->Serve();
    };

    std::thread t(exec);
    t.detach();
}

#else

void Start() {}

#endif
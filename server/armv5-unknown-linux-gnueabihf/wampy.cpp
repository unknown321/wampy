#include "wampy.h"
#include "helpers.h"

#if SECOND_PASS == 1

#include <future>

// clang-format off
#include "server.moc"
// clang-format on

void Start() {
    DLOG("enter\n");

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
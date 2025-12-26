#ifndef WAMPY_DIGICLOCK_CONFIG_H
#define WAMPY_DIGICLOCK_CONFIG_H

#include "string"

namespace DigitalClock {
    struct Config {
        std::string color{};
        std::string language{};
        void Default() { color = "silver"; }
    };
} // namespace DigitalClock
#endif // WAMPY_CONFIG_H

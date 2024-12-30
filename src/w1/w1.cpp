#include "w1.h"

namespace W1 {
    std::map<std::string, uint> colorByName = {{"Default", 0}, {"Peach", 3}, {"Red", 5}, {"Blue", 7}, {"Green", 9}};
    std::map<uint, std::string> colorByValue = {{0, "Default"}, {3, "Peach"}, {5, "Red"}, {7, "Blue"}, {9, "Green"}};
    uint defaultColor = 0;

    void SetColor(uint color) {
        char command[23];
        if (colorByValue.find(color) == colorByValue.end()) {
            DLOG("unknown color 0x%.8x\n", color);
            return;
        }

        snprintf(command, 23, "nvpflag clv 0x%.8x", color);
        DLOG("command %s\n", command);
        auto code = system(command);
        if (code != 0) {
            DLOG("failure, code %d, setting color to default\n", code);
            system("nvpflag clv 0x00000000");
            return;
        }

        DLOG("new color is %s (0x%.8x)\n", colorByValue.at(color).c_str(), color);
    }
} // namespace W1
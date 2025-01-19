#include "hagoromoToString.h"
#include <map>
#include <string>

std::map<int, std::string> eq6PresetToString = {
    {0, "Off"},
    {5, "Bright"},
    {7, "Excited"},
    {8, "Mellow"},
    {6, "Relaxed"},
    {2, "Vocal"},
    {9, "Custom 1"},
    {10, "Custom 2"},
};

std::map<int, std::string> vptA50SmallToString = {
    {0, "?"},
    {1, "Studio"},
    {2, "Club"},
    {3, "Concert hall"},
    {4, "Matrix"},
};

std::map<int, std::string> dcFilterToString = {
    {0, "?"},
    {1, "A Low"},
    {2, "A Standard"},
    {3, "A High"},
    {4, "B Low"},
    {5, "B Standard"},
    {6, "B High"},
};

std::map<int, std::string> vinylTypeToString = {
    {0, "?"},
    {1, "Standard"},
    {2, "Arm resonance"},
    {3, "Turntable resonance"},
    {4, "Surface noise"},
    {5, "?"},
    {6, "?"},
    {7, "NW-A50 default mode?"},
};

std::map<int, std::string> dseeModeToString = {
    {0, "?"},
    {1, "AI"},
    {2, "Female vocal"},
    {3, "Male vocal"},
    {4, "Percussion"},
    {5, "Strings"},
};

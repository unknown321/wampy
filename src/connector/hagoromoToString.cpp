#include "hagoromoToString.h"

#include <libintl.h>
#include <map>
#include <string>

std::map<int, std::string> eq6PresetToString = {
    {0, gettext("Off")},
    {5, gettext("Bright")},
    {7, gettext("Excited")},
    {8, gettext("Mellow")},
    {6, gettext("Relaxed")},
    {2, gettext("Vocal")},
    {9, gettext("Custom 1")},
    {10, gettext("Custom 2")},
};

std::map<int, std::string> vptA50SmallToString = {
    {0, "?"},
    {1, gettext("Studio")},
    {2, gettext("Club")},
    {3, gettext("Concert hall")},
    {4, gettext("Matrix")},
};

std::map<int, std::string> dcFilterToString = {
    {0, "?"},
    {1, gettext("A Low")},
    {2, gettext("A Standard")},
    {3, gettext("A High")},
    {4, gettext("B Low")},
    {5, gettext("B Standard")},
    {6, gettext("B High")},
};

std::map<int, std::string> vinylTypeToString = {
    {0, "?"},
    {1, gettext("Standard")},
    {2, gettext("Arm resonance")},
    {3, gettext("Turntable resonance")},
    {4, gettext("Surface noise")},
    {5, "?"},
    {6, "?"},
    {7, gettext("NW-A50 default mode?")},
};

std::map<int, std::string> dseeModeToString = {
    {0, "?"},
    {1, gettext("AI")},
    {2, gettext("Female vocal")},
    {3, gettext("Male vocal")},
    {4, gettext("Percussion")},
    {5, gettext("Strings")},
};

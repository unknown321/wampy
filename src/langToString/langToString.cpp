#include "langToString.h"

#include <map>
#include <string>

std::map<std::string, std::string> LangToString = {
    {"en", "English"},
    {"ru", "Русский"},
    {"ja", "日本語"},
    {"zh_CN", "简体中文"},
    {"vi", "Tiếng Việt"},
    {"es", "Español"},
    {"fr", "Français"},
    {"de", "Deutsch"},
    {"ko", "한국어"},
    {"pt_BR", "Português (Brasil)"},
    {"it", "Italiano"},
    {"pl", "Polski"},
};

#ifdef DESKTOP
std::string localeDir = "../tl/locale";
#else
std::string localeDir = "/system/vendor/unknown321/usr/share/wampy/locale";
#endif

#include "skinVariant.h"

#include "Magick++.h"

bool SkinExists(const std::string &name, SkinList *l, std::string *filepath) {
    for (const auto &s : *l) {
        if (s.name == name) {
            *filepath = s.fullPath;
            return true;
        }
    }

    return false;
}
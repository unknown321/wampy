#ifndef WAMPY_SKINVARIANT_H
#define WAMPY_SKINVARIANT_H

#include "connector/connector.h"
#include "imgui.h"
#include "util/util.h"

struct Fonts {
    ImFont *regular;
    ImFont *number;
    ImFont *bitmap;
};

typedef std::pair<char *, size_t> bitmapData;

typedef std::vector<directoryEntry> SkinList;

bool SkinExists(const std::string &name, SkinList *l, std::string *filepath);

class SkinVariant {
  public:
    SkinVariant() = default;

    Connector *connector{};
    bool *render{};
    void *skin{};
    std::string loadStatusStr{};
    bool debug = false;

    bool loading = false;

    virtual void Draw() = 0;

    virtual int Load(std::string filename, ImFont **FontRegular) = 0;
};

#endif // WAMPY_SKINVARIANT_H

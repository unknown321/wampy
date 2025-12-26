#include "langToString.h"

#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <libintl.h>

int main(int argc, char *argv[]) {
    for (const auto &t : LangToString) {
#ifdef DESKTOP
        setenv("LANGUAGE", t.first.c_str(), 1);
        setlocale(LC_ALL, "");
        bindtextdomain("wampy", "../tl/locale");
        textdomain("wampy");
#else
        setenv("LC_ALL", t.first.c_str(), 1);
        setenv("LC_MESSAGES", "", 1);
        setlocale(LC_MESSAGES, "");
        textdomain("wampy");
        bind_textdomain_codeset("wampy", "UTF-8");
        setenv("LANGUAGE", t.first.c_str(), 1);
        bindtextdomain("wampy", "/system/vendor/unknown321/usr/share/wampy/locale");
#endif
        printf("%s %s\n", t.first.c_str(), gettext("Bright"));
    }
}
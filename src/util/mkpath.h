#ifndef WAMPY_MKPATH_H
#define WAMPY_MKPATH_H
#include <sys/stat.h>

int mkpath(const char *path, mode_t mode);

static int do_mkdir(const char *path, mode_t mode);

#endif // WAMPY_MKPATH_H

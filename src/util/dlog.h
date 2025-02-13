
#ifndef WAMPY_DLOG_H
#define WAMPY_DLOG_H

#define DLOG(fmt, ...) fprintf(stderr, "[wampy] %s %s:%d " fmt, __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif // WAMPY_DLOG_H

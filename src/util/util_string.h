#ifndef WAMPY_UTIL_STRING_H
#define WAMPY_UTIL_STRING_H

#include "string"
#include "vector"

std::vector<std::string> split(const std::string &s, const std::string &delimiter);

std::string join(const std::vector<std::string> &v, int start);

std::string join(const std::vector<std::string> &v, int start, std::string sep);

#endif // WAMPY_UTIL_STRING_H

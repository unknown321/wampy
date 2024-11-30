#ifndef WSTRING_H
#define WSTRING_H

#include <iostream>
#include <string>
#include <vector>

static std::wstring stringShift(const std::wstring &s, std::vector<std::vector<int>> &shift, int len);

static std::wstring to_wide(const char *mbstr);

static std::string to_bytes(const wchar_t *wstr);

int utfLen(const char *c, size_t length);

int utfLen(const std::string &s);

void utfShift(const std::string &s, char *result);

void utfCut(const std::string &s, int maxLength, char *result);

#endif
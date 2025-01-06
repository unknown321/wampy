#include "wstring.h"
#include <cstring>

static std::wstring stringShift(const std::wstring &s, std::vector<std::vector<int>> &shift, int len) {
    int val = 0;
    for (auto &i : shift)
        // If shift[i][0] = 0, then left shift
        // Otherwise, right shift
        val += i[0] == 0 ? -i[1] : i[1];

    // Stores length of the string
    //            int len = (int) s.length();

    // Effective shift calculation
    val = val % len;

    // Stores modified string
    std::wstring result;

    // Right rotation
    if (val > 0)
        result = s.substr(len - val, val) + s.substr(0, len - val);

    // Left rotation
    else
        result = s.substr(-val, len + val) + s.substr(0, -val);

    return result;
}

static std::wstring to_wide(const char *mbstr) {
    std::mbstate_t state = std::mbstate_t();
    std::size_t len = 1 + std::mbsrtowcs(nullptr, &mbstr, 0, &state);
    if (len == 0) {
        return (reinterpret_cast<const wchar_t *>(""));
    }
    std::vector<wchar_t> wstr(len);
    std::mbsrtowcs(&wstr[0], &mbstr, wstr.size(), &state);
    //    std::wcout << "Wide string: " << &wstr[0] << '\n'
    //               << "The length, including '\\0': " << wstr.size() << '\n';
    auto s = std::wstring(wstr.data());
    return s;
}

static std::string to_bytes(const wchar_t *wstr) {
    std::mbstate_t state = std::mbstate_t();
    std::size_t len = 1 + std::wcsrtombs(nullptr, &wstr, 0, &state);
    std::vector<char> mbstr(len);
    std::wcsrtombs(&mbstr[0], &wstr, mbstr.size(), &state);
    //            std::cout << "multibyte string: " << &mbstr[0] << '\n'
    //                      << "Length, including '\\0': " << mbstr.size() << '\n';

    auto s = std::string(mbstr.data());
    return s;
}

int utfLen(const char *c, size_t length) {
    int len = 0;

    for (int i = 0; i < length; i++) {

        unsigned char lb = c[i];

        if ((lb & 0x80) == 0) // lead bit is zero, must be a single ascii
            len += 1;
        else if ((lb & 0xE0) == 0xC0) {
            len += 1;
            i++;
        } else if ((lb & 0xF0) == 0xE0) {
            len += 1;
            i = i + 2;
        } else if ((lb & 0xF8) == 0xF0) {
            len += 1;
            i = i + 3;
        } else
            printf("Unrecognized lead byte (%02x)\n", lb);
    }

    return len;
}

int utfLen(const std::string &s) { return utfLen(s.c_str(), s.length()); }

void utfShift(const char *s, size_t length, char *result) {
    int len = 0;
    unsigned char lb = s[0];

    if ((lb & 0x80) == 0) // lead bit is zero, must be a single ascii
        len = 1;
    else if ((lb & 0xE0) == 0xC0) // 110x xxxx
        len = 2;
    else if ((lb & 0xF0) == 0xE0) // 1110 xxxx
        len = 3;
    else if ((lb & 0xF8) == 0xF0) // 1111 0xxx
        len = 4;
    else
        printf("Unrecognized lead byte (%02x)\n", lb);

    strcpy(result, s + len);
    for (int i = 0; i < len; i++) {
        result[length - len + i] = s[i];
    }
    result[length] = 0;
}

void utfShift(const std::string &s, char *result) { utfShift(s.c_str(), s.length(), result); }

void utfCut(const char *s, size_t length, int maxLength, char *result) {
    int l = utfLen(s);
    if (l <= maxLength) {
        return;
    }

    int len = 0;
    const char *b = s;
    for (int i = 0; i < length; i++) {
        unsigned char lb = b[i];
        if (len == maxLength) {
            result[i] = 0;
            return;
        }

        if ((lb & 0x80) == 0) {
            len++;
            result[i] = lb;
        } else if ((lb & 0xE0) == 0xC0) {
            result[i] = lb;
            result[i + 1] = b[i + 1];
            i++;
            len++;
        } else if ((lb & 0xF0) == 0xE0) {
            result[i] = lb;
            result[i + 1] = b[i + 1];
            result[i + 2] = b[i + 2];
            i = i + 2;
            len++;
        } else if ((lb & 0xF8) == 0xF0) {
            result[i] = lb;
            result[i + 1] = b[i + 1];
            result[i + 2] = b[i + 2];
            result[i + 3] = b[i + 3];
            i = i + 3;
            len++;
        } else {
            printf("Unrecognized lead byte (%02x)\n", lb);
        }
    }
}

void utfCut(const std::string &s, int maxLength, char *result) { utfCut(s.c_str(), s.length(), maxLength, result); }

// TODO: take a look at https://github.com/skeeto/branchless-utf8

void utfCharLen(const char *c, int *len) {
    unsigned char lb = c[0];
    *len = 0;
    if ((lb & 0x80) == 0) // lead bit is zero, must be a single ascii
        *len = 1;
    else if ((lb & 0xE0) == 0xC0) // 110x xxxx
        *len = 2;
    else if ((lb & 0xF0) == 0xE0) // 1110 xxxx
        *len = 3;
    else if ((lb & 0xF8) == 0xF0) // 1111 0xxx
        *len = 4;
    else
        printf("Unrecognized lead byte (%02x)\n", lb);
}

// does len UTF8-characters fit into string s starting from byte start?
// start byte must be a valid utf8 character, otherwise doesn't fit
void utfFits(const char *s, int start, int len, bool *result, int *endWhenFits) {
    int end = start;
    int charLen = 0;
    auto fullLen = (int)strlen(s);
    for (int i = 0; i < len; i++) {
        auto c = s[end];
        // reached end of string = doesn't fit
        if (c == '\0') {
            *result = false;
            *endWhenFits = 0;
            return;
        }

        utfCharLen(&c, &charLen);
        if (charLen == 0) {
            *result = false;
            *endWhenFits = 0;
            return;
        }

        end += charLen;
        if (end > fullLen) {
            *endWhenFits = 0;
            *result = false;
            return;
        }
    }

    *result = true;
    *endWhenFits = end;
}

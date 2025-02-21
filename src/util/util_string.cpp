#include "util_string.h"

std::vector<std::string> split(const std::string &s, const std::string &delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    auto v = s.substr(pos_start);
    res.push_back(v);
    return res;
}

std::string join(const std::vector<std::string> &v, int start) {
    std::string res;
    for (int i = start; i < v.size(); i++) {
        res += v[i];
    }

    return res;
}

std::string join(const std::vector<std::string> &v, int start, std::string sep) {
    std::string res;
    for (int i = start; i < v.size(); i++) {
        res += v[i] + sep;
    }

    return res;
}

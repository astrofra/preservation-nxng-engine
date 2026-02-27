#include "IO/cqString.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>

int qString::qstrncmp(const char* a, const char* b, std::size_t n) const {
    if (!a || !b) {
        return (a == b) ? 0 : (a ? 1 : -1);
    }
    return std::strncmp(a, b, n);
}

int qString::qstrcmp(const char* a, const char* b) const {
    if (!a || !b) {
        return (a == b) ? 0 : (a ? 1 : -1);
    }
    return std::strcmp(a, b);
}

int qString::qstrsearch(const char* haystack, const char* needle) const {
    if (!haystack || !needle) {
        return -1;
    }
    const char* hit = std::strstr(haystack, needle);
    if (!hit) {
        return -1;
    }
    return static_cast<int>(hit - haystack);
}

void qString::qstrlwr(char* s) const {
    if (!s) {
        return;
    }
    for (char* p = s; *p; ++p) {
        *p = static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
    }
}

void qString::qstrrpl(char* s, char from, char to) const {
    if (!s) {
        return;
    }
    for (char* p = s; *p; ++p) {
        if (*p == from) {
            *p = to;
        }
    }
}

float qString::a2f(const char* s) const {
    if (!s) {
        return 0.0f;
    }
    return std::strtof(s, nullptr);
}

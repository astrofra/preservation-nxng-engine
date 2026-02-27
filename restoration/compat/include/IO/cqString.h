#pragma once

#include <cstddef>

class qString {
public:
    int qstrncmp(const char* a, const char* b, std::size_t n) const;
    int qstrcmp(const char* a, const char* b) const;
    int qstrsearch(const char* haystack, const char* needle) const;
    void qstrlwr(char* s) const;
    void qstrrpl(char* s, char from, char to) const;
    float a2f(const char* s) const;
};

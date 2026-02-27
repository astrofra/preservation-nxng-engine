#pragma once

class MCIO {
public:
    unsigned long GetLong(const char* p) const;
    unsigned short GetWord(const char* p) const;
    float LongAsFloat(unsigned long v) const;
};

#pragma once

#include "IMG.h"

class GENERIC {
public:
    explicit GENERIC(IMG* img);

    void GetType(const char* filename);
    int DecodeBuffer();

private:
    IMG* img_;
};

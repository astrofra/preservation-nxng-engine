#pragma once

#include "IMG.h"

#define IDCT_FLOAT 0

class JPEG {
public:
    explicit JPEG(IMG* img);
    int Save(const char* path, int /*idct_mode*/, int quality, bool flip_vertical);

private:
    IMG* img_;
};

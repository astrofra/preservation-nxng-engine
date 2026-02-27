#pragma once

#include <cstddef>

#define IMG_OK 0
#define IMG_ERROR 1

struct IMG_PixelFormat {
    int bpp;
};

class IMG {
public:
    IMG();
    ~IMG();

    void UseAfterDeath(bool enabled);
    void SetSource(unsigned char* source, unsigned long size);
    void AdoptPixels(unsigned char* pixels, int w, int h);

    bool Average(double threshold, unsigned char* r, unsigned char* g, unsigned char* b) const;
    void Apply3x3Convolution();

    int width;
    int height;
    unsigned char* adress;
    IMG_PixelFormat PixelFormat;
    int convolutionmatrix[9];

    unsigned char* source;
    unsigned long source_size;

private:
    bool own_pixels_;
    bool use_after_death_;
};

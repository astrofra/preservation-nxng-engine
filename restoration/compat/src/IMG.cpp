#include "IMG.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <vector>

IMG::IMG()
    : width(0),
      height(0),
      adress(nullptr),
      source(nullptr),
      source_size(0),
      own_pixels_(false),
      use_after_death_(false) {
    PixelFormat.bpp = 32;
    for (int& c : convolutionmatrix) {
        c = 0;
    }
}

IMG::~IMG() {
    if (own_pixels_) {
        delete[] adress;
    }
    if (!use_after_death_ && source) {
        delete[] source;
    }
}

void IMG::UseAfterDeath(bool enabled) {
    use_after_death_ = enabled;
}

void IMG::SetSource(unsigned char* src, unsigned long size) {
    source = src;
    source_size = size;
}

void IMG::AdoptPixels(unsigned char* pixels, int w, int h) {
    if (own_pixels_) {
        delete[] adress;
    }
    adress = pixels;
    width = w;
    height = h;
    PixelFormat.bpp = 32;
    own_pixels_ = true;
}

bool IMG::Average(double threshold, unsigned char* r, unsigned char* g, unsigned char* b) const {
    if (!adress || width <= 0 || height <= 0) {
        return false;
    }

    const std::size_t count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    const auto* px = reinterpret_cast<const unsigned char*>(adress);

    unsigned long long sr = 0;
    unsigned long long sg = 0;
    unsigned long long sb = 0;

    for (std::size_t i = 0; i < count; ++i) {
        const auto* p = px + i * 4;
        sb += p[0];
        sg += p[1];
        sr += p[2];
    }

    const double ar = static_cast<double>(sr) / static_cast<double>(count);
    const double ag = static_cast<double>(sg) / static_cast<double>(count);
    const double ab = static_cast<double>(sb) / static_cast<double>(count);

    if (r) *r = static_cast<unsigned char>(ar);
    if (g) *g = static_cast<unsigned char>(ag);
    if (b) *b = static_cast<unsigned char>(ab);

    // Returns true when image is close to monochrome under threshold.
    for (std::size_t i = 0; i < count; ++i) {
        const auto* p = px + i * 4;
        if (std::abs(static_cast<double>(p[2]) - ar) > 255.0 * threshold ||
            std::abs(static_cast<double>(p[1]) - ag) > 255.0 * threshold ||
            std::abs(static_cast<double>(p[0]) - ab) > 255.0 * threshold) {
            return false;
        }
    }

    return true;
}

void IMG::Apply3x3Convolution() {
    if (!adress || width < 3 || height < 3) {
        return;
    }

    std::array<int, 9> kernel{};
    int ksum = 0;
    for (int i = 0; i < 9; ++i) {
        kernel[i] = convolutionmatrix[i];
        ksum += kernel[i];
    }
    if (ksum == 0) {
        kernel = {1, 2, 1, 2, 4, 2, 1, 2, 1};
        ksum = 16;
    }

    const std::size_t npx = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    std::vector<unsigned char> out(npx * 4);
    const auto* in = reinterpret_cast<const unsigned char*>(adress);

    auto idx = [this](int x, int y) {
        return static_cast<std::size_t>(y * width + x) * 4;
    };

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int rb = 0, rg = 0, rr = 0, ra = 0;
            int k = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx, ++k) {
                    const int sx = std::clamp(x + kx, 0, width - 1);
                    const int sy = std::clamp(y + ky, 0, height - 1);
                    const std::size_t s = idx(sx, sy);
                    rb += in[s + 0] * kernel[k];
                    rg += in[s + 1] * kernel[k];
                    rr += in[s + 2] * kernel[k];
                    ra += in[s + 3] * kernel[k];
                }
            }
            const std::size_t d = idx(x, y);
            out[d + 0] = static_cast<unsigned char>(std::clamp(rb / ksum, 0, 255));
            out[d + 1] = static_cast<unsigned char>(std::clamp(rg / ksum, 0, 255));
            out[d + 2] = static_cast<unsigned char>(std::clamp(rr / ksum, 0, 255));
            out[d + 3] = static_cast<unsigned char>(std::clamp(ra / ksum, 0, 255));
        }
    }

    std::memcpy(adress, out.data(), out.size());
}

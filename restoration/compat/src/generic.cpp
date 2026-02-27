#include "generic.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstddef>

GENERIC::GENERIC(IMG* img) : img_(img) {}

void GENERIC::GetType(const char* /*filename*/) {
    // Legacy API kept for compatibility. Decoder is auto-detected by stb_image.
}

int GENERIC::DecodeBuffer() {
    if (!img_ || !img_->source || img_->source_size == 0) {
        return IMG_ERROR;
    }

    int w = 0;
    int h = 0;
    int n = 0;
    unsigned char* rgba = stbi_load_from_memory(
        img_->source,
        static_cast<int>(img_->source_size),
        &w,
        &h,
        &n,
        4);

    if (!rgba || w <= 0 || h <= 0) {
        if (rgba) {
            stbi_image_free(rgba);
        }
        return IMG_ERROR;
    }

    const std::size_t count = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
    auto* bgra = new unsigned char[count * 4];
    for (std::size_t i = 0; i < count; ++i) {
        bgra[i * 4 + 0] = rgba[i * 4 + 2];
        bgra[i * 4 + 1] = rgba[i * 4 + 1];
        bgra[i * 4 + 2] = rgba[i * 4 + 0];
        bgra[i * 4 + 3] = rgba[i * 4 + 3];
    }

    stbi_image_free(rgba);
    img_->AdoptPixels(bgra, w, h);
    return IMG_OK;
}

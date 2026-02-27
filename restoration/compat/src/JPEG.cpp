#include "JPEG.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <vector>

JPEG::JPEG(IMG* img) : img_(img) {}

int JPEG::Save(const char* path, int /*idct_mode*/, int quality, bool flip_vertical) {
    if (!img_ || !img_->adress || img_->width <= 0 || img_->height <= 0 || !path) {
        return IMG_ERROR;
    }

    stbi_flip_vertically_on_write(flip_vertical ? 1 : 0);

    const std::size_t count = static_cast<std::size_t>(img_->width) * static_cast<std::size_t>(img_->height);
    std::vector<unsigned char> rgb(count * 3);
    const auto* bgra = img_->adress;

    for (std::size_t i = 0; i < count; ++i) {
        rgb[i * 3 + 0] = bgra[i * 4 + 2];
        rgb[i * 3 + 1] = bgra[i * 4 + 1];
        rgb[i * 3 + 2] = bgra[i * 4 + 0];
    }

    const int ok = stbi_write_jpg(path, img_->width, img_->height, 3, rgb.data(), quality);
    return ok ? IMG_OK : IMG_ERROR;
}

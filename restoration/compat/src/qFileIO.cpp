#include "IO/qFileIO.h"

#include <filesystem>
#include <fstream>
#include <vector>

qFileIO::qFileIO() : last_size_(0) {}

unsigned long qFileIO::GetSize(const char* path) {
    last_size_ = 0;
    if (!path || !*path) {
        return 0;
    }
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec) {
        return 0;
    }
    last_size_ = static_cast<unsigned long>(size);
    return last_size_;
}

bool qFileIO::Exist(const char* path) {
    if (!path || !*path) {
        return false;
    }
    std::error_code ec;
    return std::filesystem::is_regular_file(path, ec);
}

unsigned char* qFileIO::Load(const char* path) {
    const auto size = GetSize(path);
    if (!size) {
        return nullptr;
    }

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return nullptr;
    }

    auto* buffer = new unsigned char[size];
    in.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(size));
    if (!in) {
        delete[] buffer;
        return nullptr;
    }
    return buffer;
}

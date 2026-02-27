#include "xPack/xPack.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace {
std::vector<unsigned char> read_file(const std::filesystem::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) {
        return {};
    }
    in.seekg(0, std::ios::end);
    const auto size = static_cast<std::size_t>(in.tellg());
    in.seekg(0, std::ios::beg);
    std::vector<unsigned char> data(size);
    if (size > 0) {
        in.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
        if (!in) {
            return {};
        }
    }
    return data;
}
}

xLinker::xLinker() : writable_(false) {}

int xLinker::OpenxLink(const char* path) {
    if (!path || !*path) {
        return XPACK_ERROR;
    }
    backing_path_ = path;
    writable_ = false;

    std::error_code ec;
    if (std::filesystem::exists(backing_path_, ec)) {
        return XPACK_OK;
    }
    return XPACK_ERROR;
}

int xLinker::CreatexLink(const char* path) {
    if (!path || !*path) {
        return XPACK_ERROR;
    }
    backing_path_ = path;
    writable_ = true;
    in_memory_entries_.clear();
    return XPACK_OK;
}

int xLinker::ClosexLink() {
    if (!writable_) {
        return XPACK_OK;
    }

    if (backing_path_.empty()) {
        return XPACK_ERROR;
    }

    std::filesystem::path dir = backing_path_ + ".d";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) {
        return XPACK_ERROR;
    }

    for (const auto& [name, data] : in_memory_entries_) {
        std::filesystem::path out = dir / name;
        std::filesystem::create_directories(out.parent_path(), ec);
        if (ec) {
            return XPACK_ERROR;
        }
        std::ofstream f(out, std::ios::binary);
        if (!f) {
            return XPACK_ERROR;
        }
        f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        if (!f) {
            return XPACK_ERROR;
        }
    }

    return XPACK_OK;
}

unsigned long xLinker::SizeFromxLink(const char* name) {
    if (!name || !*name) {
        return 0;
    }

    const std::string key(name);
    if (auto it = in_memory_entries_.find(key); it != in_memory_entries_.end()) {
        return static_cast<unsigned long>(it->second.size());
    }

    if (backing_path_.empty()) {
        return 0;
    }

    std::error_code ec;
    std::filesystem::path p = std::filesystem::path(backing_path_ + ".d") / key;
    if (std::filesystem::exists(p, ec)) {
        return static_cast<unsigned long>(std::filesystem::file_size(p, ec));
    }

    p = std::filesystem::path(name);
    if (std::filesystem::exists(p, ec)) {
        return static_cast<unsigned long>(std::filesystem::file_size(p, ec));
    }

    return 0;
}

unsigned char* xLinker::LoadFromxLink(const char* name) {
    if (!name || !*name) {
        return nullptr;
    }

    const std::string key(name);
    std::vector<unsigned char> data;

    if (auto it = in_memory_entries_.find(key); it != in_memory_entries_.end()) {
        data = it->second;
    } else if (!backing_path_.empty()) {
        std::filesystem::path p = std::filesystem::path(backing_path_ + ".d") / key;
        data = read_file(p);
        if (data.empty()) {
            data = read_file(std::filesystem::path(name));
        }
    }

    if (data.empty()) {
        return nullptr;
    }

    auto* out = new unsigned char[data.size()];
    std::copy(data.begin(), data.end(), out);
    return out;
}

int xLinker::AddToxLinkAlias(const char* source_path, const char* alias_name) {
    if (!source_path || !alias_name || !*source_path || !*alias_name) {
        return XPACK_ERROR;
    }

    auto data = read_file(std::filesystem::path(source_path));
    if (data.empty()) {
        return XPACK_ERROR;
    }

    in_memory_entries_[alias_name] = std::move(data);
    return XPACK_OK;
}

int xLinker::AddBufferToxLink(const char* alias_name, const unsigned char* buffer, unsigned long size) {
    if (!alias_name || !buffer || !*alias_name || size == 0) {
        return XPACK_ERROR;
    }

    in_memory_entries_[alias_name] = std::vector<unsigned char>(buffer, buffer + size);
    return XPACK_OK;
}

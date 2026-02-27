#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#define XPACK_OK 0
#define XPACK_ERROR 1

class xLinker {
public:
    xLinker();

    int OpenxLink(const char* path);
    int CreatexLink(const char* path);
    int ClosexLink();

    unsigned long SizeFromxLink(const char* name);
    unsigned char* LoadFromxLink(const char* name);

    int AddToxLinkAlias(const char* source_path, const char* alias_name);
    int AddBufferToxLink(const char* alias_name, const unsigned char* buffer, unsigned long size);

private:
    std::string backing_path_;
    bool writable_;
    std::unordered_map<std::string, std::vector<unsigned char>> in_memory_entries_;
};

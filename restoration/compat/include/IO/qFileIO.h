#pragma once

#include <cstddef>

class qFileIO {
public:
    qFileIO();

    // Returns file size in bytes, or 0 on failure.
    unsigned long GetSize(const char* path);

    // Returns true when the path exists and is a regular file.
    bool Exist(const char* path);

    // Allocates and returns file contents. Caller owns returned buffer.
    unsigned char* Load(const char* path);

private:
    unsigned long last_size_;
};

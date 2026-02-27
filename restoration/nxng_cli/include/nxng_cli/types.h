#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace nxng {

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Mesh {
    std::string source_name;
    std::vector<Vec3> vertices;
    std::vector<std::uint32_t> triangles;
};

struct SceneData {
    std::vector<Mesh> meshes;
    Vec3 bounds_min{0.0f, 0.0f, 0.0f};
    Vec3 bounds_max{0.0f, 0.0f, 0.0f};
    bool has_bounds{false};
};

}  // namespace nxng

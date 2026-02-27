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

struct Vec2 {
    float u;
    float v;
};

enum class TextureProjection {
    kNone = 0,
    kPlanar,
    kCylindrical,
    kCubic,
};

struct Surface {
    std::string name;
    std::string texture_path;
    Vec3 texture_center{0.0f, 0.0f, 0.0f};
    Vec3 texture_size{1.0f, 1.0f, 1.0f};
    std::uint16_t texture_flags{0};
    std::uint8_t color_r{200};
    std::uint8_t color_g{200};
    std::uint8_t color_b{200};
    TextureProjection projection{TextureProjection::kNone};
};

struct Triangle {
    std::uint32_t i0{0};
    std::uint32_t i1{0};
    std::uint32_t i2{0};
    std::int32_t surface_index{-1};
};

struct Mesh {
    std::string source_name;
    std::string source_path;
    std::vector<Vec3> vertices;
    std::vector<Triangle> triangles;
    std::vector<Surface> surfaces;
};

struct SceneData {
    std::vector<Mesh> meshes;
    Vec3 bounds_min{0.0f, 0.0f, 0.0f};
    Vec3 bounds_max{0.0f, 0.0f, 0.0f};
    bool has_bounds{false};
};

}  // namespace nxng

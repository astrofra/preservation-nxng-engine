#include "nxng_cli/scene_loader.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace nxng {
namespace {

std::string trim(std::string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || std::isspace(static_cast<unsigned char>(s.back())))) {
        s.pop_back();
    }
    std::size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) {
        ++i;
    }
    return s.substr(i);
}

bool starts_with(const std::string& s, const char* prefix) {
    const std::size_t n = std::strlen(prefix);
    return s.size() >= n && s.compare(0, n, prefix) == 0;
}

std::string normalize_legacy_path(std::string p) {
    std::replace(p.begin(), p.end(), '\\', '/');
    return p;
}

std::string basename_legacy(const std::string& path) {
    const auto normalized = normalize_legacy_path(path);
    const auto slash = normalized.find_last_of('/');
    return (slash == std::string::npos) ? normalized : normalized.substr(slash + 1);
}

std::vector<unsigned char> read_binary(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
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

std::uint16_t be16(const unsigned char* p) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(p[0]) << 8U) | p[1]);
}

std::uint32_t be32(const unsigned char* p) {
    return (static_cast<std::uint32_t>(p[0]) << 24U) |
           (static_cast<std::uint32_t>(p[1]) << 16U) |
           (static_cast<std::uint32_t>(p[2]) << 8U) |
           static_cast<std::uint32_t>(p[3]);
}

float be_f32(const unsigned char* p) {
    const std::uint32_t bits = be32(p);
    float out = 0.0f;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

void update_bounds(SceneData* scene, const Vec3& v) {
    if (!scene->has_bounds) {
        scene->bounds_min = v;
        scene->bounds_max = v;
        scene->has_bounds = true;
        return;
    }
    scene->bounds_min.x = std::min(scene->bounds_min.x, v.x);
    scene->bounds_min.y = std::min(scene->bounds_min.y, v.y);
    scene->bounds_min.z = std::min(scene->bounds_min.z, v.z);
    scene->bounds_max.x = std::max(scene->bounds_max.x, v.x);
    scene->bounds_max.y = std::max(scene->bounds_max.y, v.y);
    scene->bounds_max.z = std::max(scene->bounds_max.z, v.z);
}

bool parse_lwob(const std::filesystem::path& path, Mesh* mesh, std::string* error) {
    const auto bytes = read_binary(path);
    if (bytes.size() < 12) {
        if (error) *error = "File too small for LWO: " + path.string();
        return false;
    }

    if (std::memcmp(bytes.data(), "FORM", 4) != 0) {
        if (error) *error = "Missing FORM header: " + path.string();
        return false;
    }

    const std::string form_type(reinterpret_cast<const char*>(bytes.data() + 8), 4);
    if (form_type != "LWOB") {
        if (error) *error = "Unsupported LWO form type '" + form_type + "' in " + path.string();
        return false;
    }

    std::size_t pos = 12;
    while (pos + 8 <= bytes.size()) {
        const char* id = reinterpret_cast<const char*>(bytes.data() + pos);
        const std::uint32_t chunk_size = be32(bytes.data() + pos + 4);
        const std::size_t data_pos = pos + 8;
        if (data_pos + chunk_size > bytes.size()) {
            if (error) *error = "Corrupt chunk size in " + path.string();
            return false;
        }

        if (std::memcmp(id, "PNTS", 4) == 0) {
            const std::size_t n = chunk_size / 12;
            mesh->vertices.reserve(mesh->vertices.size() + n);
            for (std::size_t i = 0; i < n; ++i) {
                const auto* p = bytes.data() + data_pos + i * 12;
                mesh->vertices.push_back({be_f32(p + 0), -be_f32(p + 4), be_f32(p + 8)});
            }
        } else if (std::memcmp(id, "POLS", 4) == 0) {
            std::size_t p = data_pos;
            const std::size_t end = data_pos + chunk_size;
            while (p + 2 <= end) {
                const std::uint16_t nedge = be16(bytes.data() + p);
                p += 2;
                if (nedge < 3) {
                    break;
                }
                if (p + static_cast<std::size_t>(nedge) * 2 + 2 > end) {
                    break;
                }

                std::vector<std::uint16_t> face;
                face.reserve(nedge);
                for (std::uint16_t i = 0; i < nedge; ++i) {
                    face.push_back(be16(bytes.data() + p));
                    p += 2;
                }

                const std::uint16_t surf = be16(bytes.data() + p);
                p += 2;

                // Triangulate polygon as a fan.
                for (std::uint16_t i = 1; i + 1 < nedge; ++i) {
                    mesh->triangles.push_back(face[0]);
                    mesh->triangles.push_back(face[i]);
                    mesh->triangles.push_back(face[i + 1]);
                }

                // LWOB detail polygons for negative surface IDs.
                if (surf >= 0x8000 && p + 2 <= end) {
                    const std::uint16_t ndetail = be16(bytes.data() + p);
                    p += 2;
                    for (std::uint16_t d = 0; d < ndetail && p + 2 <= end; ++d) {
                        const std::uint16_t dnedge = be16(bytes.data() + p);
                        p += 2;
                        const std::size_t skip = static_cast<std::size_t>(dnedge) * 2 + 2;
                        if (p + skip > end) {
                            p = end;
                            break;
                        }
                        p += skip;
                    }
                }
            }
        }

        pos = data_pos + chunk_size + (chunk_size & 1U);
    }

    if (mesh->vertices.empty() || mesh->triangles.empty()) {
        if (error) *error = "No geometry decoded from " + path.string();
        return false;
    }

    return true;
}

std::filesystem::path resolve_object_path(const std::filesystem::path& lws_path, const std::string& raw) {
    const auto scene_dir = lws_path.parent_path();
    const std::string normalized = normalize_legacy_path(raw);

    std::vector<std::filesystem::path> candidates;
    candidates.emplace_back(normalized);
    candidates.emplace_back(scene_dir / normalized);
    candidates.emplace_back(scene_dir / basename_legacy(raw));

    for (const auto& c : candidates) {
        std::error_code ec;
        if (std::filesystem::exists(c, ec)) {
            return c;
        }
    }

    return scene_dir / basename_legacy(raw);
}

}  // namespace

bool LoadSceneFromLws(const std::filesystem::path& lws_path, SceneData* out, std::string* error) {
    if (!out) {
        if (error) *error = "Null scene output pointer";
        return false;
    }

    *out = SceneData{};

    std::ifstream in(lws_path);
    if (!in) {
        if (error) *error = "Cannot open scene file: " + lws_path.string();
        return false;
    }

    std::string line;
    std::vector<std::filesystem::path> objects;

    while (std::getline(in, line)) {
        line = trim(line);
        if (starts_with(line, "LoadObject ")) {
            const std::string raw = trim(line.substr(std::strlen("LoadObject ")));
            if (!raw.empty()) {
                objects.push_back(resolve_object_path(lws_path, raw));
            }
        }
    }

    if (objects.empty()) {
        if (error) *error = "No LoadObject entries found in " + lws_path.string();
        return false;
    }

    for (const auto& object_path : objects) {
        Mesh mesh;
        mesh.source_name = object_path.filename().string();
        std::string mesh_error;
        if (!parse_lwob(object_path, &mesh, &mesh_error)) {
            if (error) *error = mesh_error;
            return false;
        }

        for (const auto& v : mesh.vertices) {
            update_bounds(out, v);
        }

        out->meshes.push_back(std::move(mesh));
    }

    if (out->meshes.empty()) {
        if (error) *error = "No meshes loaded from scene";
        return false;
    }

    return true;
}

}  // namespace nxng

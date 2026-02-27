#include "nxng_cli/scene_loader.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace nxng {
namespace {

struct RawFace {
    std::vector<std::uint16_t> indices;
    std::uint16_t surface_index{0};
};

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

std::pair<std::string, std::size_t> read_cstr_even(const unsigned char* data, std::size_t size, std::size_t offset) {
    if (offset >= size) {
        return {"", size};
    }

    std::size_t end = offset;
    while (end < size && data[end] != 0) {
        ++end;
    }

    if (end >= size) {
        return {"", size};
    }

    std::string s(reinterpret_cast<const char*>(data + offset), end - offset);
    std::size_t next = end + 1;
    if (next & 1U) {
        ++next;
    }
    return {s, std::min(next, size)};
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

TextureProjection projection_from_string(const std::string& kind) {
    if (kind.find("Planar") != std::string::npos) {
        return TextureProjection::kPlanar;
    }
    if (kind.find("Cylindrical") != std::string::npos) {
        return TextureProjection::kCylindrical;
    }
    if (kind.find("Cubic") != std::string::npos) {
        return TextureProjection::kCubic;
    }
    return TextureProjection::kNone;
}

std::string resolve_texture_path(const std::filesystem::path& object_path, const std::string& raw_path) {
    if (raw_path.empty()) {
        return {};
    }

    const auto object_dir = object_path.parent_path();
    const auto normalized = normalize_legacy_path(raw_path);
    const auto base = basename_legacy(raw_path);

    std::vector<std::filesystem::path> candidates;
    candidates.emplace_back(normalized);
    candidates.emplace_back(object_dir / normalized);
    if (!base.empty()) {
        candidates.emplace_back(object_dir / base);
        candidates.emplace_back(object_dir.parent_path() / base);
    }

    for (const auto& c : candidates) {
        std::error_code ec;
        if (std::filesystem::exists(c, ec)) {
            return c.lexically_normal().string();
        }
    }

    return base.empty() ? std::string{} : (object_dir / base).lexically_normal().string();
}

void parse_srfs_chunk(const unsigned char* data, std::size_t size, std::vector<std::string>* out_names) {
    std::size_t p = 0;
    while (p < size) {
        const auto [name, next] = read_cstr_even(data, size, p);
        if (next <= p) {
            break;
        }
        if (!name.empty()) {
            out_names->push_back(name);
        }
        p = next;
    }
}

void parse_surf_chunk(const unsigned char* data,
                      std::size_t size,
                      const std::filesystem::path& object_path,
                      std::unordered_map<std::string, Surface>* out_surfaces) {
    const auto [name, after_name] = read_cstr_even(data, size, 0);
    if (name.empty() || after_name > size) {
        return;
    }

    Surface surface;
    surface.name = name;

    std::string raw_texture_path;
    std::size_t p = after_name;
    while (p + 6 <= size) {
        const char* sid = reinterpret_cast<const char*>(data + p);
        const std::uint16_t sub_size = be16(data + p + 4);
        const std::size_t sub_data = p + 6;
        if (sub_data + sub_size > size) {
            break;
        }

        if (std::memcmp(sid, "COLR", 4) == 0 && sub_size >= 3) {
            surface.color_r = data[sub_data + 0];
            surface.color_g = data[sub_data + 1];
            surface.color_b = data[sub_data + 2];
        } else if ((std::memcmp(sid, "CTEX", 4) == 0 ||
                    std::memcmp(sid, "DTEX", 4) == 0 ||
                    std::memcmp(sid, "STEX", 4) == 0 ||
                    std::memcmp(sid, "BTEX", 4) == 0 ||
                    std::memcmp(sid, "TTEX", 4) == 0) &&
                   sub_size > 0) {
            const auto [tex_kind, ignored] = read_cstr_even(data + sub_data, sub_size, 0);
            (void)ignored;
            surface.projection = projection_from_string(tex_kind);
        } else if (std::memcmp(sid, "TIMG", 4) == 0 && sub_size > 0) {
            const auto [path, ignored] = read_cstr_even(data + sub_data, sub_size, 0);
            (void)ignored;
            if (!path.empty()) {
                raw_texture_path = path;
            }
        } else if (std::memcmp(sid, "RIMG", 4) == 0 && sub_size > 0) {
            const auto [path, ignored] = read_cstr_even(data + sub_data, sub_size, 0);
            (void)ignored;
            if (raw_texture_path.empty() && !path.empty()) {
                raw_texture_path = path;
                if (surface.projection == TextureProjection::kNone) {
                    surface.projection = TextureProjection::kCubic;
                }
            }
        } else if (std::memcmp(sid, "TSIZ", 4) == 0 && sub_size >= 12) {
            surface.texture_size.x = be_f32(data + sub_data + 0);
            surface.texture_size.y = be_f32(data + sub_data + 4);
            surface.texture_size.z = be_f32(data + sub_data + 8);
        } else if (std::memcmp(sid, "TCTR", 4) == 0 && sub_size >= 12) {
            surface.texture_center.x = be_f32(data + sub_data + 0);
            surface.texture_center.y = be_f32(data + sub_data + 4);
            surface.texture_center.z = be_f32(data + sub_data + 8);
        } else if (std::memcmp(sid, "TFLG", 4) == 0 && sub_size >= 2) {
            surface.texture_flags = be16(data + sub_data);
        }

        p = sub_data + sub_size + (sub_size & 1U);
    }

    surface.texture_path = resolve_texture_path(object_path, raw_texture_path);
    (*out_surfaces)[name] = surface;
}

void parse_pols_chunk(const unsigned char* data, std::size_t size, std::vector<RawFace>* out_faces) {
    std::size_t p = 0;
    while (p + 2 <= size) {
        const std::uint16_t nedge = be16(data + p);
        p += 2;
        if (nedge < 3) {
            break;
        }

        const std::size_t face_bytes = static_cast<std::size_t>(nedge) * 2;
        if (p + face_bytes + 2 > size) {
            break;
        }

        RawFace face;
        face.indices.reserve(nedge);
        for (std::uint16_t i = 0; i < nedge; ++i) {
            face.indices.push_back(be16(data + p));
            p += 2;
        }

        const std::int16_t surface_ref = static_cast<std::int16_t>(be16(data + p));
        p += 2;
        face.surface_index = (surface_ref < 0)
                                 ? static_cast<std::uint16_t>(-surface_ref)
                                 : static_cast<std::uint16_t>(surface_ref);

        if (face.indices.size() >= 3) {
            out_faces->push_back(std::move(face));
        }

        // LWOB detail polygons appear when the surface index is negative.
        if (surface_ref < 0 && p + 2 <= size) {
            const std::uint16_t ndetail = be16(data + p);
            p += 2;
            for (std::uint16_t d = 0; d < ndetail && p + 2 <= size; ++d) {
                const std::uint16_t dnedge = be16(data + p);
                p += 2;
                const std::size_t skip = static_cast<std::size_t>(dnedge) * 2 + 2;
                if (p + skip > size) {
                    p = size;
                    break;
                }
                p += skip;
            }
        }
    }
}

bool parse_lwob(const std::filesystem::path& path, Mesh* mesh, std::string* error) {
    const auto bytes = read_binary(path);
    if (bytes.size() < 12) {
        if (error) {
            *error = "File too small for LWO: " + path.string();
        }
        return false;
    }

    if (std::memcmp(bytes.data(), "FORM", 4) != 0) {
        if (error) {
            *error = "Missing FORM header: " + path.string();
        }
        return false;
    }

    const std::string form_type(reinterpret_cast<const char*>(bytes.data() + 8), 4);
    if (form_type != "LWOB") {
        if (error) {
            *error = "Unsupported LWO form type '" + form_type + "' in " + path.string();
        }
        return false;
    }

    std::vector<std::string> srfs;
    std::unordered_map<std::string, Surface> parsed_surfaces;
    std::vector<RawFace> faces;

    std::size_t pos = 12;
    while (pos + 8 <= bytes.size()) {
        const char* id = reinterpret_cast<const char*>(bytes.data() + pos);
        const std::uint32_t chunk_size = be32(bytes.data() + pos + 4);
        const std::size_t data_pos = pos + 8;
        if (data_pos + chunk_size > bytes.size()) {
            if (error) {
                *error = "Corrupt chunk size in " + path.string();
            }
            return false;
        }

        if (std::memcmp(id, "PNTS", 4) == 0) {
            const std::size_t n = chunk_size / 12;
            mesh->vertices.reserve(mesh->vertices.size() + n);
            for (std::size_t i = 0; i < n; ++i) {
                const auto* p = bytes.data() + data_pos + i * 12;
                mesh->vertices.push_back({be_f32(p + 0), -be_f32(p + 4), be_f32(p + 8)});
            }
        } else if (std::memcmp(id, "SRFS", 4) == 0) {
            parse_srfs_chunk(bytes.data() + data_pos, chunk_size, &srfs);
        } else if (std::memcmp(id, "POLS", 4) == 0) {
            parse_pols_chunk(bytes.data() + data_pos, chunk_size, &faces);
        } else if (std::memcmp(id, "SURF", 4) == 0) {
            parse_surf_chunk(bytes.data() + data_pos, chunk_size, path, &parsed_surfaces);
        }

        pos = data_pos + chunk_size + (chunk_size & 1U);
    }

    if (mesh->vertices.empty() || faces.empty()) {
        if (error) {
            *error = "No geometry decoded from " + path.string();
        }
        return false;
    }

    std::unordered_map<std::string, std::int32_t> surface_name_to_index;
    for (const auto& name : srfs) {
        if (surface_name_to_index.find(name) != surface_name_to_index.end()) {
            continue;
        }
        Surface s;
        s.name = name;
        auto it = parsed_surfaces.find(name);
        if (it != parsed_surfaces.end()) {
            s = it->second;
            s.name = name;
        }
        const std::int32_t idx = static_cast<std::int32_t>(mesh->surfaces.size());
        surface_name_to_index.emplace(name, idx);
        mesh->surfaces.push_back(std::move(s));
    }

    for (const auto& [name, s] : parsed_surfaces) {
        if (surface_name_to_index.find(name) != surface_name_to_index.end()) {
            continue;
        }
        const std::int32_t idx = static_cast<std::int32_t>(mesh->surfaces.size());
        surface_name_to_index.emplace(name, idx);
        mesh->surfaces.push_back(s);
    }

    auto resolve_surface_index = [&](std::uint16_t srfs_index) -> std::int32_t {
        if (srfs_index == 0 || srfs_index > srfs.size()) {
            return -1;
        }
        const auto& name = srfs[srfs_index - 1];
        const auto it = surface_name_to_index.find(name);
        if (it == surface_name_to_index.end()) {
            return -1;
        }
        return it->second;
    };

    for (const auto& face : faces) {
        if (face.indices.size() < 3) {
            continue;
        }
        const std::int32_t tri_surface = resolve_surface_index(face.surface_index);
        for (std::size_t i = 1; i + 1 < face.indices.size(); ++i) {
            mesh->triangles.push_back(Triangle{
                static_cast<std::uint32_t>(face.indices[0]),
                static_cast<std::uint32_t>(face.indices[i]),
                static_cast<std::uint32_t>(face.indices[i + 1]),
                tri_surface,
            });
        }
    }

    return !mesh->triangles.empty();
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
        if (error) {
            *error = "Null scene output pointer";
        }
        return false;
    }

    *out = SceneData{};

    std::ifstream in(lws_path);
    if (!in) {
        if (error) {
            *error = "Cannot open scene file: " + lws_path.string();
        }
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
        if (error) {
            *error = "No LoadObject entries found in " + lws_path.string();
        }
        return false;
    }

    for (const auto& object_path : objects) {
        Mesh mesh;
        mesh.source_name = object_path.filename().string();
        mesh.source_path = object_path.string();
        std::string mesh_error;
        if (!parse_lwob(object_path, &mesh, &mesh_error)) {
            if (error) {
                *error = mesh_error;
            }
            return false;
        }

        for (const auto& v : mesh.vertices) {
            update_bounds(out, v);
        }

        out->meshes.push_back(std::move(mesh));
    }

    if (out->meshes.empty()) {
        if (error) {
            *error = "No meshes loaded from scene";
        }
        return false;
    }

    return true;
}

}  // namespace nxng

#include "nxng_cli/renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include <GLFW/glfw3.h>

#include "stb_image.h"

namespace nxng {
namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 6.28318530717958647692f;

enum class Axis {
    kX,
    kY,
    kZ,
};

Vec3 sub(const Vec3& a, const Vec3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

float length(const Vec3& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 normalized(const Vec3& v) {
    const float len = length(v);
    if (len < 1.0e-6f) {
        return {0.0f, 1.0f, 0.0f};
    }
    return {v.x / len, v.y / len, v.z / len};
}

float safe_scale(float s) {
    if (std::fabs(s) < 1.0e-6f) {
        return 1.0f;
    }
    return s;
}

Axis axis_from_flags(std::uint16_t flags) {
    // Legacy LightWave flags appear to use low bits for texture axis.
    if ((flags & 0x0001U) != 0) {
        return Axis::kX;
    }
    if ((flags & 0x0002U) != 0) {
        return Axis::kY;
    }
    if ((flags & 0x0004U) != 0) {
        return Axis::kZ;
    }
    return Axis::kY;
}

Axis dominant_axis(const Vec3& n) {
    const float ax = std::fabs(n.x);
    const float ay = std::fabs(n.y);
    const float az = std::fabs(n.z);
    if (ax >= ay && ax >= az) {
        return Axis::kX;
    }
    if (ay >= ax && ay >= az) {
        return Axis::kY;
    }
    return Axis::kZ;
}

Vec2 planar_uv(const Vec3& local, const Vec3& tex_size, Axis axis) {
    const float sx = safe_scale(tex_size.x);
    const float sy = safe_scale(tex_size.y);
    const float sz = safe_scale(tex_size.z);

    switch (axis) {
    case Axis::kX:
        return {local.z / sz + 0.5f, local.y / sy + 0.5f};
    case Axis::kY:
        return {local.x / sx + 0.5f, local.z / sz + 0.5f};
    case Axis::kZ:
        return {local.x / sx + 0.5f, local.y / sy + 0.5f};
    }

    return {local.x / sx + 0.5f, local.z / sz + 0.5f};
}

Vec2 cylindrical_uv(const Vec3& local, const Vec3& tex_size, Axis axis) {
    const float sx = safe_scale(tex_size.x);
    const float sy = safe_scale(tex_size.y);
    const float sz = safe_scale(tex_size.z);

    switch (axis) {
    case Axis::kX: {
        const float u = 0.5f + std::atan2(local.z, local.y) / kTwoPi;
        const float v = local.x / sx + 0.5f;
        return {u, v};
    }
    case Axis::kY: {
        const float u = 0.5f + std::atan2(local.x, local.z) / kTwoPi;
        const float v = local.y / sy + 0.5f;
        return {u, v};
    }
    case Axis::kZ: {
        const float u = 0.5f + std::atan2(local.x, local.y) / kTwoPi;
        const float v = local.z / sz + 0.5f;
        return {u, v};
    }
    }

    return {0.0f, 0.0f};
}

Vec2 compute_uv(const Vec3& v, const Surface& surface, const Vec3& tri_normal) {
    const Vec3 local = sub(v, surface.texture_center);

    switch (surface.projection) {
    case TextureProjection::kPlanar:
        return planar_uv(local, surface.texture_size, axis_from_flags(surface.texture_flags));
    case TextureProjection::kCylindrical:
        return cylindrical_uv(local, surface.texture_size, axis_from_flags(surface.texture_flags));
    case TextureProjection::kCubic:
        return planar_uv(local, surface.texture_size, dominant_axis(tri_normal));
    case TextureProjection::kNone:
        break;
    }

    return planar_uv(local, surface.texture_size, Axis::kY);
}

GLuint create_texture_2d(const std::string& path) {
    if (path.empty()) {
        return 0;
    }

    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        return 0;
    }

    int w = 0;
    int h = 0;
    int comp = 0;
    unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &comp, 4);
    if (!pixels || w <= 0 || h <= 0) {
        if (pixels) {
            stbi_image_free(pixels);
        }
        return 0;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 w,
                 h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pixels);
    stbi_image_free(pixels);
    return tex;
}

class OpenGlRenderer final : public Renderer {
public:
    int Run(const SceneData& scene, const std::string& title) override {
        if (!glfwInit()) {
            return 2;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

        GLFWwindow* window = glfwCreateWindow(1280, 720, title.c_str(), nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            return 3;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        stbi_set_flip_vertically_on_load(1);

        std::unordered_map<std::string, GLuint> texture_cache;
        for (const auto& mesh : scene.meshes) {
            for (const auto& surface : mesh.surfaces) {
                if (surface.texture_path.empty()) {
                    continue;
                }
                if (texture_cache.find(surface.texture_path) != texture_cache.end()) {
                    continue;
                }
                texture_cache[surface.texture_path] = create_texture_2d(surface.texture_path);
            }
        }

        const float cx = (scene.bounds_min.x + scene.bounds_max.x) * 0.5f;
        const float cy = (scene.bounds_min.y + scene.bounds_max.y) * 0.5f;
        const float cz = (scene.bounds_min.z + scene.bounds_max.z) * 0.5f;

        const float dx = scene.bounds_max.x - scene.bounds_min.x;
        const float dy = scene.bounds_max.y - scene.bounds_min.y;
        const float dz = scene.bounds_max.z - scene.bounds_min.z;
        const float radius = std::max(1.0f, std::sqrt(dx * dx + dy * dy + dz * dz) * 0.5f);

        float yaw = 25.0f;
        float pitch = -20.0f;
        float distance = radius * 2.5f;
        bool auto_rotate = true;

        while (!glfwWindowShouldClose(window)) {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                yaw -= 1.0f;
            }
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                yaw += 1.0f;
            }
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                pitch -= 1.0f;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                pitch += 1.0f;
            }
            if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
                distance = std::max(radius * 0.5f, distance * 0.98f);
            }
            if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
                distance = std::min(radius * 20.0f, distance * 1.02f);
            }
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                auto_rotate = false;
            }

            if (auto_rotate) {
                yaw += 0.2f;
            }

            int fbw = 0;
            int fbh = 0;
            glfwGetFramebufferSize(window, &fbw, &fbh);
            fbh = std::max(1, fbh);

            glViewport(0, 0, fbw, fbh);
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.05f, 0.06f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            const float aspect = static_cast<float>(fbw) / static_cast<float>(fbh);
            const float fov_deg = 60.0f;
            const float near_z = std::max(0.01f, radius * 0.01f);
            const float far_z = radius * 100.0f;
            const float top = near_z * std::tan(fov_deg * kPi / 360.0f);
            const float right = top * aspect;
            glFrustum(-right, right, -top, top, near_z, far_z);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glTranslatef(0.0f, 0.0f, -distance);
            glRotatef(pitch, 1.0f, 0.0f, 0.0f);
            glRotatef(yaw, 0.0f, 1.0f, 0.0f);
            glTranslatef(-cx, -cy, -cz);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_BLEND);

            for (const auto& mesh : scene.meshes) {
                for (const auto& tri : mesh.triangles) {
                    if (tri.i0 >= mesh.vertices.size() || tri.i1 >= mesh.vertices.size() || tri.i2 >= mesh.vertices.size()) {
                        continue;
                    }

                    const Surface* surface = nullptr;
                    if (tri.surface_index >= 0 && static_cast<std::size_t>(tri.surface_index) < mesh.surfaces.size()) {
                        surface = &mesh.surfaces[static_cast<std::size_t>(tri.surface_index)];
                    }

                    GLuint tex = 0;
                    if (surface && !surface->texture_path.empty()) {
                        const auto it = texture_cache.find(surface->texture_path);
                        if (it != texture_cache.end()) {
                            tex = it->second;
                        }
                    }

                    if (tex != 0) {
                        glEnable(GL_TEXTURE_2D);
                        glBindTexture(GL_TEXTURE_2D, tex);
                        glColor3f(1.0f, 1.0f, 1.0f);
                    } else {
                        glDisable(GL_TEXTURE_2D);
                        if (surface) {
                            glColor3ub(surface->color_r, surface->color_g, surface->color_b);
                        } else {
                            glColor3f(0.8f, 0.8f, 0.8f);
                        }
                    }

                    const Vec3& v0 = mesh.vertices[tri.i0];
                    const Vec3& v1 = mesh.vertices[tri.i1];
                    const Vec3& v2 = mesh.vertices[tri.i2];
                    const Vec3 n = normalized(cross(sub(v1, v0), sub(v2, v0)));

                    glBegin(GL_TRIANGLES);
                    glNormal3f(n.x, n.y, n.z);

                    if (tex != 0 && surface) {
                        const Vec2 uv0 = compute_uv(v0, *surface, n);
                        const Vec2 uv1 = compute_uv(v1, *surface, n);
                        const Vec2 uv2 = compute_uv(v2, *surface, n);
                        glTexCoord2f(uv0.u, uv0.v);
                        glVertex3f(v0.x, v0.y, v0.z);
                        glTexCoord2f(uv1.u, uv1.v);
                        glVertex3f(v1.x, v1.y, v1.z);
                        glTexCoord2f(uv2.u, uv2.v);
                        glVertex3f(v2.x, v2.y, v2.z);
                    } else {
                        glVertex3f(v0.x, v0.y, v0.z);
                        glVertex3f(v1.x, v1.y, v1.z);
                        glVertex3f(v2.x, v2.y, v2.z);
                    }

                    glEnd();
                }
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        for (const auto& [path, tex] : texture_cache) {
            (void)path;
            if (tex != 0) {
                glDeleteTextures(1, &tex);
            }
        }

        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }
};

}  // namespace

std::unique_ptr<Renderer> CreateOpenGlRenderer() {
    return std::make_unique<OpenGlRenderer>();
}

}  // namespace nxng

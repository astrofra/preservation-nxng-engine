#include "nxng_cli/renderer.h"

#include <cmath>
#include <memory>
#include <string>

#include <GLFW/glfw3.h>

namespace nxng {
namespace {

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
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) yaw -= 1.0f;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yaw += 1.0f;
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) pitch -= 1.0f;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) pitch += 1.0f;
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
            const float top = near_z * std::tan(fov_deg * 3.1415926535f / 360.0f);
            const float right = top * aspect;
            glFrustum(-right, right, -top, top, near_z, far_z);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glTranslatef(0.0f, 0.0f, -distance);
            glRotatef(pitch, 1.0f, 0.0f, 0.0f);
            glRotatef(yaw, 0.0f, 1.0f, 0.0f);
            glTranslatef(-cx, -cy, -cz);

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            for (std::size_t m = 0; m < scene.meshes.size(); ++m) {
                const auto& mesh = scene.meshes[m];
                const float hue = static_cast<float>((m * 37) % 255) / 255.0f;
                glColor3f(0.2f + hue * 0.8f, 0.6f, 1.0f - hue * 0.7f);

                glBegin(GL_TRIANGLES);
                for (std::size_t i = 0; i + 2 < mesh.triangles.size(); i += 3) {
                    const auto i0 = mesh.triangles[i + 0];
                    const auto i1 = mesh.triangles[i + 1];
                    const auto i2 = mesh.triangles[i + 2];
                    if (i0 >= mesh.vertices.size() || i1 >= mesh.vertices.size() || i2 >= mesh.vertices.size()) {
                        continue;
                    }
                    const auto& v0 = mesh.vertices[i0];
                    const auto& v1 = mesh.vertices[i1];
                    const auto& v2 = mesh.vertices[i2];
                    glVertex3f(v0.x, v0.y, v0.z);
                    glVertex3f(v1.x, v1.y, v1.z);
                    glVertex3f(v2.x, v2.y, v2.z);
                }
                glEnd();
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
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

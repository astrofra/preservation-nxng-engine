#include <filesystem>
#include <iostream>
#include <string>

#include "nxng_cli/renderer.h"
#include "nxng_cli/scene_loader.h"

namespace {

void print_usage(const char* argv0) {
    std::cout << "Usage: " << argv0 << " --scene <path/to/scene.lws> [--no-window]\n";
}

}  // namespace

int main(int argc, char** argv) {
    std::filesystem::path scene_path = "demos/vestige/scene.lws";
    bool no_window = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--scene") {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
                return 1;
            }
            scene_path = argv[++i];
        } else if (arg == "--no-window") {
            no_window = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    nxng::SceneData scene;
    std::string error;
    if (!nxng::LoadSceneFromLws(scene_path, &scene, &error)) {
        std::cerr << "Failed to load scene: " << error << "\n";
        return 2;
    }

    std::size_t vertex_count = 0;
    std::size_t triangle_count = 0;
    for (const auto& mesh : scene.meshes) {
        vertex_count += mesh.vertices.size();
        triangle_count += mesh.triangles.size() / 3;
    }

    std::cout << "Loaded scene: " << scene_path << "\n";
    std::cout << "Objects: " << scene.meshes.size() << ", Vertices: " << vertex_count
              << ", Triangles: " << triangle_count << "\n";

    if (no_window) {
        return 0;
    }

    auto renderer = nxng::CreateOpenGlRenderer();
    if (!renderer) {
        std::cerr << "Failed to create renderer\n";
        return 3;
    }

    return renderer->Run(scene, "nxng-cli (OpenGL)");
}

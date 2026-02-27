#pragma once

#include <memory>
#include <string>

#include "nxng_cli/types.h"

namespace nxng {

class Renderer {
public:
    virtual ~Renderer() = default;
    virtual int Run(const SceneData& scene, const std::string& title) = 0;
};

std::unique_ptr<Renderer> CreateOpenGlRenderer();

}  // namespace nxng

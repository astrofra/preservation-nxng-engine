#pragma once

#include <filesystem>
#include <string>

#include "nxng_cli/types.h"

namespace nxng {

bool LoadSceneFromLws(const std::filesystem::path& lws_path, SceneData* out, std::string* error);

}  // namespace nxng

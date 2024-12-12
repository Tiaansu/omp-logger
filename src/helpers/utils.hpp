#pragma once

#include <fmt/color.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <cstdio>
#include "omp-logger.hpp"
#include "../component.hpp"

namespace fs = std::filesystem;

namespace helpers
{
    const char* GetLogLevelName(OmpLogger::ELogLevel level);

    bool CreatePathRecursively(const fs::path& path);

    fmt::rgb GetLogLevelColorFromConfig(StringView key);

    uint32_t StringToHex(const std::string& hex);

    std::string SanitizeScriptName(const std::string& name);
}
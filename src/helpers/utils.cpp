#include <algorithm>
#include <iostream>
#include "utils.hpp"

const char* helpers::GetLogLevelName(OmpLogger::LogLevel level)
{
    switch (level)
    {
        case OmpLogger::LogLevel::Debug:
            return "Debug";
        case OmpLogger::LogLevel::Info:
            return "Info";
        case OmpLogger::LogLevel::Warning:
            return "Warning";
        case OmpLogger::LogLevel::Error:
            return "Error";
        case OmpLogger::LogLevel::Fatal:
            return "Fatal";
    }
    return "<unknown>";
}

bool helpers::CreatePathRecursively(const fs::path& path)
{
    if (fs::exists(path.parent_path()))
    {
        return true;
    }
    fs::create_directories(path.parent_path());

    std::ofstream file(path);
    if (!file.is_open())
    {
        return false;
    }
    
    file.close();
    return true;
}

fmt::rgb helpers::GetLogLevelColorFromConfig(StringView key)
{
    auto ompLogger = OmpLoggerComponent::Get();
    auto core = ompLogger->getCore();
    IConfig& config = core->getConfig();

    StringView value = config.getString(key);

    if (value.empty())
    {
        return fmt::color::white;
    }
    return fmt::rgb(helpers::StringToHex(value.data()));
}

uint32_t helpers::StringToHex(const std::string& hex)
{
    uint32_t result = 0xFFFFFF;
    int ret;

    if (hex.substr(0, 2) == "0x")
    {
        ret = sscanf(hex.c_str(), "0x%x", &result);
    }
    else
    {
        ret = sscanf(hex.c_str(), "%x", &result);
    }
    return ret == 1 ? result : 0xFFFFFF;
}
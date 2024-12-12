#include <algorithm>
#include <iostream>
#include "utils.hpp"

const char* helpers::GetLogLevelName(OmpLogger::ELogLevel level)
{
    switch (level)
    {
        case OmpLogger::ELogLevel::Debug:
            return "Debug";
        case OmpLogger::ELogLevel::Info:
            return "Info";
        case OmpLogger::ELogLevel::Warning:
            return "Warning";
        case OmpLogger::ELogLevel::Error:
            return "Error";
        case OmpLogger::ELogLevel::Fatal:
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

std::string helpers::SanitizeScriptName(const std::string& name)
{
    std::string output = name;
    
    size_t amxPos = output.rfind(".amx");
    if (amxPos != std::string::npos && amxPos + 4 == output.length())
    {
        output.replace(amxPos, 4, "");
    }

    std::size_t spacePos = output.find(' ');
    if (spacePos != std::string::npos)
    {
        return output.substr(0, spacePos);
    }
    return output;
}
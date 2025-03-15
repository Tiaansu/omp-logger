#pragma once

#include <string_view>

namespace LoggerConfig
{
    inline constexpr std::string_view log_directory                = "logger.log_directory";
    inline constexpr std::string_view log_format                   = "logger.log_format";
    inline constexpr std::string_view timestamp_format             = "logger.timestamp_format";

    inline constexpr std::string_view enable_source_for_all_levels = "logger.enable_source_for_all_level";
    inline constexpr std::string_view display_source               = "logger.display_source";
    inline constexpr std::string_view log_level_capitalized        = "logger.is_log_level_capitalized";

    namespace Color
    {
        inline constexpr std::string_view enabled_timestamp        = "logger.color.enabled_timestamp";
        inline constexpr std::string_view enabled_log_level        = "logger.color.enabled_log_level";
        inline constexpr std::string_view enabled_name             = "logger.color.enabled_name";

        namespace Value
        {
            inline constexpr std::string_view debug                = "logger.color.debug";
            inline constexpr std::string_view info                 = "logger.color.info";
            inline constexpr std::string_view warning              = "logger.color.warning";
            inline constexpr std::string_view error                = "logger.color.error";
            inline constexpr std::string_view fatal                = "logger.color.fatal";
        }
    }
}
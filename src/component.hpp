#pragma once

#include <map>
#include <string>
#include <any>
#include <string_view>

#include <fmt/color.h>
#include <sdk.hpp>
#include <Server/Components/Pawn/pawn.hpp>
#include <Impl/pool_impl.hpp>

#include "omp-logger.hpp"
#include "logger-config.hpp"
#include "logger.hpp"
#include "result.hpp"

class OmpLoggerComponent final
    : public IOmpLoggerComponent
    , public PawnEventHandler
    , public CoreEventHandler
{
private:
    std::map<std::string, std::any> config_ = 
    {
        {make_string(LoggerConfig::enable_source_for_all_levels), false},
        {make_string(LoggerConfig::display_source),               true},
        {make_string(LoggerConfig::log_level_capitalized),        true},
        {make_string(LoggerConfig::log_directory),                "logs"},
        {make_string(LoggerConfig::log_format),                   "[{{timestamp}}] [{{name}}] [{{log_level}}] {{message}}"},
        {make_string(LoggerConfig::timestamp_format),             "%Y-%m-%dT%H:%M:%S%z"},
        {make_string(LoggerConfig::Color::enabled_timestamp),     false},
        {make_string(LoggerConfig::Color::enabled_log_level),     false},
        {make_string(LoggerConfig::Color::enabled_name),          false},
        {make_string(LoggerConfig::Color::Value::debug),          "0xADD8E6"},
        {make_string(LoggerConfig::Color::Value::info),           "0x90EE90"},
        {make_string(LoggerConfig::Color::Value::warning),        "0xFFD700"},
        {make_string(LoggerConfig::Color::Value::error),          "0xFFB266"},
        {make_string(LoggerConfig::Color::Value::fatal),          "0xFF7F7F"},
    };

    ICore* core_ = nullptr;
    IPawnComponent* pawn_ = nullptr;
    MarkedPoolStorage<Logger, IOmpLogger, 1, 1000> pool_;
    MarkedPoolStorage<LogsResult, ILogsResult, 1, 5000> logsResults_;
    inline static OmpLoggerComponent* instance_ = nullptr;

    // configs
    std::map<OmpLogger::LogLevel, fmt::rgb> logLevelColors_ = {};
    bool isLogLevelNameCapitalized_ = false;
    bool isLoggingWithSource_ = true;
    bool isEnableSourceForAll_ = false;
    bool isTimestampColorized_ = false;
    bool isLogLevelColorized_ = false;
    bool isLogNameColorized_ = false;
    String logTimestampFormat_;
    String logDirectoryPath_;
    String logFormat_;
    std::FILE* serverLoggerFile_ = nullptr;

    static std::string make_string(std::string_view sv)
    {
        return std::string(sv);
    }

public:
    ~OmpLoggerComponent();

    // API - Logger
    IOmpLogger* createLogger(StringView name, int32_t color, OmpLogger::LogLevel level, bool isPlugin) override;
    bool destroyLogger(IOmpLogger* logger) override;
    IOmpLogger* getLogger(int id) override;

    // API - Result
    ILogsResult* initLogsResult(std::vector<std::string> logs);
    bool deleteLogsResult(ILogsResult* result);
    ILogsResult* getLogsResult(int id);

    // Component
    StringView componentName() const override
    {
        return "Logger";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(2, 0, 0, 0);
    }

    void onTick(Microseconds elapsed, TimePoint now) override;
    void onLoad(ICore* c) override;
    void onInit(IComponentList* components) override;
    void onReady() override;
    void onFree(IComponent* component) override;
    void free() override;
    void reset() override;
    void onAmxLoad(IPawnScript& script) override;
    void onAmxUnload(IPawnScript& script) override;
    void provideConfiguration(ILogger& logger, IEarlyConfig& config, bool defaults) override;

    static OmpLoggerComponent* Get()
    {
        if (instance_ == nullptr)
        {
            instance_ = new OmpLoggerComponent();
        }
        return instance_;
    }

    ICore* getCore()
    {
        return core_;
    }

    // API - Config
    fmt::rgb getLogLevelColor(OmpLogger::LogLevel level) const;
    bool isLogLevelNameCapitalized() const;
    bool isLoggingWithSource() const;
    bool isEnableSourceForAll() const;
    bool isTimestampColorized() const;
    bool isLogLevelColorized() const;
    bool isLogNameColorized() const;
    String getLogTimestampFormat() const;
    String getLogFormat() const;
    std::FILE* getServerLogFile() const;
};
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
#include "omp-log.hpp"
#include "debug-manager.hpp"
#include "logs-result.hpp"
#include "logger-config.hpp"

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
    MarkedPoolStorage<OmpLog, IOmpLog, 1, 1000> pool_;
    MarkedPoolStorage<LogsResult, ILogsResult, 1, 5000> logsResults_;
    inline static OmpLoggerComponent* instance_ = nullptr;

    // configs
    std::map<OmpLogger::ELogLevel, fmt::rgb> logLevelColors_ = {};
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

    // API
    IOmpLog* createLogger(StringView name, int32_t color, OmpLogger::ELogLevel level, bool isPlugin) override;

    bool destroyLogger(IOmpLog* logger) override;

    IOmpLog* getLogger(int id) override;

    // Fetch logs
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
        return SemanticVersion(0, 0, 1, 0);
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
    fmt::rgb getLogLevelColor(OmpLogger::ELogLevel level)
    {
        auto it = logLevelColors_.find(level);
        return it == logLevelColors_.end() ? fmt::color::white : it->second;
    }
    
    bool IsLogLevelNameCapitalized()
    {
        return isLogLevelNameCapitalized_;
    }

    bool IsLoggingWithSource()
    {
        return isLoggingWithSource_;
    }

    bool IsEnableSourceForAll()
    {
        return isEnableSourceForAll_;
    }

    bool IsTimestampColorized()
    {
        return isTimestampColorized_;
    }

    bool IsLogLevelColorized()
    {
        return isLogLevelColorized_;
    }

    bool IsLogNameColorized()
    {
        return isLogNameColorized_;
    }

    String getLogTimestampFormat()
    {
        return logTimestampFormat_;
    }

    String getLogFormat()
    {
        return logFormat_;
    }

    std::FILE* getServerLogFile()
    {
        return serverLoggerFile_;
    }

    // Debugger
    void debugRegisterAMX(AMX* amx) override;

    void debugEraseAMX(AMX* amx) override;

    bool debugInitData(const char* filepath);

    bool debugGetFunctionCall(AMX* const amx, ucell address, AmxFuncCallInfo& dest);
};
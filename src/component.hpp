#pragma once

#include <map>
#include <string>
#include <any>

#include <fmt/color.h>
#include <sdk.hpp>
#include <Server/Components/Pawn/pawn.hpp>
#include <Impl/pool_impl.hpp>

#include "omp-logger.hpp"
#include "omp-log.hpp"
#include "debug-manager.hpp"
#include "logs-result.hpp"
#include "common.hpp"

class OmpLoggerComponent final
    : public IOmpLoggerComponent
    , public PawnEventHandler
    , public CoreEventHandler
{
private:
    std::map<std::string, std::any> config_ = 
    {
        {LOGGER_CONFIG_KEY_ENABLE_SOURCE_FOR_ALL_LEVEL, false},
        {LOGGER_CONFIG_KEY_DISPLAY_SOURCE, true},
        {LOGGER_CONFIG_KEY_LOG_DIRECTORY, "logs"},
        {LOGGER_CONFIG_KEY_IS_LOG_LEVEL_UPPERCASE, false},
        {LOGGER_CONFIG_KEY_COLOR_DEBUG, "0xADD8E6"},
        {LOGGER_CONFIG_KEY_COLOR_INFO, "0x90EE90"},
        {LOGGER_CONFIG_KEY_COLOR_WARNING, "0xFFD700"},
        {LOGGER_CONFIG_KEY_COLOR_ERROR, "0xFFB266"},
        {LOGGER_CONFIG_KEY_COLOR_FATAL, "0xFF7F7F"},
        {LOGGER_CONFIG_KEY_TIMESTAMP_FORMAT, "%Y-%m-%dT%H:%M:%S%z"},
        {LOGGER_CONFIG_KEY_COLORIZED_TIMESTAMP, false}
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
    String logTimestampFormat_;
    String logDirectoryPath_;
    std::FILE* serverLoggerFile_ = nullptr;

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
    fmt::rgb getLogLevelColor(OmpLogger::ELogLevel level);

    bool IsLogLevelNameCapitalized();

    bool IsLoggingWithSource();

    bool IsEnableSourceForAll();

    bool IsTimestampColorized();

    String getLogTimestampFormat();

    std::FILE* getServerLoggingFile();

    // Debugger
    void debugRegisterAMX(AMX* amx) override;

    void debugEraseAMX(AMX* amx) override;

    bool debugInitData(const char* filepath);

    bool debugGetFunctionCall(AMX* const amx, ucell address, AmxFuncCallInfo& dest);
};
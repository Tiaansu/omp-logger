#pragma once

#include <map>
#include <string>

#include <fmt/color.h>
#include <sdk.hpp>
#include <Server/Components/Pawn/pawn.hpp>
#include <Impl/pool_impl.hpp>

#include "omp-logger.hpp"
#include "omp-log.hpp"
#include "debug-manager.hpp"

class OmpLoggerComponent final
    : public IOmpLoggerComponent
    , public PawnEventHandler
{
private:
    ICore* core_ = nullptr;

    IPawnComponent* pawn_ = nullptr;

    MarkedPoolStorage<OmpLog, IOmpLog, 1, 1000> pool_;

    inline static OmpLoggerComponent* instance_ = nullptr;

    // configs
    std::map<OmpLogger::ELogLevel, fmt::rgb> logLevelColors_ = {};
    bool isLogLevelNameCapitalized_ = false;
    bool isLoggingWithSource_ = true;
    bool isEnableSourceForAll_ = false;
    String logTimestampFormat_;
    String logDirectoryPath_;
    std::FILE* serverLoggerFile_ = nullptr;

public:
    ~OmpLoggerComponent();

    // API
    IOmpLog* createLogger(StringView name, int32_t color, OmpLogger::ELogLevel level, bool isPlugin) override;

    bool destroyLogger(IOmpLog* logger) override;

    IOmpLog* getLogger(int id) override;

    // Component
    StringView componentName() const override
    {
        return "Logger";
    }
    
    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 1, 0);
    }
    
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

    String getLogTimestampFormat();

    std::FILE* getServerLoggingFile();

    // Debugger
    void debugRegisterAMX(AMX* amx) override;

    void debugEraseAMX(AMX* amx) override;

    bool debugInitData(const char* filepath);

    bool debugGetFunctionCall(AMX* const amx, ucell address, AmxFuncCallInfo& dest);
};
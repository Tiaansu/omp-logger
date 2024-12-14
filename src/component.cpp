#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>
#include <fmt/format.h>
#include "component.hpp"
#include "helpers/utils.hpp"

// API
IOmpLog* OmpLoggerComponent::createLogger(StringView name, int32_t color, OmpLogger::ELogLevel level, bool isPlugin)
{
    String path = name.data();
    std::replace(path.begin(), path.end(), '/', '_');

    auto filepath = fmt::format("{}/{}{}.log", logDirectoryPath_, isPlugin ? "plugins/" : "", name);

    bool ret = helpers::CreatePathRecursively(filepath);

    if (!ret)
    {
        core_->logLn(LogLevel::Error, "Failed to create log file \"%s\".", filepath.c_str());
        return nullptr;
    }

    if (color < 0)
    {
        core_->logLn(LogLevel::Warning, "Negative number found (%i) in logger \"%s\". Please provide a valid color (non-negative).", color, name.data());
        color = 0;
    }

    std::FILE* file = ::fopen(filepath.c_str(), "a+");
    if (file == nullptr)
    {
        core_->logLn(LogLevel::Error, "Failed to open log file \"%s\".", filepath.c_str());
        return nullptr;
    }

    return pool_.emplace(name, color, level, file);
}

bool OmpLoggerComponent::destroyLogger(IOmpLog* logger)
{
    int id = static_cast<IOmpLog*>(logger)->getID();
    pool_.release(id, false);
    return true;
}

IOmpLog* OmpLoggerComponent::getLogger(int id)
{
    return pool_.get(id);
}

// Callbacks
void OmpLoggerComponent::onLoad(ICore* c)
{
    core_ = c;
    setAmxLookups(core_);
}

void OmpLoggerComponent::onInit(IComponentList* components)
{
    pawn_ = components->queryComponent<IPawnComponent>();
    if (pawn_ == nullptr)
    {
        StringView name = componentName();
        core_->logLn(LogLevel::Error, "Error loading component %.*s: Pawn component not loaded", name.length(), name.data());
        return;
    }

    setAmxFunctions(pawn_->getAmxFunctions());
    setAmxLookups(components);
    pawn_->getEventDispatcher().addEventHandler(this);
    core_->getEventDispatcher().addEventHandler(this);

    IConfig& config = core_->getConfig();
    isLogLevelNameCapitalized_ = (config.getBool("logger.log_level_capitalized")) ? (*config.getBool("logger.log_level_capitalized")) : false;
    isLoggingWithSource_ = (config.getBool("logger.display_source")) ? (*config.getBool("logger.display_source")) : true;
    isEnableSourceForAll_ = (config.getBool("logger.enable_source_for_all")) ? (*config.getBool("logger.enable_source_for_all")) : false;
    logTimestampFormat_ = String(config.getString("logger.timestamp_format"));
    logDirectoryPath_ = String(config.getString("logger.log_directory"));

    serverLoggerFile_ = ::fopen(String(config.getString("logging.file")).c_str(), "a");

    logLevelColors_[OmpLogger::ELogLevel::Debug] = helpers::GetLogLevelColorFromConfig("logger.colors.debug");
    logLevelColors_[OmpLogger::ELogLevel::Info] = helpers::GetLogLevelColorFromConfig("logger.colors.info");
    logLevelColors_[OmpLogger::ELogLevel::Warning] = helpers::GetLogLevelColorFromConfig("logger.colors.warning");
    logLevelColors_[OmpLogger::ELogLevel::Error] = helpers::GetLogLevelColorFromConfig("logger.colors.error");
    logLevelColors_[OmpLogger::ELogLevel::Fatal] = helpers::GetLogLevelColorFromConfig("logger.colors.fatal");

    DynamicArray<StringView> mainScripts(config.getStringsCount("pawn.main_scripts"));
    config.getStrings("pawn.main_scripts", Span<StringView>(mainScripts.data(), mainScripts.size()));

    for (auto& script : mainScripts)
    {
        String filepath = "gamemodes/" + helpers::SanitizeScriptName(String(script)) + ".amx";
        debugInitData(filepath.c_str());
    }

    DynamicArray<StringView> sideScripts(config.getStringsCount("pawn.side_scripts"));
    config.getStrings("pawn.side_scripts", Span<StringView>(sideScripts.data(), sideScripts.size()));
    for (auto& script : sideScripts)
    {
        debugInitData(String(script).c_str());
    }
}

void OmpLoggerComponent::onReady()
{
}

void OmpLoggerComponent::onFree(IComponent* component)
{
    if (component == pawn_)
    {
        pawn_ = nullptr;
        setAmxFunctions();
        setAmxLookups();
    }
}

void OmpLoggerComponent::provideConfiguration(ILogger& logger, IEarlyConfig& config, bool defaults)
{
    if (defaults)
    {
        config.setBool("logger.log_level_capitalized", false);
        config.setBool("logger.enable_source_for_all", false);
        config.setBool("logger.display_source", true);
        config.setString("logger.colors.debug", "0xADD8E6");
        config.setString("logger.colors.info", "0x90EE90");
        config.setString("logger.colors.warning", "0xFFD700");
        config.setString("logger.colors.error", "0xFFB266");
        config.setString("logger.colors.fatal", "0xFF7F7F");
        config.setString("logger.timestamp_format", "%Y-%m-%dT%H:%M:%S%z");
        config.setString("logger.log_directory", "logs");
    }
    else
    {
        if (config.getType("logger.display_source") == ConfigOptionType_None)
        {
            config.setBool("logger.display_source", true);
        }
        if (config.getType("logger.log_level_capitalized") == ConfigOptionType_None)
        {
            config.setBool("logger.log_level_capitalized", false);
        }
        if (config.getType("logger.enable_source_for_all") == ConfigOptionType_None)
        {
            config.setBool("logger.enable_source_for_all", false);
        }
        if (config.getType("logger.colors.debug") == ConfigOptionType_None)
        {
            config.setString("logger.colors.debug", "0xADD8E6");
        }
        if (config.getType("logger.colors.info") == ConfigOptionType_None)
        {
            config.setString("logger.colors.info", "0x90EE90");
        }
        if (config.getType("logger.colors.warning") == ConfigOptionType_None)
        {
            config.setString("logger.colors.warning", "0xFFD700");
        }
        if (config.getType("logger.colors.error") == ConfigOptionType_None)
        {
            config.setString("logger.colors.error", "0xFFB266");
        }
        if (config.getType("logger.colors.fatal") == ConfigOptionType_None)
        {
            config.setString("logger.colors.fatal", "0xFF7F7F");
        }
        if (config.getType("logger.timestamp_format") == ConfigOptionType_None)
        {
            config.setString("logger.timestamp_format", "");
        }
        if (config.getType("logger.log_directory") == ConfigOptionType_None)
        {
            config.setString("logger.log_directory", "logs");
        }
    }
}

void OmpLoggerComponent::free()
{
    core_->printLn("[omp-logger] Logger releasing...");
    for (auto logger : pool_)
    {
        std::FILE* file = logger->getFile();
        if (file != nullptr)
        {
            fclose(file);
        }
    }

    if (serverLoggerFile_ != nullptr)
    {
        fclose(serverLoggerFile_);
        serverLoggerFile_ = nullptr;
    }

    delete this;
    core_->printLn("[omp-logger] Logger released.");
}

void OmpLoggerComponent::reset()
{
}

void OmpLoggerComponent::onAmxLoad(IPawnScript& script)
{
    AMX* amx = script.GetAMX();
    pawn_natives::AmxLoad(amx);
    debugRegisterAMX(amx);
    amxToPawnScript_.emplace(amx, &script);
}

void OmpLoggerComponent::onAmxUnload(IPawnScript& script)
{
    AMX* amx = script.GetAMX();
    debugEraseAMX(amx);
    amxToPawnScript_.erase(amx);
}

OmpLoggerComponent::~OmpLoggerComponent()
{
    if (pawn_)
    {
        pawn_->getEventDispatcher().removeEventHandler(this);
    }
    if (core_)
    {
        core_->getEventDispatcher().removeEventHandler(this);
    }
}

// API - Config
fmt::rgb OmpLoggerComponent::getLogLevelColor(OmpLogger::ELogLevel level)
{
    auto it = logLevelColors_.find(level);
    return it == logLevelColors_.end() ? fmt::color::white : it->second;
}

bool OmpLoggerComponent::IsLogLevelNameCapitalized()
{
    return isLogLevelNameCapitalized_;
}

bool OmpLoggerComponent::IsLoggingWithSource()
{
    return isLoggingWithSource_;
}

bool OmpLoggerComponent::IsEnableSourceForAll()
{
    return isEnableSourceForAll_;
}

String OmpLoggerComponent::getLogTimestampFormat()
{
    return logTimestampFormat_;
}

std::FILE* OmpLoggerComponent::getServerLoggingFile()
{
    return serverLoggerFile_;
}

// API - Debugger
void OmpLoggerComponent::debugRegisterAMX(AMX* amx)
{
    DebugManager::Get()->RegisterAMX(amx);
}

void OmpLoggerComponent::debugEraseAMX(AMX* amx)
{
    DebugManager::Get()->EraseAMX(amx);
}

bool OmpLoggerComponent::debugInitData(const char* filepath)
{
    return DebugManager::Get()->InitDebugData(filepath);
}

bool OmpLoggerComponent::debugGetFunctionCall(AMX* const amx, ucell address, AmxFuncCallInfo& dest)
{
    return DebugManager::Get()->GetFunctionCall(amx, address, dest);
}
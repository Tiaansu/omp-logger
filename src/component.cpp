#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>
#include <fmt/format.h>
#include "component.hpp"
#include "helpers/utils.hpp"
#include "threaded-queue.hpp"

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

// Fetch logs
ILogsResult* OmpLoggerComponent::initLogsResult(std::vector<std::string> logs)
{
    return logsResults_.emplace(logs);
}

bool OmpLoggerComponent::deleteLogsResult(ILogsResult* result)
{
    int id = static_cast<ILogsResult*>(result)->getID();
    logsResults_.release(id, false);
    return true;
}

ILogsResult* OmpLoggerComponent::getLogsResult(int id)
{
    return logsResults_.get(id);
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
    isLogLevelNameCapitalized_ = (config.getBool(LOGGER_CONFIG_KEY_IS_LOG_LEVEL_UPPERCASE)) ? (*config.getBool(LOGGER_CONFIG_KEY_IS_LOG_LEVEL_UPPERCASE)) : false;
    isLoggingWithSource_ = (config.getBool(LOGGER_CONFIG_KEY_DISPLAY_SOURCE)) ? (*config.getBool(LOGGER_CONFIG_KEY_DISPLAY_SOURCE)) : true;
    isEnableSourceForAll_ = (config.getBool(LOGGER_CONFIG_KEY_ENABLE_SOURCE_FOR_ALL_LEVEL)) ? (*config.getBool(LOGGER_CONFIG_KEY_ENABLE_SOURCE_FOR_ALL_LEVEL)) : false;
    isTimestampColorized_ = (config.getBool(LOGGER_CONFIG_KEY_COLORIZED_TIMESTAMP)) ? (*config.getBool(LOGGER_CONFIG_KEY_COLORIZED_TIMESTAMP)) : false;
    logTimestampFormat_ = String(config.getString(LOGGER_CONFIG_KEY_TIMESTAMP_FORMAT));
    logDirectoryPath_ = String(config.getString(LOGGER_CONFIG_KEY_LOG_DIRECTORY));
    core_->printLn("hello! %s=%s", LOGGER_CONFIG_KEY_LOG_DIRECTORY, logDirectoryPath_.c_str());

    serverLoggerFile_ = ::fopen(String(config.getString("logging.file")).c_str(), "a");

    logLevelColors_[OmpLogger::ELogLevel::Debug] = helpers::GetLogLevelColorFromConfig(LOGGER_CONFIG_KEY_COLOR_DEBUG);
    logLevelColors_[OmpLogger::ELogLevel::Info] = helpers::GetLogLevelColorFromConfig(LOGGER_CONFIG_KEY_COLOR_INFO);
    logLevelColors_[OmpLogger::ELogLevel::Warning] = helpers::GetLogLevelColorFromConfig(LOGGER_CONFIG_KEY_COLOR_WARNING);
    logLevelColors_[OmpLogger::ELogLevel::Error] = helpers::GetLogLevelColorFromConfig(LOGGER_CONFIG_KEY_COLOR_ERROR);
    logLevelColors_[OmpLogger::ELogLevel::Fatal] = helpers::GetLogLevelColorFromConfig(LOGGER_CONFIG_KEY_COLOR_FATAL);

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
        for (const auto& [key, value] : config_)
        {
            if (value.type() == typeid(char const*))
            {
                config.setString(key, std::any_cast<const char*>(value));
            }
            else if (value.type() == typeid(bool))
            {
                config.setBool(key, std::any_cast<bool>(value));
            }
        }
    }
    else
    {
        for (const auto& [key, value] : config_)
        {
            if (config.getType(key) == ConfigOptionType_None)
            {
                if (value.type() == typeid(char const*))
                {
                    config.setString(key, std::any_cast<const char*>(value));
                }
                else if (value.type() == typeid(bool))
                {
                    config.setBool(key, std::any_cast<bool>(value));
                }
            }
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

    ThreadedQueue::Get()->Destroy();

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
}

void OmpLoggerComponent::onAmxUnload(IPawnScript& script)
{
    AMX* amx = script.GetAMX();
    debugEraseAMX(amx);
}

void OmpLoggerComponent::onTick(Microseconds elapsed, TimePoint now)
{
    ThreadedQueue::Get()->Process();
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

bool OmpLoggerComponent::IsTimestampColorized()
{
    return isTimestampColorized_;
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
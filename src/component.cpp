#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>
#include <fmt/format.h>
#include "component.hpp"
#include "helpers/utils.hpp"
#include "threaded-queue.hpp"

// API - Logger
IOmpLogger* OmpLoggerComponent::createLogger(StringView name, int32_t color, OmpLogger::LogLevel level, bool isPlugin)
{
    String path = name.data();
    std::replace(path.begin(), path.end(), '/', '_');

    auto filepath = fmt::format("{}/{}{}.log", logDirectoryPath_, isPlugin ? "plugins/" : "", name);
    
    bool ret = helpers::CreatePathRecursively(filepath);

    if (!ret)
    {
        core_->logLn(LogLevel::Error, "Failed to create log directory for logger %.*s", name.length(), name.data());
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
        core_->logLn(LogLevel::Error, "Failed to open log file \"%s\" for logger %.*s", filepath.c_str(), name.length(), name.data());
        return nullptr;
    }

    return pool_.emplace(name, color, level, file);
}

bool OmpLoggerComponent::destroyLogger(IOmpLogger* logger)
{
    int id = static_cast<IOmpLogger*>(logger)->getID();
    pool_.release(id, false);
    return true;
}

// API - Results
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

    isLogLevelNameCapitalized_ = (config.getBool(make_string(LoggerConfig::log_level_capitalized))) ? (*config.getBool(make_string(LoggerConfig::log_level_capitalized))) : false;
    isLoggingWithSource_ = (config.getBool(make_string(LoggerConfig::display_source))) ? (*config.getBool(make_string(LoggerConfig::display_source))) : true;
    isEnableSourceForAll_ = (config.getBool(make_string(LoggerConfig::enable_source_for_all_levels))) ? (*config.getBool(make_string(LoggerConfig::enable_source_for_all_levels))) : false;
    logTimestampFormat_ = String(config.getString(make_string(LoggerConfig::timestamp_format)));
    logDirectoryPath_ = String(config.getString(make_string(LoggerConfig::log_directory)));
    logFormat_ = String(config.getString(make_string(LoggerConfig::log_format)));
    
    serverLoggerFile_ = ::fopen(String(config.getString("logging.file")).c_str(), "a");
    
    isTimestampColorized_ = (config.getBool(make_string(LoggerConfig::Color::enabled_timestamp))) ? (*config.getBool(make_string(LoggerConfig::Color::enabled_timestamp))) : false;
    isLogLevelColorized_ = (config.getBool(make_string(LoggerConfig::Color::enabled_log_level))) ? (*config.getBool(make_string(LoggerConfig::Color::enabled_log_level))) : false;
    isLogNameColorized_ = (config.getBool(make_string(LoggerConfig::Color::enabled_name))) ? (*config.getBool(make_string(LoggerConfig::Color::enabled_name))) : false;
    logLevelColors_[OmpLogger::LogLevel::Debug] = helpers::GetLogLevelColorFromConfig(make_string(LoggerConfig::Color::Value::debug));
    logLevelColors_[OmpLogger::LogLevel::Info] = helpers::GetLogLevelColorFromConfig(make_string(LoggerConfig::Color::Value::info));
    logLevelColors_[OmpLogger::LogLevel::Warning] = helpers::GetLogLevelColorFromConfig(make_string(LoggerConfig::Color::Value::warning));
    logLevelColors_[OmpLogger::LogLevel::Error] = helpers::GetLogLevelColorFromConfig(make_string(LoggerConfig::Color::Value::error));
    logLevelColors_[OmpLogger::LogLevel::Fatal] = helpers::GetLogLevelColorFromConfig(make_string(LoggerConfig::Color::Value::fatal));
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
    // for (auto logger : pool_)
    // {
    //     std::FILE* file = logger->getFile();
    //     if (file != nullptr)
    //     {
    //         fclose(file);
    //     }
    // }

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
    pawn_natives::AmxLoad(script.GetAMX());
}

void OmpLoggerComponent::onAmxUnload(IPawnScript& script)
{
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
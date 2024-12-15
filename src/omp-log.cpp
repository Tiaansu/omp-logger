#include "omp-log.hpp"
#include "component.hpp"
#include "helpers/utils.hpp"

using namespace Impl;

int OmpLog::getID() const
{
    return poolID;
}

StringView OmpLog::getName() const
{
    return name_;
}

OmpLogger::ELogLevel OmpLog::getLogLevel() const
{
    return level_;
}

bool OmpLog::isLogLevel(OmpLogger::ELogLevel level) const
{
    return (level_ & level) == level;
}

uint32_t OmpLog::getColor() const
{
    return color_;
}

std::FILE* OmpLog::getFile() const
{
    return file_;
}

long long countTotalLines(std::FILE* file)
{
    if (file == nullptr)
    {
        return 0;
    }

    ::fseek(file, 0, SEEK_SET);

    long long lineCount = 0;
    char buffer[4096];

    while (::fgets(buffer, sizeof(buffer), file) != nullptr)
    {
        lineCount ++;
    }

    ::fseek(file, 0, SEEK_SET);
    return lineCount;
}

long long countMatchingLines(std::FILE* file, const std::string& searchTerm)
{
    if (file == nullptr)
    {
        return 0;
    }

    ::fseek(file, 0, SEEK_SET);

    long long lineCount = 0;
    char buffer[4096];
    while (std::fgets(buffer, sizeof(buffer), file) != nullptr)
    {
        std::string line(buffer);
        if (line.find(searchTerm) != std::string::npos)
        {
            lineCount ++;
        }
    }

    std::fseek(file, 0, SEEK_SET);
    return lineCount;
}

PaginatedResult OmpLog::fetchLogs(int linesPerPage, int pageStart, const std::string& searchTerm) const
{
    PaginatedResult result;

    if (file_ == nullptr)
    {
        return result;
    }

    if (!searchTerm.empty())
    {
        result.totalPages = (countMatchingLines(file_, searchTerm) + linesPerPage - 1) / linesPerPage;
    }
    else
    {
        result.totalPages = (countTotalLines(file_) + linesPerPage - 1) / linesPerPage;
    }

    if (pageStart == -1 || pageStart > result.totalPages)
    {
        pageStart = 1;
    }

    result.currentPage = pageStart;

    long long startLine = (pageStart - 1) * linesPerPage;

    if (!searchTerm.empty())
    {
        ::fseek(file_, 0, SEEK_SET);
        long long currentLine = 0;
        char buffer[4096];
        while (currentLine < startLine && ::fgets(buffer, sizeof(buffer), file_) != nullptr)
        {
            std::string line(buffer);
            if (line.find(searchTerm) != std::string::npos)
            {
                currentLine ++;
            }
        }
    }
    else
    {
        ::fseek(file_, 0, SEEK_SET);
        long long currentLine = 0;
        char buffer[4096];
        while (currentLine < startLine && ::fgets(buffer, sizeof(buffer), file_) != nullptr)
        {
            currentLine ++;
        }
    }

    char buffer[4096];
    long long linesRead = 0;
    while (linesRead < linesPerPage && ::fgets(buffer, sizeof(buffer), file_) != nullptr)
    {
        std::string line(buffer);
        line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), line.end());

        if (searchTerm.empty() || line.find(searchTerm) != std::string::npos)
        {
            result.lines.push_back(line);
            linesRead ++;
        }
    }
    return result;
}

bool OmpLog::log(AMX* amx, OmpLogger::ELogLevel level, StringView message) const
{
    return log_INTERNAL(amx, level, message);
}

bool OmpLog::log(OmpLogger::ELogLevel level, StringView message) const
{
    return log_INTERNAL(nullptr, level, message);
}

bool OmpLog::log_INTERNAL(AMX* amx, OmpLogger::ELogLevel level, StringView message) const
{
    if (!isLogLevel(level))
    {
        return false;
    }

    auto ompLogger = OmpLoggerComponent::Get();

    const std::string timestampFormat = ompLogger->getLogTimestampFormat();
    const std::time_t now = WorldTime::to_time_t(WorldTime::now());
    char timeStr[64] = { 0 };

    if (!timestampFormat.empty())
    {
        std::strftime(timeStr, sizeof timeStr, timestampFormat.c_str(), std::localtime(&now));
        fmt::print("[");
        fmt::print(fmt::fg(fmt::color::gray), timeStr);
        fmt::print("] ");
    }

    fmt::print("[");
    fmt::print(fmt::fg(getColor() == 0 ? fmt::color::light_yellow : fmt::rgb(getColor())), getName().data());
    fmt::print("] [");

    std::string logLevelName = helpers::GetLogLevelName(level);
    if (!ompLogger->IsLogLevelNameCapitalized())
    {
        std::transform(logLevelName.begin(), logLevelName.end(), logLevelName.begin(), ::toupper);
    }
    fmt::print(fmt::fg(ompLogger->getLogLevelColor(level)), logLevelName);
    fmt::print("] {:s}\n", message.data());

    return logToFile_INTERNAL(amx, level, message, timeStr, logLevelName);
}

bool OmpLog::logToFile_INTERNAL(AMX* amx, OmpLogger::ELogLevel level, StringView message, const char* timeStr, const std::string& logLevelName) const
{
    auto ompLogger = OmpLoggerComponent::Get();

    std::FILE
        *file = getFile(),
        *serverLoggerFile = ompLogger->getServerLoggingFile();

    if (file)
    {
        if (*timeStr)
        {
            fmt::print(file, "[{}] ", timeStr);
        }

        fmt::print(file, "[{}] {}", logLevelName, message.data());

        if (ompLogger->IsLoggingWithSource() && amx != nullptr)
        {
            AmxFuncCallInfo dest;
            bool ret = ompLogger->debugGetFunctionCall(amx, amx->cip, dest);
            bool shouldToDisplayToAll = ompLogger->IsEnableSourceForAll() ? true : (level == OmpLogger::ELogLevel::Warning || level == OmpLogger::ELogLevel::Error || level == OmpLogger::ELogLevel::Fatal);
            if (ret && shouldToDisplayToAll)
            {
                fmt::print(file, " ({}:{})", dest.file, dest.line);
            }
        }

        fmt::print(file, "\n");
        fflush(file);
    }

    if (serverLoggerFile)
    {
        const String coreTimestampFormat = String(ompLogger->getCore()->getConfig().getString("logging.timestamp_format"));
        char coreTimeStr[64] = { 0 };

        if (!coreTimestampFormat.empty())
        {
            std::time_t now = WorldTime::to_time_t(WorldTime::now());
            std::strftime(coreTimeStr, sizeof coreTimeStr, coreTimestampFormat.c_str(), std::localtime(&now));
            fmt::print(serverLoggerFile, "{} ", coreTimeStr);
        }
        fmt::print(serverLoggerFile, "[{}] [{}] {}\n", helpers::GetLogLevelName(level), getName().data(), message.data());
        fflush(serverLoggerFile);
    }
    return true;
}

OmpLog::OmpLog(StringView name, uint32_t color, OmpLogger::ELogLevel level, std::FILE* file)
    : level_(level)
    , name_(name)
    , color_(color)
    , file_(file)
{
}
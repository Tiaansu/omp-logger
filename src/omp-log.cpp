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

bool caseInsensitiveCompare(const std::string& str1, const std::string& str2)
{
    std::string lowerStr1 = str1;
    std::string lowerStr2 = str2;
    std::transform(lowerStr1.begin(), lowerStr1.end(), lowerStr1.begin(), ::tolower);
    std::transform(lowerStr2.begin(), lowerStr2.end(), lowerStr2.begin(), ::tolower);
    return lowerStr1.find(lowerStr2) != std::string::npos;
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

long long countMatchingLines(std::FILE* file, const std::string& searchTerm, bool caseSensitive)
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
        if (caseSensitive)
        {
            if (line.find(searchTerm) != std::string::npos)
            {
                lineCount ++;
            }
        }
        else
        {
            if (caseInsensitiveCompare(line, searchTerm))
            {
                lineCount ++;
            }
        }
    }

    std::fseek(file, 0, SEEK_SET);
    return lineCount;
}

PaginatedResult OmpLog::fetchLogs(int linesPerPage, int pageStart, const std::string& searchTerm, bool caseSensitive) const
{
    PaginatedResult result;

    if (file_ == nullptr)
    {
        return result;
    }

    if (!searchTerm.empty())
    {
        result.totalPages = (countMatchingLines(file_, searchTerm, caseSensitive) + linesPerPage - 1) / linesPerPage;
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

            if (caseSensitive)
            {
                if (line.find(searchTerm) != std::string::npos)
                {
                    currentLine ++;
                }
            }
            else
            {
                if (caseInsensitiveCompare(line, searchTerm))
                {
                    currentLine ++;
                }
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

        bool caseSensitiveResult = caseSensitive ? line.find(searchTerm) != std::string::npos : caseInsensitiveCompare(line, searchTerm);
        if (searchTerm.empty() || caseSensitiveResult)
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

    std::string formattedTimestamp;
    if (!timestampFormat.empty())
    {
        std::strftime(timeStr, sizeof timeStr, timestampFormat.c_str(), std::localtime(&now));
        formattedTimestamp = ompLogger->IsTimestampColorized() ? fmt::format(fmt::fg(fmt::color::gray), timeStr) : timeStr;
    }

    std::string formattedName = ompLogger->IsLogNameColorized()
        ? fmt::format(fmt::fg(getColor() == 0 ? fmt::color::light_yellow : fmt::rgb(getColor())), getName().data())
        : getName().data();

    std::string logLevelName = helpers::GetLogLevelName(level);
    if (!ompLogger->IsLogLevelNameCapitalized())
    {
        std::transform(logLevelName.begin(), logLevelName.end(), logLevelName.begin(), ::toupper);
    }
    std::string formattedLogLevelName = ompLogger->IsLogLevelColorized()
        ? fmt::format(fmt::fg(ompLogger->getLogLevelColor(level)), logLevelName)
        : logLevelName;

    std::string formattedLog = ompLogger->getLogFormat();

    size_t pos = 0;
    if ((pos = formattedLog.find("{{timestamp}}")) != std::string::npos)
    {
        formattedLog.replace(pos, std::string("{{timestamp}}").length(), formattedTimestamp);
    }
    if ((pos = formattedLog.find("{{name}}")) != std::string::npos)
    {
        formattedLog.replace(pos, std::string("{{name}}").length(), formattedName);
    }
    if ((pos = formattedLog.find("{{log_level}}")) != std::string::npos)
    {
        formattedLog.replace(pos, std::string("{{log_level}}").length(), formattedLogLevelName);
    }
    if ((pos = formattedLog.find("{{message}}")) != std::string::npos)
    {
        formattedLog.replace(pos, std::string("{{message}}").length(), message.data());
    }
    fmt::println("{}", formattedLog);
    return logToFile_INTERNAL(amx, level, message, timeStr, logLevelName);
}

bool OmpLog::logToFile_INTERNAL(AMX* amx, OmpLogger::ELogLevel level, StringView message, const char* timeStr, const std::string& logLevelName) const
{
    auto ompLogger = OmpLoggerComponent::Get();

    std::FILE
        *file = getFile(),
        *serverLoggerFile = ompLogger->getServerLogFile();

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
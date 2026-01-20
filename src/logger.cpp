#include "logger.hpp"
#include "component.hpp"
#include "helpers/utils.hpp"

using namespace Impl;

int Logger::getID() const
{
    return poolID;
}

StringView Logger::getName() const
{
    return name_;
}

OmpLogger::LogLevel Logger::getLogLevel() const
{
    return level_;
}

bool Logger::isLogLevel(OmpLogger::LogLevel level) const
{
    return (level_ & level) == level;
}

uint32_t Logger::getColor() const
{
    return color_;
}

std::FILE* Logger::getFile() const
{
    return file_;
}

bool Logger::log(const char* file, int line, OmpLogger::LogLevel level, StringView message) const
{
    return log__INTERNAL(file, line, level, message);
}

bool Logger::log__INTERNAL(const char* file, int line, OmpLogger::LogLevel level, StringView message) const
{
    if (!isLogLevel(level))
    {
        return false;
    }

    auto component = OmpLoggerComponent::Get();

    const std::string timestampFormat = component->getLogTimestampFormat();
    const std::time_t now = WorldTime::to_time_t(WorldTime::now());
    char timeStr[64] = { 0 };

    std::string formattedTimestamp;
    if (!timestampFormat.empty())
    {
        std::strftime(timeStr, sizeof timeStr, timestampFormat.c_str(), std::localtime(&now));
        formattedTimestamp = component->isTimestampColorized() ? fmt::format(fmt::fg(fmt::color::gray), timeStr) : timeStr;
    }

    std::string formattedName = component->isLogNameColorized()
        ? fmt::format(fmt::fg(getColor() == 0 ? fmt::color::light_yellow : fmt::rgb(getColor())), getName().data())
        : getName().data();

    std::string logLevelName = helpers::GetLogLevelName(level);
    if (!component->isLogLevelNameCapitalized())
    {
        std::transform(logLevelName.begin(), logLevelName.end(), logLevelName.begin(), ::toupper);
    }

    std::string formattedLogLevelName = component->isLogLevelColorized()
        ? fmt::format(fmt::fg(component->getLogLevelColor(level)), logLevelName)
        : logLevelName;

    std::string formattedLog = component->getLogFormat();

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
    return logToFile__INTERNAL(file, line, level, message, timeStr, logLevelName);
}

bool Logger::logToFile__INTERNAL(const char* file, int line, OmpLogger::LogLevel level, StringView message, const char* timeStr, const std::string& logLevelName) const
{
    auto component = OmpLoggerComponent::Get();

    std::FILE *file_ = getFile(), *serverLoggerFile = component->getServerLogFile();

    if (file_)
    {
        if (*timeStr)
        {
            fmt::print(file_, "[{}] ", timeStr);
        }

        fmt::print(file_, "[{}] {}", logLevelName, message.data());

        if (component->isLoggingWithSource())
        {
            bool shouldDisplayToAll = component->isEnableSourceForAll() ? true : (level == OmpLogger::LogLevel::Warning || level == OmpLogger::LogLevel::Error || level == OmpLogger::LogLevel::Fatal);
            if (shouldDisplayToAll)
            {
                fmt::print(file_, " ({}:{})", file, line);
            }
        }

        fmt::print(file_, "\n");
        fflush(file_);
    }

    if (serverLoggerFile)
    {
        const String coreTimestampFormat = String(component->getCore()->getConfig().getString("logging.timestamp_format"));
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

PaginatedResult Logger::fetchLogs(int linesPerPage, int pageStart, const std::string& searchTerm, bool caseSensitive) const
{
    PaginatedResult result;

    if (file_ == nullptr)
    {
        return result;
    }

    if (!searchTerm.empty())
    {
        result.totalPages = (int)(countMatchingLines(file_, searchTerm, caseSensitive) + linesPerPage - 1) / linesPerPage;
    }
    else
    {
        result.totalPages = (int)(countTotalLines(file_) + linesPerPage - 1) / linesPerPage;
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

Logger::Logger(StringView name, uint32_t color, OmpLogger::LogLevel level, std::FILE* file)
    : name_(name)
    , level_(level)
    , color_(color)
    , file_(file)
{
}
#pragma once

#include "omp-logger.hpp"

#include <sdk.hpp>
#include <Impl/pool_impl.hpp>

using namespace Impl;

class Logger final
    : public IOmpLogger
    , public PoolIDProvider
    , public NoCopy
{
private:
    String const name_;
    OmpLogger::LogLevel level_;
    uint32_t color_;
    std::FILE* file_;

public:
    int getID() const;

    StringView getName() const;
    
    OmpLogger::LogLevel getLogLevel() const;
    
    bool isLogLevel(OmpLogger::LogLevel level) const;
    
    uint32_t getColor() const;
    
    std::FILE* getFile() const;
    
    bool log(const char* file, int line, OmpLogger::LogLevel level, StringView message) const;
    bool log(OmpLogger::LogLevel level, StringView message) const;

    PaginatedResult fetchLogs(int linesPerPage, int pageStart, const std::string& searchTerm, bool caseSensitive) const override;
    
    Logger(StringView name, uint32_t color, OmpLogger::LogLevel level, std::FILE* file);

private:
    bool log__INTERNAL(const char* file, int line, OmpLogger::LogLevel level, StringView message) const;
    bool logToFile__INTERNAL(const char* file, int line, OmpLogger::LogLevel level, StringView message, const char* timeStr, const std::string& logLevelName) const;
};
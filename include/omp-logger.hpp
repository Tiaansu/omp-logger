#pragma once

#include <sdk.hpp>

namespace OmpLogger
{
    enum LogLevel
    {
        None = 0,
        Debug = 1,
        Info = 2,
        Warning = 4,
        Error = 8,
        Fatal = 16
    };

    inline LogLevel operator|(LogLevel a, LogLevel b)
    {
        return static_cast<LogLevel>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline LogLevel &operator|=(LogLevel &in, LogLevel val)
    {
        return in = in | val;
    }
}

struct IOmpLogger;
struct PaginatedResult;

static const UID OmpLoggerComponent_UID = UID(0xAB3BC9C4E583C8B4);
class IOmpLoggerComponent : public IComponent
{
public:
    PROVIDE_UID(OmpLoggerComponent_UID);

    virtual IOmpLogger* createLogger(StringView name, int32_t color, OmpLogger::LogLevel level, bool isPlugin) = 0;

    virtual bool destroyLogger(IOmpLogger* log) = 0;

    virtual IOmpLogger* getLogger(int id) = 0;
};

struct IOmpLogger
{
    virtual int getID() const = 0;

    virtual StringView getName() const = 0;

    virtual OmpLogger::LogLevel getLogLevel() const = 0;

    virtual bool isLogLevel(OmpLogger::LogLevel level) const = 0;

    virtual std::FILE* getFile() const = 0;

    virtual uint32_t getColor() const = 0;

    virtual PaginatedResult fetchLogs(int linesPerPage, int pageStart, const std::string& searchTerm, bool caseSensitive) const = 0;
    
    virtual bool log(const char* file, int line, OmpLogger::LogLevel level, StringView message) const = 0;

    virtual bool log(OmpLogger::LogLevel level, StringView message) const = 0;
};

class OmpLoggerManager
{
public:
    void Initialize(IOmpLoggerComponent* component)
    {
        if (component)
        {
            ompLogger_ = component;
        }
    }

    IOmpLoggerComponent* GetOmpLogger()
    {
        return ompLogger_;
    }

    static OmpLoggerManager& Get()
    {
        static OmpLoggerManager instance;
        return instance;
    }

private:
    IOmpLoggerComponent* ompLogger_ = nullptr;
};

struct PaginatedResult
{
    std::vector<std::string> lines;
    int currentPage;
    int totalPages;
};
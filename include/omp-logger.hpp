#pragma once

#include <sdk.hpp>

#include "../src/amx/amx.h"
#include "../src/amx/amxdbg.h"

struct AmxDebugInfo
{
    long line;
    const char* function;
};

namespace OmpLogger
{
    enum ELogLevel
    {
        None = 0,
        Debug = 1,
        Info = 2,
        Warning = 4,
        Error = 8,
        Fatal = 16
    };

    inline ELogLevel operator|(ELogLevel a, ELogLevel b)
    {
        return static_cast<ELogLevel>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline ELogLevel &operator|=(ELogLevel &in, ELogLevel val)
    {
        return in = in | val;
    }
}

struct IOmpLog;
struct PaginatedResult;

static const UID OmpLoggerComponent_UID = UID(0xAB3BC9C4E583C8B4);
class IOmpLoggerComponent : public IComponent
{
public:
    PROVIDE_UID(OmpLoggerComponent_UID);

    virtual IOmpLog* createLogger(StringView name, int32_t color, OmpLogger::ELogLevel level, bool isPlugin) = 0;

    virtual bool destroyLogger(IOmpLog* log) = 0;

    virtual IOmpLog* getLogger(int id) = 0;

    // Debugger
    virtual void debugRegisterAMX(AMX* amx) = 0;
    
    virtual void debugEraseAMX(AMX* amx) = 0;
};

struct IOmpLog
{
    virtual int getID() const = 0;

    virtual StringView getName() const = 0;

    virtual OmpLogger::ELogLevel getLogLevel() const = 0;

    virtual bool isLogLevel(OmpLogger::ELogLevel level) const = 0;

    virtual std::FILE* getFile() const = 0;

    virtual uint32_t getColor() const = 0;

    virtual PaginatedResult fetchLogs(int linesPerPage, int pageStart, const std::string& searchTerm, bool caseSensitive) const = 0;

    virtual bool log(AMX* amx, OmpLogger::ELogLevel level, StringView message) const = 0;
    
    virtual bool log(OmpLogger::ELogLevel level, StringView message) const = 0;
};

struct AmxFuncCallInfo
{
    long line;
    const char* file;
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
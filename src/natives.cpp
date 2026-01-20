#include <algorithm>
#include <string>
#include <vector>
#include <thread>

#include <sdk.hpp>
#include <Server/Components/Pawn/Impl/pawn_natives.hpp>

#include "component.hpp"
#include "natives.hpp"
#include "omp-logger.hpp"
#include "threaded-queue.hpp"
#include "helpers/format.hpp"

SCRIPT_API(Logger_Create, int(std::string const& name, int32_t color, OmpLogger::LogLevel level))
{
    auto component = OmpLoggerComponent::Get();
    auto ret = component->createLogger(name, static_cast<uint32_t>(color), level, false);
    return ret ? ret->getID() : -1;
}

SCRIPT_API(Logger_Destroy, bool(IOmpLogger& logger))
{
    auto component = OmpLoggerComponent::Get();
    bool ret = component->destroyLogger(&logger);
    return ret;
}

SCRIPT_API(_Logger_Log, bool(std::string const& file, int line, IOmpLogger& logger, OmpLogger::LogLevel level, cell const* format))
{
    return logger.log(file.c_str(), line, level, AmxStringFormatter(format, GetAMX(), GetParams(), 5));
}

SCRIPT_API(_Logger_Debug, bool(std::string const& file, int line, IOmpLogger& logger, cell const* format))
{
    return logger.log(file.c_str(), line, OmpLogger::LogLevel::Debug, AmxStringFormatter(format, GetAMX(), GetParams(), 4));
}

SCRIPT_API(_Logger_Info, bool(std::string const& file, int line, IOmpLogger& logger, cell const* format))
{
    return logger.log(file.c_str(), line, OmpLogger::LogLevel::Info, AmxStringFormatter(format, GetAMX(), GetParams(), 4));
}

SCRIPT_API(_Logger_Warning, bool(std::string const& file, int line, IOmpLogger& logger, cell const* format))
{
    return logger.log(file.c_str(), line, OmpLogger::LogLevel::Warning, AmxStringFormatter(format, GetAMX(), GetParams(), 4));
}

SCRIPT_API(_Logger_Error, bool(std::string const& file, int line, IOmpLogger& logger, cell const* format))
{
    return logger.log(file.c_str(), line, OmpLogger::LogLevel::Error, AmxStringFormatter(format, GetAMX(), GetParams(), 4));
}

SCRIPT_API(_Logger_Fatal, bool(std::string const& file, int line, IOmpLogger& logger, cell const* format))
{
    return logger.log(file.c_str(), line, OmpLogger::LogLevel::Fatal, AmxStringFormatter(format, GetAMX(), GetParams(), 4));
}

SCRIPT_API(Logger_FetchLogs, bool(IPlayer& player, IOmpLogger& logger, int linesPerPage, int pageStart, std::string const& callback, std::string const& searchTerm, bool caseSensitive))
{
    AMX* amx = GetAMX();

    auto func = [&player, &logger, amx, callback, searchTerm, linesPerPage, pageStart, caseSensitive]()
    {
        int funcIDX = 0;
        if (!amx_FindPublic(amx, callback.c_str(), &funcIDX))
        {
            PaginatedResult result = logger.fetchLogs(linesPerPage, pageStart, searchTerm, caseSensitive);
            ILogsResult* logsResult = OmpLoggerComponent::Get()->initLogsResult(result.lines);
            amx_Push(amx, result.totalPages);
            amx_Push(amx, result.currentPage);
            amx_Push(amx, (int)result.lines.size());
            amx_Push(amx, logsResult->getID());
            amx_Push(amx, logger.getID());
            amx_Push(amx, player.getID());
            amx_Exec(amx, NULL, funcIDX);
        }
    };
    ThreadedQueue::Get()->Dispatch(func);
    return true;
}

SCRIPT_API(Logger_GetResult, int(ILogsResult& result, int row, OutputOnlyString& logs))
{
    logs = result.getLog(row);
    return std::get<StringView>(logs).length();
}

SCRIPT_API(Logger_FreeResult, bool(ILogsResult& result))
{
    return OmpLoggerComponent::Get()->deleteLogsResult(&result);
}
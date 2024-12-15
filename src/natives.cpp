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

SCRIPT_API(Logger_Create, int(std::string const& name, int32_t color, OmpLogger::ELogLevel level))
{
    auto ompLogger = OmpLoggerComponent::Get();
    auto ret = ompLogger->createLogger(name, static_cast<uint32_t>(color), level, false);
    return ret != nullptr ? ret->getID() : -1;
}

SCRIPT_API(Logger_Destroy, bool(IOmpLog& logger))
{
    auto ompLogger = OmpLoggerComponent::Get();
    return ompLogger->destroyLogger(&logger);
}

SCRIPT_API(Logger_Log, bool(IOmpLog& logger, OmpLogger::ELogLevel level, cell const* format))
{
    AMX* amx = GetAMX();
    return logger.log(amx, level, AmxStringFormatter(format, amx, GetParams(), 3));
}

SCRIPT_API(Logger_Debug, bool(IOmpLog& logger, cell const* format))
{
    AMX* amx = GetAMX();
    return logger.log(amx, OmpLogger::ELogLevel::Debug, AmxStringFormatter(format, amx, GetParams(), 2));
}

SCRIPT_API(Logger_Info, bool(IOmpLog& logger, cell const* format))
{
    AMX* amx = GetAMX();
    return logger.log(amx, OmpLogger::ELogLevel::Info, AmxStringFormatter(format, amx, GetParams(), 2));
}

SCRIPT_API(Logger_Warning, bool(IOmpLog& logger, cell const* format))
{
    AMX* amx = GetAMX();
    return logger.log(amx, OmpLogger::ELogLevel::Warning, AmxStringFormatter(format, amx, GetParams(), 2));
}

SCRIPT_API(Logger_Error, bool(IOmpLog& logger, cell const* format))
{
    AMX* amx = GetAMX();
    return logger.log(amx, OmpLogger::ELogLevel::Error, AmxStringFormatter(format, amx, GetParams(), 2));
}

SCRIPT_API(Logger_Fatal, bool(IOmpLog& logger, cell const* format))
{
    AMX* amx = GetAMX();
    return logger.log(amx, OmpLogger::ELogLevel::Fatal, AmxStringFormatter(format, amx, GetParams(), 2));
}

SCRIPT_API(Logger_FetchLogs, bool(IPlayer& player, IOmpLog& logger, int linesPerPage, int pageStart, std::string const& callback, std::string const& searchTerm, bool caseSensitive))
{
    AMX* amx = GetAMX();

    auto func = [&player, &logger, amx, callback, searchTerm, linesPerPage, pageStart, caseSensitive]() {

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
    return 1;
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
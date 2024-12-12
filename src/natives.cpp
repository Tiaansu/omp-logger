#include <algorithm>
#include <string>

#include <sdk.hpp>
#include <Server/Components/Pawn/Impl/pawn_natives.hpp>

#include "component.hpp"
#include "natives.hpp"
#include "omp-logger.hpp"
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
#include <algorithm>
#include <string>
#include <vector>
#include <json.hpp>

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

using json = nlohmann::json;

json readFileWithPagination(std::FILE* file, int pageSize, int pageNumber, int offset)
{
    json result = json::array();

    if (file == nullptr)
    {
        return result;
    }

    long startPosition = offset + (pageNumber - 1) * pageSize;

    if (::fseek(file, startPosition, SEEK_SET) != 0)
    {
        return result;
    }

    std::string line;
    char buffer[4096];

    for (int i = 0; i < pageSize; i ++)
    {
        if (::fgets(buffer, sizeof(buffer), file) != nullptr)
        {
            line = buffer;

            line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) 
            {
                return !std::isspace(ch);
            }).base(), line.end());
            result.push_back(line);
        }
        else
        {
            break;
        }
    }

    return result;
}

SCRIPT_API(Logget_FetchLogs, bool(IOmpLog& logger, int amount, int pageStart, int offset, std::string const& callback))
{
    // AMX* amx = GetAMX();
    // int funcIDX = 0;

    // if (!amx_FindPublic(amx, callback.c_str(), &funcIDX))
    // {
    //     cell addr = 0;

    //     json pageData = readFileWithPagination(logger.getFile(), amount, pageStart, offset);
    //     amx_PushString(amx, &addr, NULL, pageData.dump(4).c_str(), NULL, NULL);
    //     amx_Exec(amx, NULL, funcIDX);
    //     amx_Release(amx, addr);
    // }

    std::FILE* file = logger.getFile();
    AMX* amx = GetAMX();
    std::string func = callback;

    OmpLoggerComponent::Get()->AddCallback([file, amx, func, amount, pageStart, offset]() {
        int funcIDX = 0;

        int ret = amx_FindPublic(amx, func.c_str(), &funcIDX);

        if (!ret)
        {
            cell addr = 0;

            json pageData = readFileWithPagination(file, amount, pageStart, offset);
            amx_PushString(amx, &addr, NULL, pageData.dump(4).c_str(), NULL, NULL);
            amx_Exec(amx, NULL, funcIDX);
            amx_Release(amx, addr);
        }
        else
        {
            OmpLoggerComponent::Get()->getCore()->printLn("FAILED: %s", func.c_str());
        }
    });
    return 1;
}
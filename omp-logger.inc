#if defined _INC_omp_logger
    #endinput
#endif
#define _INC_omp_logger

#include <open.mp>

enum E_OMP_LOGGER_LEVEL
{
    None = 0,
    Debug = 1,
    Info = 2,
    Warning = 4,
    Error = 8,
    Fatal = 16,

    All = Debug | Info | Warning | Error | Fatal
};

#if !defined DEFAULT_OMP_LOGGER_LEVEL
    #define DEFAULT_OMP_LOGGER_LEVEL       (Info | Warning | Error)
#endif

#define INVALID_LOGGER_ID               (Logger:-1)

native Logger:Logger_Create(const name[], color = 0, E_OMP_LOGGER_LEVEL:level = DEFAULT_OMP_LOGGER_LEVEL);
native bool:Logger_Destroy(Logger:id);

native bool:_Logger_Log(const file[], const line, Logger:id, E_OMP_LOGGER_LEVEL:level, const format[], OPEN_MP_TAGS:...);
forward Logger_Log(Logger:id, E_OMP_LOGGER_LEVEL:level, const format[], OPEN_MP_TAGS:...);
#define Logger_Log(         _Logger_Log(__file,__line,

native bool:_Logger_Debug(const file[], const line, Logger:id, const format[], OPEN_MP_TAGS:...);
forward Logger_Debug(Logger:id, const format[], OPEN_MP_TAGS:...);
#define Logger_Debug(         _Logger_Debug(__file,__line,

native bool:_Logger_Info(const file[], const line, Logger:id, const format[], OPEN_MP_TAGS:...);
forward Logger_Info(Logger:id, const format[], OPEN_MP_TAGS:...);
#define Logger_Info(         _Logger_Info(__file,__line,

native bool:_Logger_Warning(const file[], const line, Logger:id, const format[], OPEN_MP_TAGS:...);
forward Logger_Warning(Logger:id, const format[], OPEN_MP_TAGS:...);
#define Logger_Warning(         _Logger_Warning(__file,__line,

native bool:_Logger_Error(const file[], const line, Logger:id, const format[], OPEN_MP_TAGS:...);
forward Logger_Error(Logger:id, const format[], OPEN_MP_TAGS:...);
#define Logger_Error(         _Logger_Error(__file,__line,

native bool:_Logger_Fatal(const file[], const line, Logger:id, const format[], OPEN_MP_TAGS:...);
forward Logger_Fatal(Logger:id, const format[], OPEN_MP_TAGS:...);
#define Logger_Fatal(         _Logger_Fatal(__file,__line,

// Logs result
native Logger_FetchLogs(playerid, Logger:logger, amount, pageStart, const callback[], const search[] = "", bool:caseSensitive = true);
native Logger_GetResult(LogsResult:result, row, logs[], size = sizeof logs);
native Logger_FreeResult(LogsResult:result);

// Cleanup results once they go out of scope
stock operator~(const LogsResult:result[], len)
{
    for (new i = 0; i < len; ++ i)
    {
        Logger_FreeResult(result[i]);
    }
}

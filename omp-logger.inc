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

#if !defined DEFAULT_OMP_LOG_LEVEL
    #define DEFAULT_OMP_LOG_LEVEL       (Info | Warning | Error)
#endif

#define INVALID_LOGGER_ID               (Logger:-1)

native Logger:Logger_Create(const name[], color = 0, E_OMP_LOGGER_LEVEL:level = DEFAULT_OMP_LOG_LEVEL);
native bool:Logger_Destroy(Logger:id);
native bool:Logger_Log(Logger:id, E_OMP_LOGGER_LEVEL:level, const format[], OPEN_MP_TAGS:...);
native bool:Logger_Debug(Logger:id, const format[], OPEN_MP_TAGS:...);
native bool:Logger_Info(Logger:id, const format[], OPEN_MP_TAGS:...);
native bool:Logger_Warning(Logger:id, const format[], OPEN_MP_TAGS:...);
native bool:Logger_Error(Logger:id, const format[], OPEN_MP_TAGS:...);
native bool:Logger_Fatal(Logger:id, const format[], OPEN_MP_TAGS:...);

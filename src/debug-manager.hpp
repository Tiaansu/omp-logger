#pragma once
#include <unordered_map>

#include "amx/amx.h"
#include "amx/amxdbg.h"
#include "omp-logger.hpp"

class DebugManager
{
private:
    std::unordered_map<AMX_HEADER*, AMX_DBG*> availableDebugInfo_;
    std::unordered_map<AMX*, AMX_DBG*> amxDebugMap_;

    inline static DebugManager* instance_ = nullptr;

public:
    DebugManager();
    ~DebugManager();

    static DebugManager* Get()
    {
        if (instance_ == nullptr)
        {
            instance_ = new DebugManager();
        }
        return instance_;
    }

    void RegisterAMX(AMX* amx);

    void EraseAMX(AMX* amx);

    bool InitDebugData(const char* filepath);

    bool GetFunctionCall(AMX* const amx, ucell address, AmxFuncCallInfo& dest);
};
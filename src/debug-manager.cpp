#include <cassert>

#include "debug-manager.hpp"
#include "omp-logger.hpp"

DebugManager::DebugManager() { }

DebugManager::~DebugManager()
{
    for (auto& dbg : availableDebugInfo_)
    {
        delete dbg.first;
        delete dbg.second;
    }
}

void DebugManager::RegisterAMX(AMX* amx)
{
    if (amxDebugMap_.find(amx) != amxDebugMap_.end())
    {
        return;
    }

    for (auto& d : availableDebugInfo_)

    {
        if (memcmp(d.first, amx->base, sizeof(AMX_HEADER)) == 0)
        {
            amxDebugMap_.emplace(amx, d.second);
            break;
        }
    }
}

void DebugManager::EraseAMX(AMX* amx)
{
    amxDebugMap_.erase(amx);
}

bool DebugManager::InitDebugData(const char* filepath)
{
    std::FILE* file = std::fopen(filepath, "rb");
    if (file == nullptr)
    {
        return false;
    }

    AMX_HEADER hdr;
    fread(&hdr, sizeof(hdr), 1, file);

    AMX_DBG dbg;
    int error = dbg_LoadInfo(&dbg, file);

    fclose(file);

    if (error == AMX_ERR_NONE)
    {
        availableDebugInfo_.emplace(new AMX_HEADER(hdr), new AMX_DBG(dbg));
    }
    return error == AMX_ERR_NONE;
}

bool DebugManager::GetFunctionCall(AMX* const amx, ucell address, AmxFuncCallInfo& dest)
{
    auto it = amxDebugMap_.find(amx);
    if (it == amxDebugMap_.end())
    {
        return false;
    }

    AMX_DBG* dbg = it->second;

    if (dbg_LookupLine(dbg, address, &(dest.line)) != AMX_ERR_NONE)
    {
        dest.line = -1;
        return false;
    }
    dest.line ++;

    if (dbg_LookupFile(dbg, address, &(dest.file)) != AMX_ERR_NONE)
    {
        dest.file = "<unknown>";
        return false;
    }
    return true;
}
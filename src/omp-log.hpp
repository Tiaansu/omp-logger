#pragma once

#include <sdk.hpp>
#include <Impl/pool_impl.hpp>

#include "omp-logger.hpp"

using namespace Impl;

class OmpLog final
    : public IOmpLog
    , public PoolIDProvider
    , public NoCopy
{
private:
    String const name_;

    OmpLogger::ELogLevel level_;

    uint32_t color_;

    std::FILE* file_;

public:
    int getID() const override;

    StringView getName() const override;

    OmpLogger::ELogLevel getLogLevel() const override;

    bool isLogLevel(OmpLogger::ELogLevel level) const override;

    uint32_t getColor() const override;

    std::FILE* getFile() const override;

    bool log(AMX* amx, OmpLogger::ELogLevel level, StringView message) const override;

    bool log(OmpLogger::ELogLevel level, StringView message) const override;

    OmpLog(StringView name, uint32_t color, OmpLogger::ELogLevel level, std::FILE* file);    

private:
    bool log_INTERNAL(AMX* amx, OmpLogger::ELogLevel level, StringView message) const;

    bool logToFile_INTERNAL(AMX* amx, OmpLogger::ELogLevel level, StringView message, const char* timeStr, const std::string& logLevelName) const;
};
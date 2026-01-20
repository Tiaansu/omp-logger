#pragma once

#include <sdk.hpp>
#include <Impl/pool_impl.hpp>

using namespace Impl;

struct ILogsResult
{
    virtual int getID() const = 0;
    
    virtual std::string getLog(int row) const = 0;
};

class LogsResult final
    : public ILogsResult
    , public PoolIDProvider
    , public NoCopy
{
private:
    std::vector<std::string> logs_;

public:
    int getID() const override;

    std::string getLog(int row) const override;

    LogsResult(std::vector<std::string> logs);
};
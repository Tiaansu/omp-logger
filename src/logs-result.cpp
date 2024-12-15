#include "logs-result.hpp"
#include "component.hpp"

using namespace Impl;

int LogsResult::getID() const
{
    return poolID;
}

std::string LogsResult::getLog(int row) const
{
    return logs_.at(row);
}

LogsResult::LogsResult(std::vector<std::string> logs)
    : logs_(std::move(logs))
{
}
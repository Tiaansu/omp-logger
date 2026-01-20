#pragma once

#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include "component.hpp"
#include <iostream>

namespace pawn_natives
{
    template <>
    struct ParamLookup<IOmpLogger>
    {
        static IOmpLogger* Val(cell ref) noexcept
        {
            if (auto pool = OmpLoggerComponent::Get())
            {
                return pool->getLogger(ref);
            }
            return nullptr;
        }
    };

    template <>
    class ParamCast<IOmpLogger*>
    {
    public:
        ParamCast(AMX* amx, cell* params, int idx) noexcept
        {
            value_ = ParamLookup<IOmpLogger>::Val(params[idx]);
        }

        ~ParamCast()
        {
        }

        ParamCast(ParamCast<IOmpLogger*> const&) = delete;
        ParamCast(ParamCast<IOmpLogger*>&&) = delete;

        operator IOmpLogger*()
        {
            return value_;
        }

        bool Error() const
        {
            return false;
        }

        static constexpr int Size = 1;

    private:
        IOmpLogger* value_;
    };

    template <>
    class ParamCast<IOmpLogger&>
    {
    public:
        ParamCast(AMX* amx, cell* params, int idx)
        {
            value_ = ParamLookup<IOmpLogger>::Val(params[idx]);
            if (value_ == nullptr)
            {
                OmpLoggerComponent::Get()->getCore()->logLn(LogLevel::Warning, "Invalid logger id %i", static_cast<int>(params[idx]));
                error_ = true;
            }
        }

        ~ParamCast()
        {
        }

        ParamCast(ParamCast<IOmpLogger&> const&) = delete;
        ParamCast(ParamCast<IOmpLogger&>&&) = delete;

        operator IOmpLogger&()
        {
            return *value_;
        }

        bool Error() const
        {
            return error_;
        }

        static constexpr int Size = 1;

    private:
        IOmpLogger* value_;
        bool error_ = false;
    };

    template <>
    class ParamCast<const IOmpLogger&>
    {
    public:
        ParamCast(AMX*, cell*, int) = delete;
        ParamCast() = delete;
    };

    // Logs result
    template <>
    struct ParamLookup<ILogsResult>
    {
        static ILogsResult* Val(cell ref) noexcept
        {
            if (auto pool = OmpLoggerComponent::Get())
            {
                return pool->getLogsResult(ref);
            }
            return nullptr;
        }
    };

    template <>
    class ParamCast<ILogsResult*>
    {
    public:
        ParamCast(AMX* amx, cell* params, int idx) noexcept
        {
            value_ = ParamLookup<ILogsResult>::Val(params[idx]);
        }

        ~ParamCast()
        {
        }

        ParamCast(ParamCast<ILogsResult*> const&) = delete;
        ParamCast(ParamCast<ILogsResult*>&&) = delete;

        operator ILogsResult*()
        {
            return value_;
        }

        bool Error() const
        {
            return false;
        }

        static constexpr int Size = 1;

    private:
        ILogsResult* value_;
    };

    template <>
    class ParamCast<ILogsResult&>
    {
    public:
        ParamCast(AMX* amx, cell* params, int idx)
        {
            value_ = ParamLookup<ILogsResult>::Val(params[idx]);
            if (value_ == nullptr)
            {
                OmpLoggerComponent::Get()->getCore()->logLn(LogLevel::Warning, "Invalid logs result id %i", static_cast<int>(params[idx]));
                error_ = true;
            }
        }

        ~ParamCast()
        {
        }

        ParamCast(ParamCast<ILogsResult&> const&) = delete;
        ParamCast(ParamCast<ILogsResult&>&&) = delete;

        operator ILogsResult&()
        {
            return *value_;
        }

        bool Error() const
        {
            return error_;
        }

        static constexpr int Size = 1;

    private:
        ILogsResult* value_;
        bool error_ = false;
    };

    template <>
    class ParamCast<const ILogsResult&>
    {
    public:
        ParamCast(AMX*, cell*, int) = delete;
        ParamCast() = delete;
    };
}
#pragma once

#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include "component.hpp"
#include <iostream>

namespace pawn_natives
{
    template <>
    struct ParamLookup<IOmpLog>
    {
        static IOmpLog* Val(cell ref) noexcept
        {
            if (auto pool = OmpLoggerComponent::Get())
            {
                return pool->getLogger(ref);
            }
            return nullptr;
        }
    };

    template <>
    class ParamCast<IOmpLog*>
    {
    public:
        ParamCast(AMX* amx, cell* params, int idx) noexcept
        {
            value_ = ParamLookup<IOmpLog>::Val(params[idx]);
        }

        ~ParamCast()
        {
        }

        ParamCast(ParamCast<IOmpLog*> const&) = delete;
        ParamCast(ParamCast<IOmpLog*>&&) = delete;

        operator IOmpLog*()
        {
            return value_;
        }

        bool Error() const
        {
            return false;
        }

        static constexpr int Size = 1;

    private:
        IOmpLog* value_;
    };

    template <>
    class ParamCast<IOmpLog&>
    {
    public:
        ParamCast(AMX* amx, cell* params, int idx)
        {
            value_ = ParamLookup<IOmpLog>::Val(params[idx]);
            if (value_ == nullptr)
            {
                AmxFuncCallInfo dest;
                if (!DebugManager::Get()->GetFunctionCall(amx, amx->cip, dest))
                {
                    OmpLoggerComponent::Get()->getCore()->logLn(LogLevel::Warning, "Invalid logger id %i", static_cast<int>(params[idx]));
                }
                else
                {
                    OmpLoggerComponent::Get()->getCore()->logLn(LogLevel::Warning, "Invalid logger id %i (%s:%i)",  static_cast<int>(params[idx]), dest.file, static_cast<int>(dest.line));
                }
                error_ = true;
            }
        }

        ~ParamCast()
        {
        }

        ParamCast(ParamCast<IOmpLog&> const&) = delete;
        ParamCast(ParamCast<IOmpLog&>&&) = delete;

        operator IOmpLog&()
        {
            return *value_;
        }

        bool Error() const
        {
            return error_;
        }

        static constexpr int Size = 1;

    private:
        IOmpLog* value_;
        bool error_ = false;
    };

    template <>
    class ParamCast<const IOmpLog&>
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
                AmxFuncCallInfo dest;
                if (!DebugManager::Get()->GetFunctionCall(amx, amx->cip, dest))
                {
                    OmpLoggerComponent::Get()->getCore()->logLn(LogLevel::Warning, "Invalid logs result id %i", static_cast<int>(params[idx]));
                }
                else
                {
                    OmpLoggerComponent::Get()->getCore()->logLn(LogLevel::Warning, "Invalid logs result id %i (%s:%i)",  static_cast<int>(params[idx]), dest.file, static_cast<int>(dest.line));
                }
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
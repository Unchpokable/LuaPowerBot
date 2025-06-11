#pragma once

#define DECLARE_WORKER_MODULE_INTERFACE \
    void initialize(); \
                       \
    void execute(Task task, Callback on_success = default_success_handler, Callback on_fail = default_fail_handler)\

#include <functional>

#include "error.hxx"
#include "expected.hxx"
#include "logdef.hxx"

namespace workers {

enum WorkerResult : std::uint32_t
{
    Worker_Ok,
    Worker_TaskThrownException,
    Worker_TaskReturnedError,
    Worker_UnknownFailure,
};

struct ExecResult
{
    std::uint32_t task_uid;
    WorkerResult code;
    std::optional<std::exception> exception;
    ExpectedErr<> user_error;
};

using Task = std::function<ExpectedErr<>()>;
using Callback = std::function<void(const ExecResult&)>;

inline void default_fail_handler(const ExecResult& error)
{
    luabot_logErr("Background worker failed task #{}. Code: {}, Message: {}", error.task_uid, static_cast<std::uint32_t>(error.code));
}

inline void default_success_handler(const ExecResult& result)
{
    luabot_logInfo("Background worker completed task #{}", result.task_uid);
}

}



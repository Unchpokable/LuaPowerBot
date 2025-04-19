#pragma once

#include <string>
#include <vector>

#include <sol/sol.hpp>

namespace lua::api::types::ui {

struct InlineKeyboardButton
{
    std::string text;
    std::string callbackData;
};

struct InlineKeyboard
{
    using ButtonRow = std::vector<InlineKeyboardButton>;

    std::vector<ButtonRow> buttons;
};

struct ReplyKeyboardButton
{
    std::string text;
};

struct ReplyKeyboard
{
    using ButtonRow = std::vector<ReplyKeyboardButton>;

    std::vector<ButtonRow> buttons;
};

}

namespace lua::api::types::routines {

enum class CoroutineStep
{
    Step,
    Done
};

enum class CoroutinePolicy
{
    EnqueueNext,
    RejectNext,
    SyncExecute
};

struct Coroutine
{
    sol::coroutine coroutine;
    CoroutinePolicy policy;

    template<typename ...Args>
    sol::protected_function_result operator()(Args&&... args);
};

template <typename ...Args>
sol::protected_function_result Coroutine::operator()(Args&&... args)
{
    return coroutine(std::forward<Args>(args...));
}

}

namespace lua::api::types {

void register_types(sol::state_view state);

}


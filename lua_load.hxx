#pragma once

#include <filesystem>

#include <sol/sol.hpp>

#include "expected.hxx"

#include "error.hxx"

#include "zip2memvfs.hxx"

template<typename T>
concept SolBasicType =
    std::is_same_v<T, sol::object> ||
    std::is_same_v<T, sol::table> ||
    std::is_same_v<T, sol::function> ||
    std::is_same_v<T, sol::userdata> ||
    std::is_same_v<T, sol::lightuserdata> ||
    std::is_same_v<T, sol::thread> ||
    std::is_same_v<T, sol::coroutine> ||
    std::is_same_v<T, sol::environment> ||
    std::is_same_v<T, sol::variadic_args> ||
    std::is_same_v<T, sol::variadic_results> ||
    std::is_same_v<T, sol::protected_function> ||
    std::is_same_v<T, sol::protected_function_result>;

using BytecodeMap = std::unordered_map<std::string, std::string>;

namespace lua {

class CommandBox final
{
public:
    CommandBox(sol::state&& state, std::string prefix);

    sol::global_table& commands();

private:
    sol::state _state;
    std::string _prefix;
};

Expected<BytecodeMap, errors::Error> load_bytecode_map(const std::string& folder);
Expected<BytecodeMap, errors::Error> load_bytecode_map(const files::IFileSystem& zip_fs);

Expected<CommandBox*, errors::Error> make_state_from_cached_bytecode(const BytecodeMap& bytecode_map);

Expected<CommandBox*, errors::Error> load_scripts(const std::string& folder);

}

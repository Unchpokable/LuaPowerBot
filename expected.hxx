#pragma once

#include <cassert>
#include <concepts>
#include <type_traits>
#include <utility>
#include <variant>

#include "globals.hxx"

__noreturn inline void unreachable()
{
    assert(false);
}

template<typename T, typename Err = errors::Error>
class Expected
{
public:
    using value_type = T;
    using error_type = Err;

    Expected(const value_type& value) requires std::copy_constructible<T>;
    Expected(value_type&& value) requires std::move_constructible<T>;

    Expected(const error_type& err) requires std::copy_constructible<Err>;
    Expected(error_type&& err) requires std::move_constructible<Err>;

    bool has_value() const noexcept;
    explicit operator bool() const noexcept;

    const value_type& value() const;
    const error_type& error() const;

    value_type& value();
    error_type& error();

private:
    std::variant<value_type, error_type> _data;
};

template <typename T, typename Err>
Expected<T, Err>::Expected(const value_type& value) requires std::copy_constructible<T> : _data(value) {}

template <typename T, typename Err>
Expected<T, Err>::Expected(value_type&& value) requires std::move_constructible<T> : _data(std::move(value)) {}

template <typename T, typename Err>
Expected<T, Err>::Expected(const error_type& err) requires std::copy_constructible<Err> : _data(err) {}

template <typename T, typename Err>
Expected<T, Err>::Expected(error_type&& err) requires std::move_constructible<Err> : _data(std::move(err)) {}

template <typename T, typename Err>
bool Expected<T, Err>::has_value() const noexcept
{
    return _data.index() == 0;
}

template <typename T, typename Err>
Expected<T, Err>::operator bool() const noexcept
{
    return has_value();
}

template <typename T, typename Err>
const typename Expected<T, Err>::value_type& Expected<T, Err>::value() const
{
    if(!has_value()) 
        unreachable();
    return std::get<0>(_data);
}

template <typename T, typename Err>
const typename Expected<T, Err>::error_type& Expected<T, Err>::error() const
{
    if(has_value())
        unreachable();
    return std::get<1>(_data);
}

template <typename T, typename Err>
typename Expected<T, Err>::value_type& Expected<T, Err>::value()
{
    if(!has_value()) 
        unreachable();
    return std::get<0>(_data);
}

template <typename T, typename Err>
typename Expected<T, Err>::error_type& Expected<T, Err>::error()
{
    if(has_value()) 
        unreachable();
    return std::get<1>(_data);
}

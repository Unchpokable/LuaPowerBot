#pragma once

#include <cassert>
#include <concepts>
#include <type_traits>
#include <utility>
#include <variant>

#define no_return [[noreturn]]

no_return inline void unreachable()
{
    assert(false);
}

template<typename T, typename Err>
class Expected
{
public:
    using value_type = T;
    using error_type = Err;

    Expected(const value_type& value) requires std::copy_constructible<T>;
    Expected(value_type&& value) requires std::move_constructible<T>;

    Expected(const error_type& err) requires std::copy_constructible<Err>;
    Expected(error_type&& err) requires std::move_constructible<Err>;

    bool hasValue() const noexcept;
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
bool Expected<T, Err>::hasValue() const noexcept
{
    return _data.index() == 0;
}

template <typename T, typename Err>
Expected<T, Err>::operator bool() const noexcept
{
    return hasValue();
}

template <typename T, typename Err>
const typename Expected<T, Err>::value_type& Expected<T, Err>::value() const
{
    if(!hasValue()) 
        unreachable();
    return std::get<0>(_data);
}

template <typename T, typename Err>
const typename Expected<T, Err>::error_type& Expected<T, Err>::error() const
{
    if(hasValue())
        unreachable();
    return std::get<1>(_data);
}

template <typename T, typename Err>
typename Expected<T, Err>::value_type& Expected<T, Err>::value()
{
    if(!hasValue()) 
        unreachable();
    return std::get<0>(_data);
}

template <typename T, typename Err>
typename Expected<T, Err>::error_type& Expected<T, Err>::error()
{
    if(hasValue()) 
        unreachable();
    return std::get<1>(_data);
}

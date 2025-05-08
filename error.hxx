#pragma once
#include <string>

#include "globals.hxx"

namespace errors {

template<typename T>
concept StringCompatible = requires(T t) {
    { std::string { t } };
};

template<typename T>
concept ErrorCompatible =
    std::derived_from<T, std::exception> &&
    std::constructible_from<T, const char*>;

enum FileSystemResult
{
    OK,
    FileTooBig,
    InvalidSize,
    UnableToRead,
    UnableToWrite,
    UnableToOpen
};

class Error final
{
public:
    template<StringCompatible String>
    Error(String&& msg);

    template<ErrorCompatible Err>
    __noreturn void throwError() const;

    std::string message() const;

private:
    std::string _message;
};

template<StringCompatible String>
Error::Error(String&& msg) : _message(std::forward<String>(msg)) {}

template <ErrorCompatible Err>
void Error::throwError() const {
    throw Err(_message.c_str());
}
}



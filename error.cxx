#include "error.hxx"

std::string errors::Error::message() const {
    return _message;
}

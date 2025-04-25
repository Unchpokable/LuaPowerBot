#pragma once

#ifdef _WIN32

#include "error.hxx"
#include "expected.hxx"

namespace security {

Expected<std::vector<std::uint8_t>, errors::Error> dpapi_encrypt(std::vector<std::uint8_t>& buffer);
Expected<std::vector<std::uint8_t>, errors::Error> dpapi_decrypt(std::vector<std::uint8_t>& buffer);
Expected<std::vector<std::uint8_t>, errors::Error> dpapi_encrypt_string(std::string_view string);
Expected<std::vector<std::uint8_t>, errors::Error> dpapi_decrypt_string(std::string_view string);

}

#endif

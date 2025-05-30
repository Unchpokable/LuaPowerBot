#include <vector>

#include <windows.h>
#include <wincrypt.h>

#include <format>

#include "security.hxx"

namespace security::internal {

constexpr const wchar_t* Description = L"Packed bot Telegram API Key";

}

Expected<std::vector<std::uint8_t>, errors::Error> security::dpapi_encrypt(std::vector<std::uint8_t> &buffer)
{
    DATA_BLOB blob, out_blob;

    blob.cbData = static_cast<unsigned long>(buffer.size());
    blob.pbData = buffer.data();

    auto status = CryptProtectData(&blob, internal::Description, nullptr, nullptr, nullptr, 0, &out_blob);

    if(status == FALSE) {
        return errors::Error(std::format("Windows Data Protection API Returned an error code: {}", GetLastError()));
    }

    std::vector<std::uint8_t> result;
    result.resize(out_blob.cbData);

    std::memcpy(result.data(), out_blob.pbData, out_blob.cbData);

    LocalFree(out_blob.pbData);

    return result;
}

Expected<std::vector<std::uint8_t>, errors::Error> security::dpapi_decrypt(std::vector<std::uint8_t> &buffer)
{
    DATA_BLOB blob, out_blob;

    blob.cbData = static_cast<unsigned long>(buffer.size());
    blob.pbData = buffer.data();

    wchar_t* description { nullptr };

    auto status = CryptUnprotectData(&blob, &description, nullptr, nullptr, nullptr, 0, &out_blob);

    if(status == FALSE) {
        return errors::Error(std::format("Windows Data Protection API Returned an error code: {}", GetLastError()));
    }

    std::vector<std::uint8_t> result;
    result.resize(out_blob.cbData);

    std::memcpy(result.data(), out_blob.pbData, out_blob.cbData);

    SecureZeroMemory(out_blob.pbData, out_blob.cbData);

    LocalFree(out_blob.pbData);

    if(description) {
        LocalFree(description);
    }

    return result;
}

Expected<std::vector<std::uint8_t>, errors::Error> security::dpapi_encrypt_string(std::string_view string)
{
    std::vector<std::uint8_t> vector(string.begin(), string.end());
    return dpapi_encrypt(vector);
}

Expected<std::vector<std::uint8_t>, errors::Error> security::dpapi_decrypt_string(std::string_view string)
{
    std::vector<std::uint8_t> vector(string.begin(), string.end());
    return dpapi_decrypt(vector);
}

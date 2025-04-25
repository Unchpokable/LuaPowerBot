#define _CRT_SECURE_NO_WARNINGS

#include "zipped.hxx"

#include <zlib.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <ranges>

#include "logdef.hxx"
#include "security.hxx"

namespace fs = std::filesystem;

namespace compressed::internal {

std::size_t string_hash(std::string_view string)
{
    return std::hash<std::string>{}(std::string(string));
}

bool all_zeroes(const char* buffer, std::size_t buffer_size) {
    for(std::ptrdiff_t i { 0 }; i < buffer_size; i++) {
        if(buffer[i] != 0) {
            return false;
        }
    }

    return true;
}

ByteArray zlib_compress(const ByteArray &data)
{
    if(data.empty()) {
        return {};
    }

    uLong compressed_buffer_size = compressBound(data.size());
    ByteArray compressed_data(compressed_buffer_size);

    auto result = compress2(compressed_data.data(), &compressed_buffer_size, data.data(), data.size(), Z_BEST_COMPRESSION);

    if(result != Z_OK) {
        throw std::runtime_error(std::format("ZLib :: Compression Failed: {}", std::to_string(result)));
    }

    compressed_data.resize(compressed_buffer_size);

    return compressed_data;
}

ByteArray zlib_decompress(ByteArray &compressed_data, std::size_t uncompressed_size)
{
    if(compressed_data.empty()) {
        return {};
    }

    uLong decompressed_size = uncompressed_size;
    ByteArray decompressed_data(uncompressed_size);

    z_const Bytef* source = compressed_data.data();

    uLong source_length = compressed_data.size();

    auto result = uncompress2(decompressed_data.data(), &decompressed_size, source, &source_length);

    if(result != Z_OK) {
        throw std::runtime_error(std::format("ZLib :: Decompression failed: {}", std::to_string(result)));
    }

    if(decompressed_size != uncompressed_size) {
        throw std::runtime_error("Sized dont matches!");
    }

    return decompressed_data;
}

} // namespace compressed::internal

compressed::PackedBot::PackedBot(std::string_view file)
{
    _path = file;

    try {
        open();
    } catch(const std::exception&) {
        create();
    }
}

compressed::PackedBot::~PackedBot()
{
    if(_file.is_open()) {
        _file.close();
    }
}

std::vector<compressed::PackedBot::Entry> compressed::PackedBot::entries() const
{
    std::vector<Entry> entries;
    entries.reserve(_index.size());

    for(const auto &entry : _index | std::views::values) {
        entries.push_back(entry);
    }

    return entries;
}

bool compressed::PackedBot::hasEntry(std::string_view name) const
{
    return _index.contains(name);
}

Expected<compressed::PackedBot::Entry, errors::Error> compressed::PackedBot::getEntry(std::string_view name) const
{
    auto it = _index.find(name);
    if(it == _index.end()) {
        return errors::Error("Not found: " + std::string(name));
    }

    return it->second;
}

std::vector<compressed::PackedBot::Entry> compressed::PackedBot::entriesIf(const StringPredicate &predicate) const
{
    std::vector<Entry> result;

    for(const auto &[name, entry] : _index) {
        if(predicate(name)) {
            result.push_back(entry);
        }
    }

    return result;
}

void compressed::PackedBot::addEntry(std::string_view name, const ByteArray &data)
{
    if(hasEntry(name)) {
        removeEntry(name);
    }

    Entry entry;
    entry.name = name;
    entry.uncompressed_size = data.size();

    entry.offset = -1;
    entry.compressed_size = -1;

    auto hash = internal::string_hash(name);
    _uncompressedCache[hash] = data;

    _index.insert_or_assign(std::string(name), entry);
}

void compressed::PackedBot::removeEntry(std::string_view name)
{
    auto it = _index.find(name);

    if(it == _index.end()) {
        return;
    }

    _index.erase(it);

    auto hash = internal::string_hash(name);
    auto cache_it = _uncompressedCache.find(hash);
    if(cache_it != _uncompressedCache.end()) {
        _uncompressedCache.erase(cache_it);
    }
}

void compressed::PackedBot::replaceEntry(std::string_view name, const ByteArray &data)
{
    removeEntry(name);
    addEntry(name, data);
}

Expected<compressed::ByteArray, errors::Error> compressed::PackedBot::entryData(std::string_view name)
{
    auto hash = internal::string_hash(name);

    auto cached = _uncompressedCache.find(hash);
    if(cached != _uncompressedCache.end()) {
        return cached->second;
    }

    auto entry_result = getEntry(name);

    if(!entry_result) {
        return entry_result.error();
    }

    auto entry = entry_result.value();

    _file.seekg(entry.offset, std::ios::beg);

    std::ptrdiff_t name_length;
    _file.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));

    _file.seekg(name_length, std::ios::cur); // skip file name because it is already loaded into index. todo: compare indexed and read file name

    std::size_t compressed_size, uncompressed_size;

    _file.read(reinterpret_cast<char*>(&compressed_size), sizeof(compressed_size));
    _file.read(reinterpret_cast<char*>(&uncompressed_size), sizeof(uncompressed_size));

    ByteArray compressed_data;
    compressed_data.resize(compressed_size);

    _file.read(reinterpret_cast<char*>(compressed_data.data()), compressed_size);

    if(_file.fail()) {
        return errors::Error("Failed to read data for entry: " + std::string(name));
    }

    auto unzipped_data = internal::zlib_decompress(compressed_data, uncompressed_size);

    _uncompressedCache.insert_or_assign(hash, unzipped_data);

    return unzipped_data;
}

void compressed::PackedBot::removeFromCache(std::string_view name)
{
    auto hash = internal::string_hash(name);
    auto cached_it = _uncompressedCache.find(hash);

    if(cached_it != _uncompressedCache.end()) {
        _uncompressedCache.erase(cached_it);
    }
}

void compressed::PackedBot::save()
{
    fs::path origin_path(_path);
    fs::path temp_path = origin_path.parent_path() / fs::path(origin_path.filename().string() + ".temp");

    std::fstream temp_file(temp_path.string(), std::ios::binary | std::ios::out | std::ios::trunc);

    if(!temp_file.is_open()) {
        throw std::runtime_error("Failed to create temporary file for saving: " + temp_path.string());
    }

    auto original_file_pos = _file.tellg();

    try {
        char header[32] = { 0 };
        std::strncpy(header, DefaultHeader, sizeof(header) - 1);

        temp_file.write(header, sizeof(header));

        char token[128] = { 0 };
        
        auto encrypted_key = security::dpapi_encrypt_string(_apiToken);
        if(encrypted_key) {
            auto encrypted_value = encrypted_key.value();
            std::strncpy(token, reinterpret_cast<char*>(encrypted_value.data()), encrypted_key.value().size());
        } else {
            luabot_logErr("Error while encrypting Telegram Token. Saving a not encrypted private API tokens is not allowed. Skipping...");
            temp_file.seekp(sizeof(token), std::ios::cur);
        }

        temp_file.write(token, sizeof(token));

        std::size_t index_size = _index.size();
        temp_file.write(reinterpret_cast<char*>(&index_size), sizeof(index_size));

        auto offsets_pos = temp_file.tellp();

        char index_area[1024] = { 0 };
        temp_file.write(index_area, sizeof(index_area));

        std::vector<std::size_t> offsets;
        offsets.resize(_index.size(), 0);

        std::size_t entry_index = 0;

        auto temp_index = _index;
        for(auto &[name, temp_entry] : temp_index) {
            temp_entry.offset = temp_file.tellp();
            offsets[entry_index++] = temp_entry.offset;

            ByteArray data;
            auto hash = internal::string_hash(name);

            auto cached_it = _uncompressedCache.find(hash);

            auto original_entry = _index.at(name);

            if(cached_it != _uncompressedCache.end()) {
                temp_entry.uncompressed_size = cached_it->second.size();
                data = internal::zlib_compress(cached_it->second);
                temp_entry.compressed_size = data.size();
            } else if(original_entry.offset > 0) {
                _file.seekg(original_entry.offset, std::ios::beg);

                std::ptrdiff_t name_length;

                _file.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
                _file.seekg(name_length, std::ios::cur);

                std::size_t compressed_size, uncompressed_size;
                _file.read(reinterpret_cast<char*>(&compressed_size), sizeof(compressed_size));
                _file.read(reinterpret_cast<char*>(&uncompressed_size), sizeof(uncompressed_size));

                temp_entry.compressed_size = compressed_size;
                temp_entry.uncompressed_size = uncompressed_size;

                data.resize(compressed_size);

                _file.read(reinterpret_cast<char*>(data.data()), compressed_size);

                if(_file.fail()) {
                    throw std::runtime_error(std::format("Saving failed: Failed to read data for entry {}", temp_entry.name));
                }
            } else {
                throw std::runtime_error(std::format("WTF ERROR. Entry named {} has no data", temp_entry.name));
            }

            std::size_t name_length = name.size();
            temp_file.write(reinterpret_cast<char*>(&name_length), sizeof(name_length));

            temp_file.write(name.data(), name_length);

            temp_file.write(reinterpret_cast<char*>(&temp_entry.compressed_size), sizeof(temp_entry.compressed_size));
            temp_file.write(reinterpret_cast<char*>(&temp_entry.uncompressed_size), sizeof(temp_entry.uncompressed_size));

            temp_file.write(reinterpret_cast<char*>(data.data()), data.size());
        }

        temp_file.seekp(offsets_pos);
        temp_file.write(reinterpret_cast<char*>(offsets.data()), offsets.size() * sizeof(std::size_t));
        temp_file.flush();

        if(!temp_file.fail()) {
            _index = std::move(temp_index);
        } else {
            throw std::runtime_error("Failed to write a temporary file!");
        }

        if(fs::exists(_path)) {
            fs::remove(_path);
        }

        fs::rename(temp_path, _path);

        open();

    } catch(const std::exception& e) {
        temp_file.close();

        if(fs::exists(temp_path)) {
            fs::remove(temp_path);
        }

        if(original_file_pos >= 0) {
            _file.clear();
            _file.seekg(original_file_pos);
        }

        throw;
    }
}

bool compressed::PackedBot::isTokenAvailable() const
{
    return _tokenAvailable;
}

Expected<std::string, errors::Error> compressed::PackedBot::token() const
{
    if(_tokenAvailable) {
        return _apiToken;
    }

    return errors::Error("Access to token denied or data corrupted");
}

void compressed::PackedBot::setToken(std::string_view token)
{
    _apiToken = token;
    _tokenAvailable = true;
}

void compressed::PackedBot::create() 
{
    _file.open(_path, std::ios::binary | std::ios::out | std::ios::trunc);
    if(!_file.is_open()) {
        throw std::runtime_error(std::format("Failed to open file {}", _path));
    }

    char header[32] = { 0 };
    std::strncpy(header, DefaultHeader, sizeof(header) - 1);
    _file.write(header, sizeof(header));

    char token[128] = { 0 };

    if(!_apiToken.empty()) {
        auto encrypted_key = security::dpapi_encrypt_string(_apiToken);
        if(encrypted_key) {
            auto encrypted_value = encrypted_key.value();
            std::strncpy(token, reinterpret_cast<char*>(encrypted_value.data()), encrypted_key.value().size());
        } else {
            luabot_logErr("Error while encrypting Telegram Token. Saving a not encrypted private API tokens is not allowed. Skipping...");
            _file.seekp(sizeof(token), std::ios::cur);
        }
    }

    _file.write(token, sizeof(token));

    std::size_t index_size = 0;
    _file.write(reinterpret_cast<char*>(&index_size), sizeof(index_size));

    char index_area[1024] = { 0 };
    _file.write(index_area, sizeof(index_area));

    _file.flush();
    _file.close();

    open();

    _uncompressedCache.clear();
}

void compressed::PackedBot::open() 
{
    if(_file.is_open()) {
        _file.close();
    }

    _file.open(_path, std::ios::binary | std::ios::in | std::ios::out);

    if(!_file.is_open()) {
        throw std::runtime_error(std::format("Unable to open file: {}", _path));
    }

    _index.clear();
    _uncompressedCache.clear();

    char header[32];
    _file.read(header, sizeof(header));

    if(std::strncmp(header, DefaultHeader, std::strlen(DefaultHeader)) != 0) {
        throw std::runtime_error("File corrupted or versions incompatible");
    }

    char token[128];
    _file.read(token, sizeof(token));

    if(!internal::all_zeroes(token, sizeof(token))) {
        auto unprotected_token = security::dpapi_decrypt_string(token);
        if(unprotected_token) {
            auto token_value = unprotected_token.value();
            std::string_view readable_token(reinterpret_cast<char*>(token_value.data()), unprotected_token.value().size());
            _apiToken = std::string(readable_token);
        }
        else {
            luabot_logErr("Unable to unprotect token from file. Bot project was edited on other PC or by different user or data corrupted");
        }
    }

    makeIndex();
}

void compressed::PackedBot::makeIndex() 
{
    std::size_t index_size;

    _file.read(reinterpret_cast<char*>(&index_size), sizeof(index_size));

    std::vector<std::ptrdiff_t> offsets(index_size);

    _file.read(reinterpret_cast<char*>(offsets.data()), index_size);

    for(std::size_t i { 0 }; i < index_size; i++) {
        std::size_t name_length;
        _file.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));

        std::string name;
        name.resize(name_length + 1, 0);

        _file.read(name.data(), name_length);

        std::size_t compressed_size, uncompressed_size;

        _file.read(reinterpret_cast<char*>(&compressed_size), sizeof(compressed_size));
        _file.read(reinterpret_cast<char*>(&uncompressed_size), sizeof(uncompressed_size));

        Entry entry;
        entry.name = name;
        entry.offset = offsets[i];
        entry.compressed_size = compressed_size;
        entry.uncompressed_size = uncompressed_size;

        _index.insert_or_assign(name, entry);

        _file.seekg(compressed_size, std::ios::cur);
    }
}

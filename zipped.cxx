#define _CRT_SECURE_NO_WARNINGS

#include "zipped.hxx"

#include <zlib.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <filesystem>

namespace {

std::size_t string_hash(std::string_view string)
{
    return std::hash<std::string>{}(std::string(string));
}

}

namespace fs = std::filesystem;

compressed::PackedBot::PackedBot(std::string_view file)
{
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

    for(const auto &[name, entry] : _index) {
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

    auto hash = string_hash(name);
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

    auto hash = string_hash(name);
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

Expected<compressed::PackedBot::ByteArray, errors::Error> compressed::PackedBot::entryData(std::string_view name)
{
    auto hash = string_hash(name);

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
    compressed_data.reserve(compressed_size);

    _file.read(reinterpret_cast<char*>(compressed_data.data()), compressed_size);

    if(_file.fail()) {
        return errors::Error("Failed to read data for entry: " + std::string(name));
    }

    auto unzipped_data = zlib_decompress(compressed_data, uncompressed_size);

    _uncompressedCache.insert_or_assign(hash, unzipped_data);

    return unzipped_data;
}

void compressed::PackedBot::save(const std::string& api_key)
{
    fs::path origin_path(_path);
    fs::path temp_path = origin_path.parent_path() / fs::path(origin_path.filename().string() + ".temp");

    std::fstream temp_file(temp_path.string(), std::ios::binary | std::ios::out | std::ios::trunc);

    if(!temp_file.is_open()) {
        throw std::runtime_error("Failed to create temporary file for saving: " + temp_path.string());
    }

    try {
        char header[32] = { 0 };
        std::strncpy(header, DefaultHeader, sizeof(header) - 1);

        temp_file.write(header, sizeof(header));

        char token[128];
        // todo: encrypt using DPAPI

        temp_file.write(token, sizeof(token));

    } catch(const std::exception& e) {
        
    }
}

Expected<std::string, errors::Error> compressed::PackedBot::key() const
{
    auto key_record = _index.at("key");
}

void compressed::PackedBot::create() 
{
}

void compressed::PackedBot::open() 
{
}

void compressed::PackedBot::makeIndex() 
{
}

compressed::PackedBot::ByteArray compressed::PackedBot::zlib_compress(const ByteArray &data) 
{
}

compressed::PackedBot::ByteArray compressed::PackedBot::zlib_decompress(const ByteArray &compressed_data, std::size_t uncompressed_size) 
{
}

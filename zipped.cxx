#include "zipped.hxx"

#include <zlib.h>
#include <iostream>
#include <algorithm>
#include <cstring>

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

    auto hash = std::hash<std::string> {}(std::string(name));
    _uncompressedCache[hash] = data;

    _index.insert_or_assign(std::string(name), entry);
}

void compressed::PackedBot::removeEntry(std::string_view name)
{
}

void compressed::PackedBot::replaceEntry(std::string_view name, const ByteArray &data) {
}

Expected<compressed::PackedBot::ByteArray, errors::Error> compressed::PackedBot::entryData(std::string_view name)
{
}

void compressed::PackedBot::save(const std::string& api_key)
{
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

compressed::PackedBot::ByteArray compressed::PackedBot::zlib_compressData(const ByteArray &data) 
{
}

compressed::PackedBot::ByteArray compressed::PackedBot::zlib_decompressData(const ByteArray &compressed_data, std::size_t uncompressed_size) 
{
}

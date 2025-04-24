#pragma once

#include <ctime>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "error.hxx"
#include "expected.hxx"

/*
Packed bot file structure description
byte size : type \\ comment

[header]
32 : C-string \\ version header, up to 32 bytes without \0
128 : byte array \\ DPAPI encrypted telegram API token (for gui editor)

[index] [ fixed size 1 KiB - up to 128 entries]
8 : std::size_t \\ index size
1024 : byte array \\ index. Array of std::size_t - offsets of content. Offsets can be unordered because while indexing data in any case file::seekg will be called for any next index

[content]
unrestricted array of data structured as:
8 : std::size_t \\ file name length
variadic : byte array \\ file name without \0. Byte array with length of file name length
8 : std::size_t \\ compressed data size
8 : std::size_t \\ uncompressed data size
variadic : byte array \\ content array with length of compressed data size


Generic statements:

- Any changes in entries applies only in RAM. Physical file should be opened along a full PackedBot lifetime.
- Removing and replacing files also applies only in RAM
- Save is full rewrite
*/

namespace compressed {

class PackedBot final {
public:
    using ByteArray = std::vector<std::uint8_t>;
    using StringPredicate = std::function<bool(std::string_view)>;

    constexpr static const char* DefaultHeader = "LUAPOWER_PACKED_BOT_v1";

    struct Entry {
        std::string name;
        std::ptrdiff_t offset; 
        std::size_t compressed_size;
        std::size_t uncompressed_size;
    };

    explicit PackedBot(std::string_view file);
    ~PackedBot();

    std::vector<Entry> entries() const;

    bool hasEntry(std::string_view name) const;
    Expected<Entry, errors::Error> getEntry(std::string_view name) const;

    std::vector<Entry> entriesIf(const StringPredicate &predicate) const;

    void addEntry(std::string_view name, const ByteArray& data);
    void removeEntry(std::string_view name);
    void replaceEntry(std::string_view name, const ByteArray& data);

    Expected<ByteArray, errors::Error> entryData(std::string_view name);

    void save(const std::string& api_key);

    Expected<std::string, errors::Error> key() const;

private:
    void create();
    void open();

    void makeIndex();

    static ByteArray zlib_compress(const ByteArray& data);
    static ByteArray zlib_decompress(const ByteArray& compressed_data, std::size_t uncompressed_size);

    std::string _path;
    std::fstream _file;
    std::map<std::string, Entry, std::less<>> _index;
    std::unordered_map<std::size_t, ByteArray> _uncompressedCache;
};

}

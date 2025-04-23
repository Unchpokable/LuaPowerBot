#pragma once

#include <ctime>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace compressed {

class PackedBot final {
public:
    constexpr static const char* DefaultHeader = "LUAPOWER_PACKED_BOT_v1";

    struct Entry {
        std::string name;
        std::size_t offset;
        std::size_t compressed_size;
        std::size_t uncompressed_size;
        std::time_t timestamp;
    };

    enum class Mode {
        Read,
        Write,
        ReadWrite,
        Append
    };

    std::vector<Entry> entries() const;

    bool hasEntry(std::string_view name);
    Entry getEntry(std::string_view name);

    void addEntry(std::string_view name, const std::vector<std::uint8_t>& data);

    std::vector<std::uint8_t> entryData(std::string_view name);

    std::string extractText(std::string_view name);

    void repack();

private:
    void create();
    void open();
    void readIndex(std::size_t offset);
    void writeIndex();

    void writeIndexTo(std::ostream& stream, const std::map<std::string, Entry>& entries);

    std::vector<std::uint8_t> zlib_compressData(const std::vector<std::uint8_t> *data);
    std::vector<uint8_t> zlib_decompressData(const std::vector<std::uint8_t>& compressed_data, std::size_t uncompressed_size);

    Mode _openMode { Mode::ReadWrite };

    std::string _path;
    std::fstream _file;
    std::map<std::string, Entry> _entries;
};

}

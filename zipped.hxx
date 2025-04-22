#pragma once

#include <zlib.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <ctime>

namespace compressed {

class PackedBot final {
public:
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
        Append
    };

    std::size_t entriesCount() const;
    std::vector<Entry> entries() const;

private:
    std::string _path;
    Mode _openMode;

    std::fstream _file;
    std::map<std::string, Entry> _entries;
};

}

#define _CRT_SECURE_NO_WARNINGS

#include "zip2memvfs.hxx"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <ranges>

#include "logdef.hxx"
#include "security.hxx"

Expected<vfspp::ZipFileSystemPtr, errors::Error> files::open_zip(const std::string& name)
{
    auto fs = std::make_shared<vfspp::ZipFileSystem>(name);

    fs->Initialize();

    if(fs->IsInitialized()) {
        return fs;
    }

    return errors::Error(std::format("Unable to open archive {}", name));
}

Expected<files::ByteArray, errors::Error> files::read_bytes(const ZipFS& zip, const std::string& name)
{
    auto file = zip->OpenFile(vfspp::FileInfo(name), vfspp::IFile::FileMode::Read);
    if(!file) {
        return errors::Error(std::format("File not found: {}", name));
    }

    return read_bytes(file);
}

Expected<files::ByteArray, errors::Error> files::read_bytes(const vfspp::IFilePtr& file_name)
{
    std::vector<std::uint8_t> data;
    data.resize(file_name->Size());

    std::uint64_t bytes_read = file_name->Read(data.data(), data.size());
    if(bytes_read != data.size()) {
        return errors::Error(std::format("Failed to read file: {}", file_name->GetFileInfo().Name()));
    }

    return data;
}

Expected<std::string, errors::Error> files::read_text(const ZipFS& zip, const std::string& file_name)
{
    auto file = zip->OpenFile(vfspp::FileInfo(file_name), vfspp::IFile::FileMode::Read);
    if(!file) {
        return errors::Error(std::format("File not found: {}", file_name));
    }

    return read_text(file);
}

Expected<std::string, errors::Error> files::read_text(const vfspp::IFilePtr& file)
{
    auto bytes_result = read_bytes(file);

    if(!bytes_result) {
        return bytes_result.error();
    }

    auto& data = bytes_result.value();

    return std::string(data.begin(), data.end());
}

Expected<files::VirtualFS, errors::Error> files::open_subdir(const ZipFS& zip, const std::string& directory, bool readonly)
{
    vfspp::FileInfo dir_info(directory);
    if(!zip->IsDir(dir_info)) {
        return errors::Error(std::format("Path is not a directory: {}", directory));
    }

    auto vfs = std::make_shared<vfspp::VirtualFileSystem>();

    auto sub_fs = std::make_shared<SubDirectory>(zip, directory, readonly);

    vfs->AddFileSystem("/", sub_fs);

    return vfs;
}

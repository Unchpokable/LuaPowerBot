#define _CRT_SECURE_NO_WARNINGS

#include "zip2memvfs.hxx"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <ranges>

#include "logdef.hxx"
#include "security.hxx"

namespace files::internal {

ByteArray text_to_bytes(const std::string& string)
{
    ByteArray bytes;

    bytes.insert(bytes.begin(), string.begin(), string.end());

    return bytes;
}

}

Expected<vfspp::ZipFileSystemPtr, errors::Error> files::open_zip(const std::string& name)
{
    auto fs = std::make_shared<vfspp::ZipFileSystem>(name);

    fs->Initialize();

    if(fs->IsInitialized()) {
        return fs;
    }

    return errors::Error(std::format("Unable to open archive {}", name));
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

Expected<files::ByteArray, errors::Error> files::read_bytes(const ZipFS& zip, const std::string& name)
{
    auto file = zip->OpenFile(vfspp::FileInfo(name), vfspp::IFile::FileMode::Read);
    if(!file) {
        return errors::Error(std::format("File not found: {}", name));
    }

    auto result = read_bytes(file);

    file->Close();

    return result;
}

Expected<files::ByteArray, errors::Error> files::read_bytes(const vfspp::IFilePtr& file)
{
    std::vector<std::uint8_t> data;
    data.resize(file->Size());

    std::uint64_t bytes_read = file->Read(data.data(), data.size());
    if(bytes_read != data.size()) {
        return errors::Error(std::format("Failed to read file: {}", file->GetFileInfo().Name()));
    }

    return data;
}

Expected<std::string, errors::Error> files::read_text(const ZipFS& zip, const std::string& file_name)
{
    auto file = zip->OpenFile(vfspp::FileInfo(file_name), vfspp::IFile::FileMode::Read);
    if(!file) {
        return errors::Error(std::format("File not found: {}", file_name));
    }

    auto result = read_text(file);

    file->Close();

    return result;
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

errors::FileSystemResult files::append_bytes(const ZipFS& zip, const std::string& name, const ByteArray& bytes)
{
    auto file = zip->OpenFile(name, vfspp::IFile::FileMode::Append);
    auto result = append_bytes(file, bytes);
    file->Close();

    return result;
}

errors::FileSystemResult files::append_bytes(const vfspp::IFilePtr& file, const ByteArray& bytes)
{
    auto bytes_written = file->Write(bytes.data(), bytes.size());

    if(bytes_written != bytes.size()) {
        return errors::UnableToWrite;
    }

    return errors::OK;
}

errors::FileSystemResult files::rewrite_bytes(const ZipFS& zip, const std::string& name, const ByteArray& bytes)
{
    auto file = zip->OpenFile(name, vfspp::IFile::FileMode::Write | vfspp::IFile::FileMode::Truncate);

    auto result = rewrite_bytes(file, bytes);

    file->Close();

    return result;
}

errors::FileSystemResult files::rewrite_bytes(const vfspp::IFilePtr& file, const ByteArray& bytes)
{
    if(file->IsOpened()) {
        file->Close();
    }

    file->Open(vfspp::IFile::FileMode::Write | vfspp::IFile::FileMode::Truncate);

    auto bytes_written = file->Write(bytes.data(), bytes.size());

    if(bytes_written != bytes.size()) {
        return errors::UnableToWrite;
    }

    file->Close();
    file->Open(vfspp::IFile::FileMode::Read | vfspp::IFile::FileMode::Append);

    return errors::OK;
}

errors::FileSystemResult files::append_text(const ZipFS& zip, const std::string& name, const std::string& text)
{
    auto file = zip->OpenFile(vfspp::FileInfo(name), vfspp::IFile::FileMode::Append);

    auto result = append_text(file, text);

    file->Close();

    return result;
}

errors::FileSystemResult files::append_text(const vfspp::IFilePtr& file, const std::string& text)
{
    auto text_bytes = internal::text_to_bytes(text);

    auto result = append_bytes(file, text_bytes);

    return result;
}

errors::FileSystemResult files::write_text(const ZipFS& zip, const std::string& name, const std::string& text)
{
    auto file = zip->OpenFile(vfspp::FileInfo(name), vfspp::IFile::FileMode::Append | vfspp::IFile::FileMode::Truncate);

    auto result = write_text(file, text);

    file->Close();

    return result;
}

errors::FileSystemResult files::write_text(const vfspp::IFilePtr& file, const std::string& text)
{
    auto text_bytes = internal::text_to_bytes(text);

    auto result = rewrite_bytes(file, text_bytes);

    return result;
}

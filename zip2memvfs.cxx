#define _CRT_SECURE_NO_WARNINGS

#include "zip2memvfs.hxx"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <ranges>

#include "logdef.hxx"
#include "security.hxx"

files::Zip2MemVirtualFileSystem::Zip2MemVirtualFileSystem(std::string_view zip_path)
    : _mem_fs(std::make_shared<vfspp::MemoryFileSystem>()), _zip_fs(std::make_shared<vfspp::ZipFileSystem>(std::string(zip_path)))
{
    _mem_fs->Initialize();
    _zip_fs->Initialize();
}

vfspp::FileInfo files::Zip2MemVirtualFileSystem::file_info(std::string_view name)
{
}

std::vector<vfspp::FileInfo> files::Zip2MemVirtualFileSystem::files(std::string_view root)
{
}

// todo: maybe Expected<>?
vfspp::IFilePtr files::Zip2MemVirtualFileSystem::open(std::string_view name, vfspp::IFile::FileMode mode)
{
    auto file_name = std::string(name);

    vfspp::FileInfo info(file_name);

    if(_mem_fs->IsFile(info)) {
        return _mem_fs->OpenFile(info, mode);
    }

    if(_zip_fs->IsFile(info)) {
        load_to_memory(info);

        return _mem_fs->OpenFile(info, mode);
    }

    return nullptr;
}

Expected<files::ByteArray, errors::Error> files::Zip2MemVirtualFileSystem::read(std::string_view name)
{
    auto file = open(name, vfspp::IFile::FileMode::Read);
    if(!file) {
        return errors::Error(std::format("No file named {}", name));
    }

    auto size = file->Size();

    ByteArray buffer;
    buffer.resize(size);

    file->Read(buffer.data(), size);

    return buffer;
}

vfspp::VirtualFileSystemPtr files::Zip2MemVirtualFileSystem::open_subdir(std::string_view path, bool readonly)
{
    std::string cleaned_path(path);
    if(!cleaned_path.empty() && cleaned_path.back() == '/') {
        cleaned_path.pop_back();
    }

    if(cleaned_path.empty() || cleaned_path[0] != '/') {
        cleaned_path = "/" + cleaned_path;
    }

    auto it = _opened_subdirs.find(cleaned_path);
    if(it != _opened_subdirs.end()) {
        return it->second;
    }

    auto vfs = std::make_shared<vfspp::VirtualFileSystem>();

    vfspp::FileInfo dir_info(cleaned_path);
    if(_mem_fs->IsDir(dir_info)) {
        auto sub_fs = std::make_shared<SubDirectory>(_mem_fs, cleaned_path, readonly);
        vfs->AddFileSystem("/", sub_fs);

        _opened_subdirs[cleaned_path] = vfs;
        return vfs;
    }

    if(_zip_fs->IsDir(dir_info)) {
        const auto& zip_files = _zip_fs->FileList();
        std::string search_prefix = cleaned_path;
        if(!search_prefix.empty() && search_prefix.back() != '/') {
            search_prefix += "/";
        }

        for(const auto& [file_path, file] : zip_files) {
            if(file_path.starts_with(search_prefix)) {
                load_to_memory(file->GetFileInfo());
            }
        }

        auto sub_fs = std::make_shared<SubDirectory>(_zip_fs, cleaned_path, true);
        vfs->AddFileSystem("/", sub_fs);
        _opened_subdirs[cleaned_path] = vfs;
        return vfs;
    }

    return nullptr;
}

void files::Zip2MemVirtualFileSystem::load_to_memory(const vfspp::FileInfo &file_info)
{
    vfspp::FileInfo mem_info(_mem_fs->BasePath() + file_info.AbsolutePath());
    if(_mem_fs->IsFileExists(mem_info)) {
        return;
    }

    if(_zip_fs->IsDir(file_info)) {
        auto files = _zip_fs->FileList(file_info);
        return;
    }

    auto zip_file = _zip_fs->OpenFile(file_info, vfspp::IFile::FileMode::Read);
    if(!zip_file) {
        luabot_logErr("Failed to open file {} from ZIP", file_info.AbsolutePath());
        return;
    }

    std::string dir_path = file_info.Path().parent_path().string();
    if(!dir_path.empty()) {
        vfspp::FileInfo dir_info(_mem_fs->BasePath() + dir_path);
    }

    if(!_mem_fs->CreateFile(mem_info)) {
        luabot_logErr("Failed to create file {} in memory", file_info.AbsolutePath());
        _zip_fs->CloseFile(zip_file);
        return;
    }

    auto mem_file = _mem_fs->OpenFile(mem_info, vfspp::IFile::FileMode::Write);
    if(!mem_file) {
        luabot_logErr("Failed to open file {} for writing", file_info.AbsolutePath());
        _mem_fs->RemoveFile(mem_info);
        _zip_fs->CloseFile(zip_file);
        return;
    }

    uint8_t buffer[8192];
    size_t bytes_read;
    bool success = true;

    while((bytes_read = zip_file->Read(buffer, sizeof(buffer))) > 0) {
        size_t bytes_written = mem_file->Write(buffer, bytes_read);
        if(bytes_written != bytes_read) {
            luabot_logErr("Failed to write all data to file {}", file_info.AbsolutePath());
            success = false;
            break;
        }
    }

    _mem_fs->CloseFile(mem_file);
    _zip_fs->CloseFile(zip_file);

    if(success) {
        _modified_files.insert(file_info.AbsolutePath());
    } else {
        _mem_fs->RemoveFile(mem_info);
    }

    _zip_fs->CloseFile(zip_file);
}

void files::Zip2MemVirtualFileSystem::load_directory_to_memory(const vfspp::FileInfo &file_info)
{
}

bool files::Zip2MemVirtualFileSystem::create_directory_recursive(const std::string &dir_path) const
{
    std::vector<std::string> components;
    std::stringstream ss(dir_path);
    std::string item;

    while(std::getline(ss, item, '/')) {
        if(!item.empty()) {
            components.push_back(item);
        }
    }

    std::string current_path = "/";
    for(const auto& component : components) {
        current_path += component + "/";
        vfspp::FileInfo dir_info(current_path);

        if(!_mem_fs->IsDir(dir_info)) {
            if(!_mem_fs->CreateFile(dir_info)) {
                return false;
            }
        }
    }

    return true;
}

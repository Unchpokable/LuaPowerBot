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
{ }

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

    auto it = _opened_subdirs.find(cleaned_path);
    if(it != _opened_subdirs.end()) {
        return it->second;
    }

    auto vfs = std::make_shared<vfspp::VirtualFileSystem>();

    vfspp::FileInfo dir_info(_mem_fs->BasePath() + cleaned_path);
    if(_mem_fs->IsDir(dir_info)) {
        auto sub_fs = std::make_shared<SubDirectory>(_mem_fs, cleaned_path, readonly);
        vfs->AddFileSystem("/", sub_fs);

        _opened_subdirs[cleaned_path] = vfs;
        return vfs;
    }

    vfspp::FileInfo zip_dir_info(_zip_fs->BasePath() + cleaned_path);
    if(_zip_fs->IsDir(zip_dir_info)) {
        const auto& zip_files = _zip_fs->FileList();
        for(const auto& [file_path, file] : zip_files) {
            if(file_path.starts_with(cleaned_path + "/")) {
                load_to_memory(file->GetFileInfo());
            }
        }

        auto sub_fs = std::make_shared<SubDirectory>(_zip_fs, cleaned_path);
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

    auto zip_file = _zip_fs->OpenFile(file_info, vfspp::IFile::FileMode::Read);
    if(!zip_file) {
        return;
    }

    std::string dir_path = file_info.BaseName();
    if(!dir_path.empty()) {
        vfspp::FileInfo dir_info(_mem_fs->BasePath() + dir_path);
    }

    if(_mem_fs->CreateFile(mem_info)) {
        auto mem_file = _mem_fs->OpenFile(mem_info, vfspp::IFile::FileMode::Write);
        if(mem_file) {
            uint8_t buffer[8192];
            size_t bytes_read;
            while((bytes_read = zip_file->Read(buffer, sizeof(buffer))) > 0) {
                mem_file->Write(buffer, bytes_read);
            }
            _mem_fs->CloseFile(mem_file);

            _modified_files.insert(file_info.AbsolutePath());
        }
    }

    _zip_fs->CloseFile(zip_file);
}

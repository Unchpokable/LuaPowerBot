#pragma once

#include <ctime>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "fsizes.hxx"

#include "error.hxx"
#include "expected.hxx"

#define VFSPP_ENABLE_MULTITHREADING

#include "vfspp/MemoryFileSystem.hpp"
#include "vfspp/VirtualFileSystem.hpp"
#include "vfspp/ZipFileSystem.hpp"

namespace files {

using ByteArray = std::vector<std::uint8_t>;

class SubDirectory : public vfspp::IFileSystem
{
public:
    explicit SubDirectory(vfspp::IFileSystemPtr base_fs, std::string_view path, bool readonly = false);
    
    virtual void Initialize() override;
    virtual void Shutdown() override;
    virtual bool IsInitialized() const override;
    virtual const std::string& BasePath() const override;
    virtual const TFileList& FileList() const override;
    virtual bool IsReadOnly() const override;
    virtual vfspp::IFilePtr OpenFile(const vfspp::FileInfo &filePath, vfspp::IFile::FileMode mode) override;
    virtual void CloseFile(vfspp::IFilePtr file) override;
    virtual bool CreateFile(const vfspp::FileInfo &filePath) override;
    virtual bool CopyFile(const vfspp::FileInfo &src, const vfspp::FileInfo &dest) override;
    virtual bool IsFile(const vfspp::FileInfo &filePath) const override;
    virtual bool IsFileExists(const vfspp::FileInfo &filePath) const override;
    virtual bool RemoveFile(const vfspp::FileInfo &filePath) override;
    virtual bool RenameFile(const vfspp::FileInfo &src, const vfspp::FileInfo &dest) override;
    virtual bool IsDir(const vfspp::FileInfo &dirPath) const override;

private:
    static std::string clean_path(std::string_view path);
    std::string to_full_path(std::string_view path) const;
    std::string to_local_path(std::string_view path) const;

    vfspp::IFileSystemPtr _origin_fs;
    std::string _prefix;
    bool _readonly;

    std::map<std::string, vfspp::IFilePtr, std::less<>> _cached_files;
};

class Zip2MemVirtualFileSystem final {
public:
    constexpr std::size_t full_memory_cache_limit = 200;

    using StringPredicate = std::function<bool(std::string_view)>;

    explicit Zip2MemVirtualFileSystem(std::string_view zip_path);

    vfspp::FileInfo file_info(std::string_view name);
    std::vector<vfspp::FileInfo> files(std::string_view root = "/");

    vfspp::IFilePtr open(std::string_view name, vfspp::IFile::FileMode mode);

    Expected<files::ByteArray, errors::Error> read(std::string_view name);

    vfspp::VirtualFileSystemPtr open_subdir(std::string_view path, bool readonly = false);

private:
    constexpr std::size_t full_memory_caching_threshold = sizes::megabytes(full_memory_cache_limit);

    void load_to_memory(const vfspp::FileInfo& file_info);
    void load_directory_to_memory(const vfspp::FileInfo& file_info);
    bool create_directory_recursive(const std::string& dir_path) const;

    vfspp::MemoryFileSystemPtr _mem_fs;
    vfspp::ZipFileSystemPtr _zip_fs;

    std::set<std::string> _modified_files;

    std::map<std::string, vfspp::VirtualFileSystemPtr, std::less<>> _opened_subdirs;
};

}

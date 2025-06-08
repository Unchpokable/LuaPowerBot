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

using IFileSystem = vfspp::IFileSystemPtr;
using MemFS = vfspp::MemoryFileSystemPtr;
using VirtualFS = vfspp::VirtualFileSystemPtr;

constexpr std::size_t file_size_limit = sizes::megabytes<std::size_t>(200);

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

    mutable TFileList _file_list;
};

Expected<IFileSystem> open_zip(const std::string& name);
errors::FileSystemResult save_zip(const IFileSystem& zip);

Expected<VirtualFS> open_subdir(const IFileSystem& zip, const std::string& directory, bool readonly = false);

Expected<ByteArray> read_bytes(const IFileSystem& zip, const std::string& name);
Expected<ByteArray> read_bytes(const vfspp::IFilePtr& file);
Expected<std::string> read_text(const IFileSystem& zip, const std::string& file_name);
Expected<std::string> read_text(const vfspp::IFilePtr& file);

errors::FileSystemResult append_bytes(const IFileSystem& zip, const std::string& name, const ByteArray& bytes);
errors::FileSystemResult append_bytes(const vfspp::IFilePtr& file, const ByteArray& bytes);

errors::FileSystemResult write_bytes(const IFileSystem& zip, const std::string& name, const ByteArray& bytes);
errors::FileSystemResult write_bytes(const vfspp::IFilePtr& file, const ByteArray& bytes);

errors::FileSystemResult append_text(const IFileSystem& zip, const std::string& name, const std::string& text);
errors::FileSystemResult append_text(const vfspp::IFilePtr& file, const std::string& text);

errors::FileSystemResult write_text(const IFileSystem& zip, const std::string& name, const std::string& text);
errors::FileSystemResult write_text(const vfspp::IFilePtr& file, const std::string& text);

}

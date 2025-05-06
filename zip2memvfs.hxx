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
using ZipFS = vfspp::ZipFileSystemPtr;
using VirtualFS = vfspp::VirtualFileSystemPtr;

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
};

Expected<ZipFS, errors::Error> open_zip(const std::string& name);
Expected<ByteArray, errors::Error> read_bytes(const ZipFS& zip, const std::string& name);
Expected<ByteArray, errors::Error> read_bytes(const vfspp::IFilePtr& file_name);
Expected<std::string, errors::Error> read_text(const ZipFS& zip, const std::string& file_name);
Expected<std::string, errors::Error> read_text(const vfspp::IFilePtr& file);
Expected<VirtualFS, errors::Error> open_subdir(const ZipFS& zip, const std::string& directory, bool readonly = false);

}

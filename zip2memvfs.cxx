#define _CRT_SECURE_NO_WARNINGS

#include <ranges>

#include "zip2memvfs.hxx"

#include "logdef.hxx"
#include "security.hxx"

namespace files::internal {

ByteArray text_to_bytes(const std::string& string)
{
    ByteArray bytes;

    bytes.insert(bytes.begin(), string.begin(), string.end());

    return bytes;
}

bool check_file_flags(const vfspp::IFilePtr& file, vfspp::IFile::FileMode flags)
{
    auto current_mode = file->GetCurrentMode();
    return (current_mode & flags) == flags;
}

bool is_file_dir(const vfspp::IFilePtr &file)
{
    auto info = file->GetFileInfo();
    auto name = info.AbsolutePath();

    return name.ends_with("/");
}

Expected<vfspp::MemoryFileSystemPtr> fs_map(const vfspp::ZipFileSystemPtr& zip)
{
    auto mem_fs = std::make_shared<vfspp::MemoryFileSystem>();
    mem_fs->Initialize();

    if(!mem_fs->IsInitialized()) {
        return errors::Error("Unable to create a memory-mapped file system");
    }

    for(auto &[name, file] : zip->FileList()) {
        auto info = file->GetFileInfo();

        if(info.IsDir() || !info.IsValid() || is_file_dir(file)) {
            info = vfspp::FileInfo(info.Path(), true);
        }

        mem_fs->CreateFile(info);

        if(info.IsDir()) {
            continue;
        }

        auto bytes_result = read_bytes(file);

        if(!bytes_result) {
            bytes_result.error().throwError<std::runtime_error>();
        }

        auto bytes = bytes_result.value();

        auto memory_file = mem_fs->OpenFile(info, vfspp::IFile::FileMode::ReadWrite);

        write_bytes(memory_file, bytes);

        if(memory_file->IsOpened()) {
            memory_file->Close();
        }
    }

    return mem_fs;
}

}

Expected<files::IFileSystem> files::open_zip(const std::string& name)
{
    auto fs = std::make_shared<vfspp::ZipFileSystem>(name);

    fs->Initialize();

    if(fs->IsInitialized()) {
        auto mem_fs = internal::fs_map(fs);
        if(!mem_fs) {
            return mem_fs.error();
        }

        auto ifs = vfspp::IFileSystemPtr(std::move(mem_fs.value()));

        return ifs;
    }

    return errors::Error(std::format("Unable to open archive {}", name));
}

Expected<files::IFileSystem> files::open_subdir(const IFileSystem& zip, const std::string& directory, bool readonly)
{
    vfspp::FileInfo dir_info(directory);
    if(!zip->IsDir(dir_info)) {
        return errors::Error(std::format("Path is not a directory: {}", directory));
    }

    auto sub_fs = std::make_shared<SubDirectory>(zip, directory, readonly);

    return IFileSystem(std::move(sub_fs));
}

Expected<files::ByteArray> files::read_bytes(const IFileSystem& zip, const std::string& name)
{
    auto file = zip->OpenFile(vfspp::FileInfo(name), vfspp::IFile::FileMode::Read);
    if(!file) {
        return errors::Error(std::format("File not found: {}", name));
    }

    auto result = read_bytes(file);

    file->Close();

    return result;
}

Expected<files::ByteArray> files::read_bytes(const vfspp::IFilePtr& file)
{
    std::vector<std::uint8_t> data;
    data.resize(file->Size());

    if(!file->IsOpened()) {
        file->Open(vfspp::IFile::FileMode::Read);
    }

    std::uint64_t bytes_read = file->Read(data.data(), data.size());
    if(bytes_read != data.size()) {
        return errors::Error(std::format("Failed to read file: {}", file->GetFileInfo().Name()));
    }

    return data;
}

Expected<std::string> files::read_text(const IFileSystem& zip, const std::string& file_name)
{
    auto file = zip->OpenFile(vfspp::FileInfo(file_name), vfspp::IFile::FileMode::Read);
    if(!file) {
        return errors::Error(std::format("File not found: {}", file_name));
    }

    auto result = read_text(file);

    file->Close();

    return result;
}

Expected<std::string> files::read_text(const vfspp::IFilePtr& file)
{
    auto bytes_result = read_bytes(file);

    if(!bytes_result) {
        return bytes_result.error();
    }

    auto& data = bytes_result.value();

    return std::string(data.begin(), data.end());
}

errors::FileSystemResult files::append_bytes(const IFileSystem& zip, const std::string& name, const ByteArray& bytes)
{
    auto file = zip->OpenFile(name, vfspp::IFile::FileMode::Append);
    if(!file) {
        return errors::UnableToOpen;
    }

    auto result = append_bytes(file, bytes);
    file->Close();

    return result;
}

errors::FileSystemResult files::append_bytes(const vfspp::IFilePtr& file, const ByteArray& bytes)
{
    bool should_pop_mode = false;

    if(file->IsOpened()) {
        if(!internal::check_file_flags(file, vfspp::IFile::FileMode::Append | vfspp::IFile::FileMode::Truncate)) {
            file->PushMode(vfspp::IFile::FileMode::Append | vfspp::IFile::FileMode::Truncate);
            should_pop_mode = true;
        }
    } else {
        file->Open(vfspp::IFile::FileMode::Append | vfspp::IFile::FileMode::Truncate);
    }

    auto bytes_written = file->Write(bytes.data(), bytes.size());

    if(should_pop_mode) {
        file->PopMode();
    }

    if(bytes_written != bytes.size()) {
        return errors::UnableToWrite;
    }

    return errors::OK;
}

errors::FileSystemResult files::write_bytes(const IFileSystem& zip, const std::string& name, const ByteArray& bytes)
{
    auto file = zip->OpenFile(name, vfspp::IFile::FileMode::Write | vfspp::IFile::FileMode::Truncate);
    if(!file) {
        return errors::UnableToOpen;
    }

    auto result = write_bytes(file, bytes);

    file->Close();

    return result;
}

errors::FileSystemResult files::write_bytes(const vfspp::IFilePtr& file, const ByteArray& bytes)
{
    bool should_pop_mode = false;

    if(file->IsOpened()) {
        if(!internal::check_file_flags(file, vfspp::IFile::FileMode::Write | vfspp::IFile::FileMode::Truncate)) {
            file->PushMode(vfspp::IFile::FileMode::Write | vfspp::IFile::FileMode::Truncate);
            should_pop_mode = true;
        }
    } else {
        file->Open(vfspp::IFile::FileMode::Write | vfspp::IFile::FileMode::Truncate);
    }

    auto bytes_written = file->Write(bytes.data(), bytes.size());

    if(bytes_written != bytes.size()) {
        return errors::UnableToWrite;
    }

    if(should_pop_mode) {
        file->PopMode();
    }

    return errors::OK;
}

errors::FileSystemResult files::append_text(const IFileSystem& zip, const std::string& name, const std::string& text)
{
    auto file = zip->OpenFile(vfspp::FileInfo(name), vfspp::IFile::FileMode::Append | vfspp::IFile::FileMode::Truncate);
    if(!file) {
        return errors::UnableToOpen;
    }

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

errors::FileSystemResult files::write_text(const IFileSystem& zip, const std::string& name, const std::string& text)
{
    auto file = zip->OpenFile(vfspp::FileInfo(name), vfspp::IFile::FileMode::Write | vfspp::IFile::FileMode::Truncate);
    if(!file) {
        return errors::UnableToOpen;
    }

    auto result = write_text(file, text);

    file->Close();

    return result;
}

errors::FileSystemResult files::write_text(const vfspp::IFilePtr& file, const std::string& text)
{
    auto text_bytes = internal::text_to_bytes(text);

    auto result = write_bytes(file, text_bytes);

    return result;
}

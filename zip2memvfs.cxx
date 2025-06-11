#define _CRT_SECURE_NO_WARNINGS

#include <ranges>

#include "zip2memvfs.hxx"

#include "logdef.hxx"
#include "security.hxx"
#include "scope_guard.hxx"

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

bool is_file_dir(const vfspp::IFilePtr& file)
{
    auto info = file->GetFileInfo();
    auto name = info.AbsolutePath();

    return name.ends_with("/");
}

bool has_subdirectory(const IFileSystem& fs, const vfspp::FileInfo& info)
{
    // todo: maybe there is a more accurate way to do this?
    for(auto& file: fs->FileList() | std::views::values) {
        if(file->GetFileInfo().AbsolutePath().starts_with(info.AbsolutePath())) {
            return true;
        }
    }

    return false;
}

Expected<vfspp::MemoryFileSystemPtr> fs_map(const vfspp::ZipFileSystemPtr& zip)
{
    auto mem_fs = std::make_shared<vfspp::MemoryFileSystem>();
    mem_fs->Initialize();

    if(!mem_fs->IsInitialized()) {
        return errors::Error("Unable to create a memory-mapped file system");
    }

    for(const auto &file : zip->FileList() | std::views::values) {
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

        fs->Shutdown();

        return ifs;
    }

    return errors::Error(std::format("Unable to open archive {}", name));
}

errors::FileSystemResult files::save_to_zip(const fs::path& path, const vfspp::IFileSystemPtr& fs, bool overwrite)
{
    if(!fs || !fs->IsInitialized()) {
        return errors::UnableToWrite;
    }

    if(fs::exists(path) && !overwrite) {
        return errors::UnableToWrite;
    }

    fs::path temp_dir = fs::temp_directory_path();
    fs::path temp_file = temp_dir / std::format("luabot_tmp_{}.zip", std::to_string(std::time(nullptr)));

    try {
        mz_zip_archive zip_archive = {};

        if(!mz_zip_writer_init_file(&zip_archive, temp_file.string().c_str(), 0)) {
            return errors::UnableToWrite;
        }

        const auto& file_list = fs->FileList();

        auto write_success = true;

        for(const auto &file : file_list | std::views::values) {
            if(!file) {
                continue;
            }

            auto& info = file->GetFileInfo();

            if(info.IsDir()) {
                continue;
            }

            auto content_result = read_bytes(file);
            if(!content_result) {
                luabot_logErr("Failed to read file: {}", info.AbsolutePath());
                write_success = false;
                break;
            }

            auto& content = content_result.value();
            auto archive_path = info.AbsolutePath();

            if(archive_path.starts_with("/")) {
                archive_path = archive_path.substr(1);
            }

            if(!mz_zip_writer_add_mem(&zip_archive, archive_path.c_str(), content.data(), content.size(), MZ_DEFAULT_COMPRESSION)) {
                luabot_logErr("Failed to add file to archive: {}", archive_path);
                write_success = false;
                break;
            }
        }

        if(write_success && !mz_zip_writer_finalize_archive(&zip_archive)) {
            write_success = false;
        }

        mz_zip_writer_end(&zip_archive);

        if(!write_success) {
            fs::remove(temp_file);
            return errors::UnableToWrite;
        }

        fs::rename(temp_file, path);

        fs::remove(temp_file);

        return errors::OK;

    } catch(std::exception& e) {
        luabot_logErr("Failed writing archive: {}", e.what());

        if(fs::exists(temp_file)) {
            fs::remove(temp_file);
        }

        return errors::UnableToWrite;
    }
}


Expected<files::IFileSystem> files::open_subdir(const IFileSystem& zip, const std::string& directory, bool readonly)
{
    vfspp::FileInfo dir_info(directory);
    if(!zip->IsDir(dir_info) && !internal::has_subdirectory(zip, vfspp::FileInfo(directory))) {
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

    bool was_opened = false;
    scope_guard { if(was_opened) { file->Close(); } };

    if(!file->IsOpened()) {
        file->Open(vfspp::IFile::FileMode::Read);
        was_opened = true;
    }

    data.resize(file->Size());

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

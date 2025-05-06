#include "zip2memvfs.hxx"

files::SubDirectory::SubDirectory(vfspp::IFileSystemPtr base_fs, std::string_view path, bool readonly)
    : _origin_fs(std::move(base_fs)), _prefix(path), _readonly(readonly)
{ }

void files::SubDirectory::Initialize()  { }

void files::SubDirectory::Shutdown()
{
    _cached_files.clear();
}

bool files::SubDirectory::IsInitialized() const 
{
    return _origin_fs && _origin_fs->IsInitialized();
}

const std::string& files::SubDirectory::BasePath() const 
{
    return _prefix;
}

const vfspp::IFileSystem::TFileList& files::SubDirectory::FileList() const
{
    static TFileList filtered_list;
    filtered_list.clear();

    const auto& base_list = _origin_fs->FileList();

    for(const auto& [path, file] : base_list) {
        if(path.starts_with(_prefix + "/")) {
            std::string local_path = to_local_path(path);
            filtered_list[local_path] = file;
        }
    }

    return filtered_list;
}

bool files::SubDirectory::IsReadOnly() const 
{
    return _readonly;
}

vfspp::IFilePtr files::SubDirectory::OpenFile(const vfspp::FileInfo& filePath, vfspp::IFile::FileMode mode) 
{
    std::string full_path = to_full_path(filePath.AbsolutePath());

    if(_cached_files.contains(filePath.AbsolutePath())) {
        return _cached_files[filePath.AbsolutePath()];
    }

    vfspp::FileInfo full_info(full_path);
    auto file = _origin_fs->OpenFile(full_info, mode);

    if(file) {
        _cached_files[filePath.AbsolutePath()] = file;
    }

    return file;
}

void files::SubDirectory::CloseFile(vfspp::IFilePtr file) 
{
    if(!file) {
        return;
    }

    for(auto it = _cached_files.begin(); it != _cached_files.end(); ++it) {
        if(it->second == file) {
            _cached_files.erase(it);
            break;
        }
    }

    _origin_fs->CloseFile(file);
}

bool files::SubDirectory::CreateFile(const vfspp::FileInfo& filePath) 
{
    std::string full_path = to_full_path(filePath.AbsolutePath());
    vfspp::FileInfo full_info(full_path);

    return _origin_fs->CreateFile(full_info);
}

bool files::SubDirectory::CopyFile(const vfspp::FileInfo& src, const vfspp::FileInfo& dest) 
{
    std::string full_src = to_full_path(src.AbsolutePath());
    std::string full_dest = to_full_path(dest.AbsolutePath());

    vfspp::FileInfo src_info(full_src);
    vfspp::FileInfo dest_info(full_dest);

    return _origin_fs->CopyFile(src_info, dest_info);
}

bool files::SubDirectory::IsFile(const vfspp::FileInfo& filePath) const 
{
    std::string full_path = to_full_path(filePath.AbsolutePath());
    vfspp::FileInfo full_info(full_path);

    return _origin_fs->IsFile(full_info);
}

bool files::SubDirectory::IsFileExists(const vfspp::FileInfo& filePath) const
{
    std::string full_path = to_full_path(filePath.AbsolutePath());
    vfspp::FileInfo full_info(full_path);

    return _origin_fs->IsFileExists(full_info);
}

bool files::SubDirectory::RemoveFile(const vfspp::FileInfo& filePath)
{
    std::string full_path = to_full_path(filePath.AbsolutePath());
    vfspp::FileInfo full_info(full_path);

    return _origin_fs->RemoveFile(full_info);
}

bool files::SubDirectory::RenameFile(const vfspp::FileInfo& src, const vfspp::FileInfo& dest) 
{
    std::string full_src = to_full_path(src.AbsolutePath());
    std::string full_dest = to_full_path(dest.AbsolutePath());

    vfspp::FileInfo src_info(full_src);
    vfspp::FileInfo dest_info(full_dest);

    return _origin_fs->RenameFile(src_info, dest_info);
}

bool files::SubDirectory::IsDir(const vfspp::FileInfo& dirPath) const 
{
    std::string full_path = to_full_path(dirPath.AbsolutePath());
    vfspp::FileInfo full_info(full_path);

    return _origin_fs->IsDir(full_info);
}

std::string files::SubDirectory::clean_path(std::string_view path)
{
    std::string cleaned(path);

    auto new_end = std::ranges::unique(cleaned,
        [](char a, char b) { return a == '/' && b == '/'; }).begin();
    cleaned.erase(new_end, cleaned.end());

    if(cleaned.length() > 1 && cleaned.back() == '/') {
        cleaned.pop_back();
    }

    if(cleaned.empty() || cleaned[0] != '/') {
        cleaned = "/" + cleaned;
    }

    return cleaned;
}

std::string files::SubDirectory::to_full_path(std::string_view path) const
{
    std::string cleaned = clean_path(path);

    if(_prefix.empty()) {
        return cleaned;
    }

    return _prefix + cleaned;
}

std::string files::SubDirectory::to_local_path(std::string_view path) const
{
    std::string cleaned = clean_path(path);

    if(_prefix.empty()) {
        return cleaned;
    }

    if(!cleaned.starts_with(_prefix + "/")) {
        return cleaned;
    }

    return cleaned.substr(_prefix.length());
}

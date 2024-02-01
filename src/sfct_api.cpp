#include "sfct_api.hpp"

bool sfct_api::IsFileAvailable(path filepath)
{
    if(std::filesystem::is_regular_file(filepath)){
        std::fstream file;
        file.open(filepath, std::ifstream::in | std::ifstream::binary);
        if(file.is_open()){
            file.close();
            return true;
        }
        return false;
    }
    else{
        // is a directory or something else
        return true;
    }
}

void sfct_api::FileCheck(path filepath)
{
    while(!IsFileAvailable(filepath)){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

bool sfct_api::CheckDirectory(path dir)
{
    if(!fs::is_directory(dir)){
        application::logger log(App_MESSAGE("Invalid Directory"),application::Error::WARNING,dir);
        log.to_console();
        log.to_log_file();
        return false;
    }
    return true;
}

bool sfct_api::CDirectory(path src)
{
    fs::path dir = src;
    if(dir.has_filename()){
        dir.remove_filename();
    }

    if(!fs::is_directory(dir) && !fs::create_directories(dir)){
        application::logger log(App_MESSAGE("Failed to create directories"),application::Error::WARNING,dir);
        log.to_console();
        log.to_log_file();
        return false;
    }
    return true;
}

std::optional<sfct_api::fs::path> sfct_api::CreateFileRelativePath(path src, path dst)
{
    // if src does not exist return nothing
    if(!fs::exists(src)){
        return std::nullopt;
    } 

    // if src is a file path remove the file name
    fs::path src_dir(src);
    if(src_dir.has_filename()){
        src_dir.remove_filename();
    }

    // if dst is a file path remove the file name
    fs::path dst_dir(dst);
    if(dst_dir.has_filename()){
        dst_dir.remove_filename();
    }

    // get the relative path
    // relative path can fail if src_dir and dst_dir have no common paths
    std::error_code ec;
    fs::path relativePath = fs::relative(src_dir,dst_dir,ec);
    
    if(ec){
        application::logger log(ec,application::Error::WARNING,src_dir);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    // the file destination directory with the directory new tree from src
    fs::path file_dst = dst_dir/relativePath;

    // if it fails to create the relative directory return nothing
    if(!CDirectory(file_dst)){
        return std::nullopt;
    } 

    // if everything works out and their is no errors return the new path
    // useful for copying to a new directory tree  
    return file_dst;
}

bool sfct_api::CopyFileCreatePath(path src, path dst, fs::copy_options co)
{
    // src must exist and be a regular file to be copyable
    if(!fs::exists(src) || !fs::is_regular_file(src)){
        return false;
    }
    
    // create the directories needed for the copy operation
    std::optional<fs::path> new_dst = CreateFileRelativePath(src,dst);
    
    // in case copy operation fails we can log the error 
    std::error_code ec;
    
    // if CreateFileRelativePath succeeds attempt to copy the file
    if(new_dst.has_value()){
        fs::copy_file(src,new_dst.value(),co,ec);
    }
    else{
        return false;
    }

    // if ec has value log the error and return false
    if(ec){
        application::logger log(ec,application::Error::WARNING,src);
        log.to_console();
        log.to_log_file();
        return false;
    }

    // passed all checks and successfully copied the file
    return true;
}

bool sfct_api::CreateDirectoryTree(path src, path dst)
{
    // src must be a directory and exist.
    // dst must be a directory and exist.
    if(!fs::is_directory(src) || !fs::is_directory(dst)){
        return false;
    }

    // this may need to be further optimized in the future
    // CreateFileRelativePath is not a slow function but many calls add up depending on src directory size
    for(const auto& entry: fs::recursive_directory_iterator(src)){
        CreateFileRelativePath(entry.path(),dst);
    }

    // function may succeed but it could be the case that some or all directories failed to be created
    // the errors will be in the log file or console
    return true;
}

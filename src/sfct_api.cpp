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

std::optional<sfct_api::fs::path> sfct_api::GetRelativeFilePath(path file,path base)
{
    // if file is not a file, log it and return nothing
    if(!fs::is_regular_file(file)){
        application::logger log(App_MESSAGE("Not a valid file"),application::Error::WARNING,file);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    // if base is not present and is not a directory on the system, log it and return nothing
    if(!fs::is_directory(base)){
        application::logger log(App_MESSAGE("Not a valid directory on system"),application::Error::WARNING,base);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    fs::path relative_path;

    // 'root_path()' gives the root directory or drive on Windows, e.g., "C:/" or "/"
    // 'relative_path()' gives the part of the path relative to the root directory
    // Combining them without the root gives a path relative to the root directory
    if (file.has_root_path()) {
        relative_path = file.relative_path();
    } else {
        // The path is already relative
        relative_path = file;
    }

    return base/relative_path;
}

std::optional<sfct_api::fs::path> sfct_api::GetRelativePath(path entry, path base)
{
    // if entry is not present on the system, log it and return nothing
    if(!fs::exists(entry)){
        application::logger log(App_MESSAGE("Not a valid system path"),application::Error::WARNING,entry);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    // if base is not present and is not a directory on the system, log it and return nothing
    if(!fs::is_directory(base)){
        application::logger log(App_MESSAGE("Not a valid directory on system"),application::Error::WARNING,base);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    fs::path relative_path;

    // 'root_path()' gives the root directory or drive on Windows, e.g., "C:/" or "/"
    // 'relative_path()' gives the part of the path relative to the root directory
    // Combining them without the root gives a path relative to the root directory
    if (entry.has_root_path()) {
        relative_path = entry.relative_path();
    } else {
        // The path is already relative
        relative_path = entry;
    }

    return base/relative_path;
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
    std::optional<fs::path> relativePath = GetRelativePath(src_dir,dst_dir);
    if(!relativePath.has_value()){
        return std::nullopt;
    }

    // the file destination directory with the directory new tree from src
    fs::path file_dst = dst_dir/relativePath.value();

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

std::optional<double_t> sfct_api::FileGetTransferRate(path src)
{
    // if the src path is not a file log it and return nothing
    if(!fs::is_regular_file(src)){
        application::logger log(App_MESSAGE("Not a file"),application::Error::WARNING,src);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }
    
    // setup a benchmark to test the speed
    application::benchmark test;

    // for logging errors
    std::error_code ec_1,ec_2;

    // begin timer
    test.start_clock();

    // get the initial file size
    std::uintmax_t filesize = fs::file_size(src,ec_1);

    // wait 10ms for the transfer
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // get the change in file size
    std::uintmax_t deltafilesize = fs::file_size(src,ec_2)-filesize;

    // end timer
    test.end_clock();

    // get the rate in MB/s
    double_t rate = test.speed(deltafilesize);

    // if there was an error log it
    // return nothing
    if(ec_1){
        application::logger log(ec_1,application::Error::WARNING,src);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    // if there was an error log it
    // return nothing
    if(ec_2){
        application::logger log(ec_2,application::Error::WARNING,src);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    // if the file size didnt change
    // return nothing to indicate the file is not being transfered
    if(rate == 0.0){
        return std::nullopt;
    }

    // if no errors and rate has a value not equal to zero
    // return the transfer rate in MB/s
    return rate;
}

std::optional<std::shared_ptr<std::unordered_set<sfct_api::fs::path>>> sfct_api::GetDifferenceBetweenDirectoriesSingle(path d1, path d2)
{
    if(!fs::is_directory(d1)){
        application::logger log(App_MESSAGE("Invalid directory"),application::Error::WARNING,d1);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }
    
    if(!fs::is_directory(d2)){
        application::logger log(App_MESSAGE("Invalid directory"),application::Error::WARNING,d2);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    std::unordered_set<fs::path> unique_paths,d1_paths,d2_paths;

    for(const auto& entry:fs::directory_iterator(d1)){
        std::optional<fs::path> relative_path = GetRelativePath(entry.path(),d2);
        if(relative_path.has_value()){
            d1_paths.emplace(relative_path.value());
        }
    }
}

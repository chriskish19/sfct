#include "sfct_api.hpp"

bool sfct_api::is_file_available(path filepath)
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
        // is a directory, something else or not a valid path
        return true;
    }
}

void sfct_api::file_check(path filepath)
{
    while(!is_file_available(filepath)){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

bool sfct_api::check_directory(path dir)
{
    if(!fs::is_directory(dir)){
        application::logger log(App_MESSAGE("Invalid Directory"),application::Error::WARNING,dir);
        log.to_console();
        log.to_log_file();
        return false;
    }
    return true;
}

bool sfct_api::create_directory(path src)
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

std::optional<sfct_api::fs::path> sfct_api::get_relative_file_path(path file,path base)
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

    return ext::get_relative_path(file,base);
}

std::optional<sfct_api::fs::path> sfct_api::get_relative_path(path entry, path base)
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

    return ext::get_relative_path(entry,base);
}

std::optional<sfct_api::fs::path> sfct_api::create_file_relative_path(path src, path dst)
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

    return ext::create_file_relative_path(src_dir,dst_dir);
}

bool sfct_api::copy_file_create_path(path src, path dst, fs::copy_options co)
{
    // src must exist and be a regular file to be copyable
    if(!fs::exists(src) || !fs::is_regular_file(src)){
        return false;
    }
    
    // create the directories needed for the copy operation
    std::optional<fs::path> new_dst = ext::create_file_relative_path(src,dst);
    
    // if create_file_relative_path succeeds attempt to copy the file
    if(new_dst.has_value()){
        return ext::copy_file(src,new_dst.value(),co);
    }
    
    return false;
}

bool sfct_api::create_directory_tree(path src, path dst)
{
    // src must be a directory and exist.
    // dst must be a directory and exist.
    if(!fs::is_directory(src) || !fs::is_directory(dst)){
        return false;
    }

    // this may need to be further optimized in the future
    // create_file_relative_path is not a slow function but many calls add up depending on src directory size
    for(const auto& entry: fs::recursive_directory_iterator(src)){
        ext::create_file_relative_path(entry.path(),dst);
    }

    // function may succeed but it could be the case that some or all directories failed to be created
    // the errors will be in the log file or console
    return true;
}

std::optional<double_t> sfct_api::file_get_transfer_rate(path src)
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

    // begin timer
    test.start_clock();

    // get the initial file size
    std::optional<std::uintmax_t> filesize = ext::get_file_size(src);

    // if no value returned return nothing
    if(!filesize.has_value()) 
        return std::nullopt;

    // wait 10ms for the transfer
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // get the change in file size
    std::optional<std::uintmax_t> newfilesize = ext::get_file_size(src);
    
    // if no value returned return nothing
    if(!newfilesize.has_value()) 
        return std::nullopt;
    
    // get the change in file size
    std::uintmax_t deltafilesize = newfilesize.value() - filesize.value();

    // end timer
    test.end_clock();

    // get the rate in MB/s
    double_t rate = test.speed(deltafilesize);

    // if the file size didnt change
    // return nothing to indicate the file is not being transfered
    if(rate == 0.0){
        return std::nullopt;
    }

    // if no errors and rate has a value not equal to zero
    // return the transfer rate in MB/s
    return rate;
}

std::optional<std::shared_ptr<std::unordered_set<sfct_api::fs::path>>> sfct_api::get_directory_differences_single(path d1, path d2)
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
        std::optional<fs::path> relative_path = ext::get_relative_path(entry.path(),d2);
        if(relative_path.has_value()){
            d1_paths.emplace(relative_path.value());
        }
    }


}

std::optional<sfct_api::fs::path> sfct_api::ext::get_relative_path(path entry, path base)
{
    application::path_ext _p = private_get_relative_path(entry,base);
    if(_p.e){
        application::logger log(_p.e,application::Error::WARNING,entry);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }
    return _p.p;
}

std::optional<std::uintmax_t> sfct_api::ext::get_file_size(path entry)
{
    application::file_size_ext _fse = private_get_file_size(entry);
    if(_fse.e){
        application::logger log(_fse.e,application::Error::WARNING,entry);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }
    return _fse.size;
}

bool sfct_api::ext::copy_file(path src, path dst, fs::copy_options co)
{
    application::copy_file_ext _cfe = private_copy_file(src,dst,co);
    if(_cfe.e){
        application::logger log(_cfe.e,application::Error::WARNING,src);
        log.to_console();
        log.to_log_file();
        return false;
    }
    return true;
}

sfct_api::fs::path sfct_api::ext::combine_path_tree(path entry, path base)
{
    fs::path _entry;
    if(entry.has_root_path()){
        // remove root path
        _entry = entry.relative_path();
    }
    else{
        _entry = entry;
    }
    return base/_entry;

}

std::optional<sfct_api::fs::path> sfct_api::ext::create_file_relative_path(path src, path dst)
{
    fs::path file_dst;

    // if the root paths equal, get the relative path
    if(src.root_path() == dst.root_path()){
        // get the relative path
        std::optional<fs::path> relativePath = ext::get_relative_path(src,dst);
        if(!relativePath.has_value()){
            return std::nullopt;
        }
        file_dst = dst/relativePath.value();
    }
    else{
        file_dst = ext::combine_path_tree(src,dst);
    }

    
    // if it fails to create the relative directory return nothing
    if(!ext::create_directory(file_dst).has_value()){
        return std::nullopt;
    } 

    // if everything works out and their is no errors return the new path
    // useful for copying to a new directory tree  
    return file_dst;
}

std::optional<bool> sfct_api::ext::create_directory(path dir)
{
    std::error_code e;
    if(fs::create_directories(dir,e)){
        return true;
    }

    if(e){
        application::logger log(e,application::Error::WARNING,dir);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }
    return false;
}

application::path_ext sfct_api::ext::private_get_relative_path(path entry, path base)
{
    application::path_ext _p;
    _p.p = fs::relative(entry,base,_p.e);
    return _p;
}

application::file_size_ext sfct_api::ext::private_get_file_size(path entry)
{
    application::file_size_ext _fse;
    _fse.size = fs::file_size(entry,_fse.e);
    return _fse;
}

application::copy_file_ext sfct_api::ext::private_copy_file(path src, path dst, fs::copy_options co)
{
    application::copy_file_ext _cfe;
    _cfe.rv = fs::copy_file(src,dst,co,_cfe.e);
    return _cfe;
}


#include "sfct_api.hpp"

bool sfct_api::is_file_available(path filepath)
{
    return ext::is_file_available(filepath);
}

bool sfct_api::file_check(path filepath)
{
    if(!fs::is_regular_file(filepath)){
        return false;
    }
    
    while(ext::is_file_in_transit(filepath)){}
    
    return ext::is_file_available(filepath);
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

bool sfct_api::create_directory_paths(path src)
{
    if(fs::exists(src)){
        return false;
    }
    
    fs::path dir = src;
    if(dir.has_extension()){
        dir.remove_filename();
    }

    std::optional<bool> succeded = ext::create_directory_paths(dir);

    if(succeded.has_value()){
        return succeded.value();
    }
    return false;
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

std::optional<sfct_api::fs::path> sfct_api::create_file_relative_path(path src, path dst,path src_base)
{
    // if src does not exist return nothing
    if(!fs::exists(src)){
        return std::nullopt;
    } 

    // if src_base if specified it must be a directory on the system
    if(!src_base.empty() && !fs::is_directory(src_base)){
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

    return ext::create_file_relative_path(src_dir,dst_dir,src_base);
}

bool sfct_api::copy_file_create_relative_path(path src, path dst, fs::copy_options co)
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
        ext::create_file_relative_path(entry.path(),dst,src);
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
    
    return ext::file_get_transfer_rate(src);
}

bool sfct_api::copy_file(path src, path dst, fs::copy_options co)
{
    if(!fs::is_regular_file(src)){
        application::logger log(App_MESSAGE("Not a valid file path"),application::Error::WARNING,src);
        log.to_console();
        log.to_log_file();
        return false;
    }

    if(!fs::exists(dst)){
        application::logger log(App_MESSAGE("Not a valid path"),application::Error::WARNING,dst);
        log.to_console();
        log.to_log_file();
        return false;
    }

    return ext::copy_file(src,dst,co);
}

bool sfct_api::copy_file_create_path(path src, path dst, fs::copy_options co)
{
    fs::path dst_dir = dst;
    if(dst_dir.has_extension()){
        dst_dir.remove_filename();
    }
    
    // src must exist and be a regular file to be copyable
    if(!fs::is_regular_file(src)){
        return false;
    }
    
    // create the directories needed for the copy operation
    std::optional<bool> succeded = ext::create_directory_paths(dst_dir);
    
    // if create_file_relative_path succeeds attempt to copy the file
    if(succeded.has_value()){
        return ext::copy_file(src,dst,co);
    }
    
    return false;
}

std::uintmax_t sfct_api::remove_all(path dir)
{
    if(!fs::is_directory(dir)){
        return 0;
    }

    return ext::remove_all(dir);
}

bool sfct_api::remove_file(path file)
{
    if(!fs::is_regular_file(file)){
        return false;
    }
    return ext::remove_file(file);
}

bool sfct_api::copy_symlink(path src_link, path dst, fs::copy_options co)
{
    if(!fs::is_symlink(src_link) || !fs::exists(dst)){
        return false;
    }

    return ext::copy_symlink(src_link,dst,co);
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

std::optional<sfct_api::fs::path> sfct_api::ext::create_file_relative_path(path src, path dst,path src_base)
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
    else if(!src_base.empty()){
        // get the relative path
        std::optional<fs::path> relativePath = ext::get_relative_path(src,src_base);
        if(!relativePath.has_value()){
            return std::nullopt;
        }
        file_dst = dst/relativePath.value();
    }
    else{
        file_dst = dst/ext::get_last_folder(src);
    }
    
    // if it fails to create the relative directory return nothing
    if(!ext::create_directory_paths(file_dst).has_value()){
        return std::nullopt;
    } 

    // if everything works out and their is no errors return the new path
    // useful for copying to a new directory tree  
    return file_dst;
}

std::optional<bool> sfct_api::ext::create_directory_paths(path dir)
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

sfct_api::fs::path sfct_api::ext::get_last_folder(path entry)
{
    // Get the parent path of the entry
    fs::path parent_path = entry.parent_path();
    
    // Now get the last component of the parent path, which is the immediate directory name
    return parent_path.filename();
}

bool sfct_api::ext::remove_file(path file)
{
    application::remove_file_ext _rfe = private_remove_file(file);
    if(_rfe.e){
        application::logger log(_rfe.e,application::Error::WARNING,file);
        log.to_console();
        log.to_log_file();
        return false;
    }
    return _rfe.rv;
}

std::uintmax_t sfct_api::ext::remove_all(path dir)
{
    application::remove_file_ext _rfe = private_remove_all(dir);
    if(_rfe.e){
        application::logger log(_rfe.e,application::Error::WARNING,dir);
        log.to_console();
        log.to_log_file();
    }
    return _rfe.files_removed;
}

std::optional<double_t> sfct_api::ext::file_get_transfer_rate(path filepath)
{
    // setup a benchmark to test the speed
    application::benchmark test;

    // begin timer
    test.start_clock();

    // get the initial file size
    std::optional<std::uintmax_t> filesize = ext::get_file_size(filepath);

    // if no value returned return nothing
    if(!filesize.has_value()) 
        return std::nullopt;

    // wait 10ms for the transfer
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // get the change in file size
    std::optional<std::uintmax_t> newfilesize = ext::get_file_size(filepath);
    
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

bool sfct_api::ext::copy_symlink(path src_link,path dst,fs::copy_options co)
{
    fs::path target = fs::read_symlink(src_link);
    return ext::copy_file(target,dst,co);
}

bool sfct_api::ext::is_file_available(path filepath)
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

bool sfct_api::ext::is_file_in_transit(path filepath)
{
    std::chrono::time_point t1 = fs::last_write_time(filepath);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    std::chrono::time_point t2 = fs::last_write_time(filepath);
    if(t1 == t2){
        return false;
    }
    else{
        return true;
    }
}

application::remove_file_ext sfct_api::ext::private_remove_all(path dir)
{
    application::remove_file_ext _rfe;
    _rfe.files_removed = fs::remove_all(dir,_rfe.e);
    return _rfe;
}

application::remove_file_ext sfct_api::ext::private_remove_file(path file)
{
    application::remove_file_ext _rfe;
    _rfe.rv = fs::remove(file,_rfe.e);
    return _rfe;
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


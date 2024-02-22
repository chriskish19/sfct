#include "sfct_api.hpp"

bool sfct_api::is_entry_available(path entry)
{
    if(fs::exists(entry)){
        return ext::is_entry_available(entry);
    }
    return false;
}

bool sfct_api::entry_check(path entry)
{   
    if(!fs::exists(entry)){
        return false;
    }
    
    if(!ext::is_entry_available(entry)){
        while(ext::is_entry_in_transit(entry)){}
    }
    
    return ext::is_entry_available(entry);
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

std::optional<sfct_api::fs::path> sfct_api::create_file_relative_path(path src, path dst,path src_base,bool create_dir)
{
    // if src does not exist return nothing
    if(!fs::exists(src)){
        return std::nullopt;
    } 

    // if src_base is specified it must be a directory on the system
    if(!src_base.empty() && !fs::is_directory(src_base)){
        return std::nullopt;
    }

    return ext::create_relative_path(src,dst,src_base,create_dir);
}

bool sfct_api::copy_file_create_relative_path(path src, path dst, fs::copy_options co)
{
    // src must exist and be a regular file to be copyable
    if(!fs::exists(src) || !fs::is_regular_file(src)){
        return false;
    }
    
    // create the directories needed for the copy operation
    std::optional<fs::path> new_dst = ext::create_relative_path(src,dst);
    
    // if create_relative_path succeeds attempt to copy the file
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
    // create_relative_path is not a slow function but many calls add up depending on src directory size
    for(const auto& entry: fs::recursive_directory_iterator(src)){
        ext::create_relative_path(entry.path(),dst,src);
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

    fs::path dst_dir(dst);
    if(dst_dir.has_extension()){
        dst_dir.remove_filename();
    }

    if(!fs::exists(dst_dir)){
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
    
    // if create_relative_path succeeds attempt to copy the file
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

bool sfct_api::remove_entry(path entry)
{
    if(!fs::exists(entry)){
        return false;
    }
    
    return ext::remove_entry(entry);
}

bool sfct_api::copy_symlink(path src_link, path dst, fs::copy_options co)
{
    fs::path dst_dir = dst;
    if(dst_dir.has_extension()){
        dst_dir.remove_filename();
    }
    
    if(!fs::is_symlink(src_link) || !fs::exists(dst_dir)){
        return false;
    }

    return ext::copy_symlink(src_link,dst_dir,co);
}

sfct_api::fs::copy_options sfct_api::get_copy_options(application::cs commands) noexcept
{
    using namespace application;
    using fs_co = fs::copy_options;
    fs_co co{fs_co::none};
    
    if((commands & cs::recursive) != cs::none){
        co |= fs_co::recursive;
    }
    if((commands & cs::update) != cs::none){
        co |= fs_co::update_existing;
    }
    else if((commands & cs::overwrite) != cs::none){
        co |= fs_co::overwrite_existing; 
    } 
    // add more later
    return co;
}

bool sfct_api::recursive_flag_check(application::cs commands) noexcept
{
    return ((commands & application::cs::recursive) != application::cs::none);
}

void sfct_api::copy_entry(path src, path dst, fs::copy_options co)
{
    if(!fs::exists(src)){
        application::logger log(App_MESSAGE("src non exist"),application::Error::WARNING);
        log.to_console();
        log.to_log_file();
        return;
    }
    
    fs::path dst_dir = dst;
    if(fs::is_directory(src) && dst_dir.has_extension()){
        dst_dir.remove_filename();
    }

    return ext::copy_entry(src,dst_dir,co);
}

std::optional<std::shared_ptr<std::unordered_map<sfct_api::fs::path,sfct_api::fs::path>>> sfct_api::are_directories_synced(path src, path dst, bool recursive_sync)
{
    if(!fs::is_directory(src)){
        application::logger log(App_MESSAGE("invalid directory"),application::Error::WARNING,src);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    if(!fs::is_directory(dst)){
        application::logger log(App_MESSAGE("invalid directory"),application::Error::WARNING,dst);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    return ext::are_directories_synced(src,dst,recursive_sync);
}

std::optional<application::directory_info> sfct_api::get_directory_info(const application::copyto &dir)
{
    if(!fs::is_directory(dir.source)){
        return std::nullopt;
    }

    return ext::get_directory_info(dir);
}

void sfct_api::output_entry_to_console(const fs::directory_entry &entry,const size_t prev_entry_path_length)
{
    STRING s_clear(prev_entry_path_length,' ');
    STRING s_entry_path(entry.path());
    

    STDOUT << "\r" << App_MESSAGE("Processing entry: ") << s_clear;
    STDOUT << "\r" << App_MESSAGE("Processing entry: ") << s_entry_path;

    if(STDOUT.fail()){
        STDOUT.clear();
    }
}

void sfct_api::output_path_to_console(path p, const size_t prev_p_length)
{
    STRING s_clear(prev_p_length,' ');
    STRING s_path(p);
    

    STDOUT << "\r" << App_MESSAGE("Processing entry: ") << s_clear;
    STDOUT << "\r" << App_MESSAGE("Processing entry: ") << s_path;

    if(STDOUT.fail()){
        STDOUT.clear();
    }
}

std::optional<std::uintmax_t> sfct_api::get_entry_size(path entry)
{
    if(!fs::exists(entry)){
        return std::nullopt;
    }
    return ext::get_file_size(entry);
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

std::optional<sfct_api::fs::path> sfct_api::ext::create_relative_path(path src, path dst,path src_base,bool create_dir)
{
    fs::path file_dst;

    if(!src_base.empty()){
        auto relative_path = ext::get_relative_path(src,src_base);
        if(!relative_path.has_value()){
            return std::nullopt;
        }

        file_dst = dst/relative_path.value();
    }
    else{
        auto relative_path = ext::get_relative_path(src,dst);
        if(!relative_path.has_value()){
            return std::nullopt;
        }

        file_dst = relative_path.value();
    }





    
    // if it fails to create the relative directory return nothing
    // if create_dir is false, the ext::create_directory_paths(file_dst).has_value() 
    // part of the expression will not be evaluated due to the short-circuiting behavior of the logical AND operator (&&) in C++.
    if(create_dir && !ext::create_directory_paths(file_dst).has_value()){
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

std::optional<sfct_api::fs::path> sfct_api::ext::get_last_folder(path entry)
{
    fs::path parent_path;
    if(entry.has_parent_path()){
        parent_path = entry.parent_path();
    }
    else{
        return std::nullopt;
    }
    
    // Now get the last component of the parent path, which is the immediate directory name
    return parent_path.filename();
}

bool sfct_api::ext::remove_entry(path entry)
{
    application::remove_file_ext _rfe = private_remove_entry(entry);
    if(_rfe.e){
        application::logger log(_rfe.e,application::Error::WARNING,entry);
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

bool sfct_api::ext::is_entry_available(path entry)
{
    fs::file_status fs = fs::status(entry);

    switch(fs.type()){
        case std::filesystem::file_type::none:
            // skip for now
            break;
        case std::filesystem::file_type::not_found:
            // skip for now
            break;
        case std::filesystem::file_type::regular:
            return private_open_file(entry);
            break;
        case std::filesystem::file_type::directory:
            
            break;
        case std::filesystem::file_type::symlink:{
            fs::path target = fs::read_symlink(entry);
            return private_open_file(target);
            break;
        } 
        case std::filesystem::file_type::block:
            
            break;
        case std::filesystem::file_type::character:
            
            break;
        case std::filesystem::file_type::fifo:
            
            break;
        case std::filesystem::file_type::socket:
            
            break;
        case std::filesystem::file_type::unknown:

            break;
        default:
            
            break;
    }

    return false;
}

bool sfct_api::ext::is_entry_in_transit(path entry)
{
    std::chrono::time_point t1 = fs::last_write_time(entry);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    std::chrono::time_point t2 = fs::last_write_time(entry);
    if(t1 == t2){
        return false;
    }
    else{
        return true;
    }
}

void sfct_api::ext::copy_entry(path src, path dst, fs::copy_options co)
{
    std::error_code e;
    fs::copy(src,dst,co,e);
    if(e){
        application::logger log(e,application::Error::WARNING,src);
        log.to_console();
        log.to_log_file();
    }
}

std::optional<std::shared_ptr<std::unordered_map<sfct_api::fs::path,sfct_api::fs::path>>> sfct_api::ext::are_directories_synced(path src, path dst,bool recursive_sync)
{
    std::unordered_map<fs::path,fs::path> paths_mp; // key is dst, value is src

    if(recursive_sync){

        for(const auto& entry:fs::recursive_directory_iterator(src)){
            auto relative_path = ext::create_relative_path(entry.path(),dst,src,false);
            if(relative_path.has_value()){
                paths_mp.emplace(relative_path.value(),entry.path());
            }
        }

        for(const auto& entry:fs::recursive_directory_iterator(dst)){
            auto found = paths_mp.find(entry.path());
            if(found != paths_mp.end()){
                paths_mp.erase(found);
            }
        }

        if(paths_mp.empty()){
            return std::nullopt;
        }
        
        return std::make_shared<std::unordered_map<fs::path,fs::path>>(paths_mp);

    }
    else{

        for(const auto& entry:fs::directory_iterator(src)){
            fs::path entry_path = entry.path();
            fs::path relative_path;
            if(entry_path.has_filename()){
                relative_path = dst/entry_path.filename();
                paths_mp.emplace(relative_path,entry_path);
            }
        }

        for(const auto& entry:fs::directory_iterator(dst)){
            auto found = paths_mp.find(entry.path());
            if(found != paths_mp.end()){
                paths_mp.erase(found);
            }
        }

        if(paths_mp.empty()){
            return std::nullopt;
        }
        
        return std::make_shared<std::unordered_map<fs::path,fs::path>>(paths_mp);

    }
    
}

void sfct_api::ext::log_error_code(const std::error_code &e,path p)
{
    if(e){
        application::logger log(e,application::Error::WARNING,p);
        log.to_console();
        log.to_log_file();
    }
}

application::directory_info sfct_api::ext::get_directory_info(const application::copyto &dir)
{
    if(sfct_api::recursive_flag_check(dir.commands)){

        application::directory_info di{};
        for(const auto& entry:fs::recursive_directory_iterator(dir.source)){
            std::error_code e;
            di.TotalSize += entry.file_size(e);
            ext::log_error_code(e,entry.path());
            di.FileCount++;
        }

        di.AvgFileSize = static_cast<double_t>(di.TotalSize / di.FileCount);

        return di;

    }
    else{

        application::directory_info di{};
        for(const auto& entry:fs::directory_iterator(dir.source)){
            std::error_code e;
            di.TotalSize += entry.file_size(e);
            ext::log_error_code(e,entry.path());
            di.FileCount++;
        }

        di.AvgFileSize = static_cast<double_t>(di.TotalSize / di.FileCount);

        return di;
    }
}

bool sfct_api::ext::private_open_file(path filepath)
{
    std::fstream file;
    file.open(filepath, std::ifstream::in | std::ifstream::binary);
    if(file.is_open()){
        file.close();
        return true;
    }
    return false;
}

application::remove_file_ext sfct_api::ext::private_remove_all(path dir)
{
    application::remove_file_ext _rfe;
    _rfe.files_removed = fs::remove_all(dir,_rfe.e);
    return _rfe;
}

application::remove_file_ext sfct_api::ext::private_remove_entry(path entry)
{
    application::remove_file_ext _rfe;
    _rfe.rv = fs::remove(entry,_rfe.e);
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


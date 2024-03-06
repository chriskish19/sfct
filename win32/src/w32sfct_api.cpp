#include "w32sfct_api.hpp"

bool sfct_api::is_entry_available(path entry) noexcept
{
    if(ext::exists(entry)){
        return ext::is_entry_available(entry);
    }
    return false;
}

bool sfct_api::entry_check(path entry) noexcept
{   
    if(!ext::exists(entry)){
        return false;
    }
    
    return ext::entry_check(entry);
}

bool sfct_api::create_directory_paths(path src) noexcept
{
    if(ext::exists(src)){
        return false;
    }
    
    std::optional<bool> succeded = ext::create_directory_paths(src);

    if(succeded.has_value()){
        return succeded.value();
    }
    return false;
}

std::optional<sfct_api::fs::path> sfct_api::get_relative_file_path(path file,path base) noexcept
{
    // if file is not a file, log it and return nothing
    if(!ext::is_regular_file(file)){
        application::logger log(App_MESSAGE("Not a valid file"),application::Error::WARNING,file);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    // if base is not present and is not a directory on the system, log it and return nothing
    if(!ext::is_directory(base)){
        application::logger log(App_MESSAGE("Not a valid directory on system"),application::Error::WARNING,base);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    return ext::get_relative_path(file,base);
}

std::optional<sfct_api::fs::path> sfct_api::get_relative_path(path entry, path base) noexcept
{
    // if entry is not present on the system, log it and return nothing
    if(!ext::exists(entry)){
        application::logger log(App_MESSAGE("Not a valid system path"),application::Error::WARNING,entry);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    // if base is not present and is not a directory on the system, log it and return nothing
    if(!ext::is_directory(base)){
        application::logger log(App_MESSAGE("Not a valid directory on system"),application::Error::WARNING,base);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    return ext::get_relative_path(entry,base);
}

std::optional<sfct_api::fs::path> sfct_api::create_file_relative_path(path src, path dst,path src_base,bool create_dir) noexcept
{
    // if src does not exist return nothing
    if(!ext::exists(src)){
        return std::nullopt;
    } 

    // if src_base is specified it must be a directory on the system
    if(!src_base.empty() && !ext::is_directory(src_base)){
        return std::nullopt;
    }

    return ext::create_relative_path(src,dst,src_base,create_dir);
}

bool sfct_api::copy_file_create_relative_path(path src, path dst, fs::copy_options co) noexcept
{
    // src must exist and be a regular file to be copyable
    if(!ext::exists(src) || !ext::is_regular_file(src)){
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

bool sfct_api::create_directory_tree(path src, path dst) noexcept
{
    try{
		// src must be a directory and exist.
        // dst must be a directory and exist.
        if(!ext::is_directory(src) || !ext::is_directory(dst)){
            return false;
        }

        // this may need to be further optimized in the future
        // create_relative_path is not a slow function but many calls add up depending on src directory size
        for(const auto& entry: fs::recursive_directory_iterator(src)){
            ext::create_relative_path(entry.path(),dst,src,true);
        }

        // function may succeed but it could be the case that some or all directories failed to be created
        // the errors will be in the log file or console
        return true;
	}
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << "\n";

		return false;
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << "Runtime error :" << e.what() << "\n";
		
		return false;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << "Allocation error: " << e.what() << "\n";

		return false;
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << "\n";

		return false;
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught \n";

		return false;
	}
}

std::optional<double_t> sfct_api::file_get_transfer_rate(path src) noexcept
{
    // if the src path is not a file return nothing
    if(!ext::is_regular_file(src)){
        return std::nullopt;
    }
    
    return ext::file_get_transfer_rate(src);
}

bool sfct_api::copy_file(path src, path dst, fs::copy_options co) noexcept
{
    if(!ext::is_regular_file(src)){
        return false;
    }

    return ext::copy_file(src,dst,co);
}

bool sfct_api::copy_file_create_path(path src, path dst, fs::copy_options co) noexcept
{
    // src must exist and be a regular file to be copyable
    if(!ext::is_regular_file(src)){
        return false;
    }
    
    fs::path dst_dir = dst;
    if(dst_dir.has_filename()){
        dst_dir.remove_filename();
    }
    
    // create the directories needed for the copy operation
    std::optional<bool> succeded = ext::create_directory_paths(dst_dir);
    
    // if create_relative_path succeeds attempt to copy the file
    if(succeded.has_value()){
        return ext::copy_file(src,dst,co);
    }
    
    return false;
}

std::uintmax_t sfct_api::remove_all(path dir) noexcept
{
    if(!ext::is_directory(dir)){
        return 0;
    }

    return ext::remove_all(dir);
}

bool sfct_api::remove_entry(path entry) noexcept
{
    if(!ext::exists(entry)){
        return false;
    }
    
    return ext::remove_entry(entry);
}

bool sfct_api::copy_symlink(path src_link, path dst, fs::copy_options co) noexcept
{
    fs::path dst_dir = dst;
    if(dst_dir.has_extension()){
        dst_dir.remove_filename();
    }
    
    if(!ext::is_symlink(src_link)){
        return false;
    }

    ext::copy_symlink(src_link,dst_dir,co);
    return true;
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

void sfct_api::copy_entry(path src, path dst, fs::copy_options co,bool create_dst) noexcept
{
    if(create_dst){
        ext::create_directory_paths(dst);
    }
    
    return ext::copy_entry(src,dst,co);
}

std::optional<std::shared_ptr<std::unordered_map<sfct_api::fs::path,sfct_api::fs::path>>> sfct_api::are_directories_synced(path src, path dst, bool recursive_sync) noexcept
{
    if(!ext::is_directory(src)){
        application::logger log(App_MESSAGE("invalid directory"),application::Error::WARNING,src);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    if(!ext::is_directory(dst)){
        application::logger log(App_MESSAGE("invalid directory"),application::Error::WARNING,dst);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }

    return ext::are_directories_synced(src,dst,recursive_sync);
}

std::optional<application::directory_info> sfct_api::get_directory_info(const application::copyto &dir) noexcept
{
    if(!ext::is_directory(dir.source)){
        return std::nullopt;
    }

    return ext::get_directory_info(dir);
}

void sfct_api::output_entry_to_console(const fs::directory_entry &entry,const size_t prev_entry_path_length) noexcept
{
    try{
        STRING s_clear(prev_entry_path_length,' ');
        STRING s_entry_path(entry.path());
        

        STDOUT << "\r" << App_MESSAGE("Processing entry: ") << s_clear;
        STDOUT << "\r" << App_MESSAGE("Processing entry: ") << s_entry_path;

        if(STDOUT.fail()){
            STDOUT.clear();
            STDOUT << "\n";
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";
    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught \n";
    }
}

void sfct_api::output_path_to_console(path p, const size_t prev_p_length) noexcept
{
    try{
        STRING s_clear(prev_p_length,' ');
        STRING s_path(p);
        

        STDOUT << "\r" << App_MESSAGE("Processing entry: ") << s_clear;
        STDOUT << "\r" << App_MESSAGE("Processing entry: ") << s_path;

        if(STDOUT.fail()){
            STDOUT.clear();
            STDOUT << "\n";
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";
    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught \n";
    }
}

std::optional<std::uintmax_t> sfct_api::get_entry_size(path entry) noexcept
{
    if(!ext::exists(entry)){
        return std::nullopt;
    }
    return ext::get_file_size(entry);
}

void sfct_api::process_file_queue_info_entry(const application::file_queue_info &entry) noexcept
{
    sfct_api::to_console(App_MESSAGE("Processing entry: "),entry.src);

    switch(entry.fqs){
        case application::file_queue_status::file_added:{
            switch(entry.fs_src.type()){
                case std::filesystem::file_type::none:
                    // skip for now
                    break;
                case std::filesystem::file_type::not_found:
                    // skip for now
                    break;
                case std::filesystem::file_type::regular:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    
                    break;
                }
                case std::filesystem::file_type::directory:{
                    if(ext::exists(entry.src)){
                        sfct_api::create_directory_paths(entry.dst);
                    }
                    
                    break;
                }
                case std::filesystem::file_type::symlink:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    
                    break;
                }
                case std::filesystem::file_type::block:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    
                    break;
                }
                case std::filesystem::file_type::character:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    
                    break;
                }
                case std::filesystem::file_type::fifo:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    
                    break;
                }
                case std::filesystem::file_type::socket:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    
                    break;
                }
                case std::filesystem::file_type::unknown:
                    // skip
                    break;
                default:
                    // do nothing
                    break;
            }
            break;
        }
        case application::file_queue_status::file_updated:{
            switch(entry.fs_src.type()){
                case std::filesystem::file_type::none:
                    // skip for now
                    break;
                case std::filesystem::file_type::not_found:
                    // skip for now
                    break;
                case std::filesystem::file_type::regular:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    break;
                }
                case std::filesystem::file_type::directory:
                    // skip
                    break;
                case std::filesystem::file_type::symlink:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    
                    break;
                }
                case std::filesystem::file_type::block:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    
                    break;
                }
                case std::filesystem::file_type::character:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    
                    break;
                }
                case std::filesystem::file_type::fifo:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    
                    break;
                }
                case std::filesystem::file_type::socket:{
                    if(sfct_api::entry_check(entry.src)){
                        sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                    }
                    else{
                        application::logger log(App_MESSAGE("Skipping, File is in use: "),application::Error::INFO,entry.src);
                        log.to_console();
                        log.to_log_file();
                    }
                    
                    break;
                }
                case std::filesystem::file_type::unknown:
                    // do nothing
                    break;
                default:
                    // do nothing
                    break;
            }
            break;
        }
        case application::file_queue_status::file_removed:{
            switch(entry.fs_dst.type()){
                case std::filesystem::file_type::none:
                    // skip for now
                    break;
                case std::filesystem::file_type::not_found:
                    // skip for now
                    break;
                case std::filesystem::file_type::regular:{
                    sfct_api::remove_entry(entry.dst);
                    break;
                }
                case std::filesystem::file_type::directory:{
                    sfct_api::remove_all(entry.dst);
                    break;
                }
                case std::filesystem::file_type::symlink:
                    sfct_api::remove_entry(entry.dst);
                    break;
                case std::filesystem::file_type::block:
                    sfct_api::remove_entry(entry.dst);
                    break;
                case std::filesystem::file_type::character:
                    sfct_api::remove_entry(entry.dst);
                    break;
                case std::filesystem::file_type::fifo:
                    sfct_api::remove_entry(entry.dst);
                    break;
                case std::filesystem::file_type::socket:
                    sfct_api::remove_entry(entry.dst);
                    break;
                case std::filesystem::file_type::unknown:
                    // skip
                    break;
                default:
                    // skip
                    break;
            }
            break;
        }
        case application::file_queue_status::none:
            // do nothing
            break;
        default:
            // do nothing
            break;
    }
}

void sfct_api::rename_entry(path old_entry, path new_entry) noexcept
{
    if(!ext::exists(old_entry)){
        return;
    }

    ext::rename_entry(old_entry,new_entry);
}

bool sfct_api::is_directory(path entry) noexcept
{
    return ext::is_directory(entry);
}

bool sfct_api::exists(path entry) noexcept
{
    return ext::exists(entry);
}

sfct_api::fs::path sfct_api::get_current_path() noexcept
{
    auto curr_p = ext::get_current_path();
    if(curr_p.has_value()){
        return curr_p.value();
    }
    return fs::path();
}

std::optional<sfct_api::fs::file_status> sfct_api::get_file_status(path entry) noexcept
{
    if(!ext::exists(entry)){
        return std::nullopt;
    }

    return ext::file_status(entry);
}

void sfct_api::mt_process_file_queue_info_entry(application::file_queue_info entry) noexcept
{
    return process_file_queue_info_entry(entry);
}

void sfct_api::to_console(const STRING& message,path p) noexcept
{
    try{
        STDOUT << message << p << "\n";
    
        if(STDOUT.fail()){
            STDOUT.clear();
            STDOUT << "\n";
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";
    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught \n";
    }
}

bool sfct_api::ext::is_directory(path entry) noexcept
{
    application::is_entry_ext _is = private_is_directory(entry);
    if(_is.e){
        application::logger log(_is.e,application::Error::WARNING,entry);
        log.to_console();
        log.to_log_file();
    }
    return _is.rv;
}

bool sfct_api::ext::exists(path entry) noexcept
{
    application::is_entry_ext _is = private_exists(entry);
    if(_is.e){
        application::logger log(_is.e,application::Error::WARNING,entry);
        log.to_console();
        log.to_log_file();
    }
    return _is.rv;
}

bool sfct_api::ext::is_regular_file(path entry) noexcept
{
    application::is_entry_ext _is = private_is_regular_file(entry);
    if(_is.e){
        application::logger log(_is.e,application::Error::WARNING,entry);
        log.to_console();
        log.to_log_file();
    }
    return _is.rv;
}

bool sfct_api::ext::is_symlink(path entry) noexcept
{
    application::is_entry_ext _is = private_is_symlink(entry);
    if(_is.e){
        application::logger log(_is.e,application::Error::WARNING,entry);
        log.to_console();
        log.to_log_file();
    }
    return _is.rv;
}

std::optional<std::filesystem::file_time_type> sfct_api::ext::last_write_time(path entry) noexcept
{
    application::last_write_ext _lw = private_last_write_time(entry);
    if(_lw.e){
        application::logger log(_lw.e,application::Error::WARNING,entry);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }
    return _lw.t;
}

std::optional<sfct_api::fs::file_status> sfct_api::ext::file_status(path entry) noexcept
{
    application::file_status_ext _fs = private_file_status(entry);
    if(_fs.e){
        application::logger log(_fs.e,application::Error::WARNING,entry);
        log.to_console();
        log.to_log_file();
        return std::nullopt;
    }
    return _fs.s;
}

std::optional<sfct_api::fs::path> sfct_api::ext::get_current_path() noexcept
{
    auto _p = private_current_path();
    if(_p.has_value()){
        if(_p.value().e){
            application::logger log(_p.value().e,application::Error::WARNING,_p.value().p);
            log.to_console();
            log.to_log_file();
            return std::nullopt;
        }
        return _p.value().p;
    }
    return std::nullopt;
}

std::optional<application::path_ext> sfct_api::ext::private_current_path() noexcept
{
    try{
		application::path_ext _p;
        _p.p = fs::current_path(_p.e);
        return _p;
	}
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << "Runtime error :" << e.what() << "\n";
		
		return std::nullopt;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << "Allocation error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << "\n";

		return std::nullopt;
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught \n";

		return std::nullopt;
	}
}

application::file_status_ext sfct_api::ext::private_file_status(path entry) noexcept
{
    application::file_status_ext _fs;
    _fs.s = fs::status(entry,_fs.e);
    return _fs;
}

application::last_write_ext sfct_api::ext::private_last_write_time(path entry) noexcept
{
    application::last_write_ext _lw;
    _lw.t = fs::last_write_time(entry,_lw.e);
    return _lw;
}

application::is_entry_ext sfct_api::ext::private_is_symlink(path entry) noexcept
{
    application::is_entry_ext _is;
    _is.rv = fs::is_symlink(entry,_is.e);
    return _is;
}

application::is_entry_ext sfct_api::ext::private_is_regular_file(path entry) noexcept
{
    application::is_entry_ext _is;
    _is.rv = fs::is_regular_file(entry,_is.e);
    return _is;
}

application::is_entry_ext sfct_api::ext::private_exists(path entry) noexcept
{
    application::is_entry_ext _is;
    _is.rv = fs::exists(entry,_is.e);
    return _is;
}

application::is_entry_ext sfct_api::ext::private_is_directory(path entry) noexcept
{
    application::is_entry_ext _is;
    _is.rv = fs::is_directory(entry,_is.e);
    return _is;
}

std::optional<sfct_api::fs::path> sfct_api::ext::get_relative_path(path entry, path base) noexcept
{
    auto _p = private_get_relative_path(entry,base);
    if(_p.has_value()){
        if(_p.value().e){
            application::logger log(_p.value().e,application::Error::WARNING,entry);
            log.to_console();
            log.to_log_file();
            return std::nullopt;
        }
        return _p.value().p;
    }
    return std::nullopt;
}

std::optional<std::uintmax_t> sfct_api::ext::get_file_size(path entry) noexcept
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

bool sfct_api::ext::copy_file(path src, path dst, fs::copy_options co) noexcept
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

sfct_api::fs::path sfct_api::ext::combine_path_tree(path entry, path base) noexcept
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

std::optional<sfct_api::fs::path> sfct_api::ext::create_relative_path(path src, path dst,path src_base,bool create_dir) noexcept
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

    
    fs::path file_dst_dir = file_dst; 
    if(file_dst.has_filename() && !ext::is_directory(src)){
        file_dst_dir.remove_filename();
    }

    // if it fails to create the relative directory return nothing
    // if create_dir is false, the ext::create_directory_paths(file_dst).has_value() 
    // part of the expression will not be evaluated due to the short-circuiting behavior of the logical AND operator (&&) in C++.
    if(create_dir && !ext::create_directory_paths(file_dst_dir).has_value()){
        return std::nullopt;
    } 

    // if everything works out and their is no errors return the new path
    // useful for copying to a new directory tree  
    return file_dst;
}

std::optional<bool> sfct_api::ext::create_directory_paths(path dir) noexcept
{
    try{
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
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << "Runtime error :" << e.what() << "\n";
		
		return std::nullopt;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << "Allocation error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << "\n";

		return std::nullopt;
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught \n";

		return std::nullopt;
	}
}

std::optional<sfct_api::fs::path> sfct_api::ext::get_last_folder(path entry) noexcept
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

bool sfct_api::ext::remove_entry(path entry) noexcept
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

std::uintmax_t sfct_api::ext::remove_all(path dir) noexcept
{
    auto _rfe = private_remove_all(dir);
    if(_rfe.has_value()){
        if(_rfe.value().e){
            application::logger log(_rfe.value().e,application::Error::WARNING,dir);
            log.to_console();
            log.to_log_file();
        }
        return _rfe.value().files_removed;
    }
    return 0;
}

std::optional<double_t> sfct_api::ext::file_get_transfer_rate(path filepath) noexcept
{
    // begin timer
    auto start = std::chrono::high_resolution_clock::now();

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
    auto end = std::chrono::high_resolution_clock::now();
        
    std::chrono::duration<double_t> duration = end - start;
    double_t seconds = duration.count();

    // avoid division by zero but it should never happen in this case
    // its good practice to check for
    if(seconds == 0.0){
        return std::nullopt;
    }

    double_t speed = deltafilesize / seconds; // Bytes per second
    double_t rate = speed / 1024 / 1024; // MB/s

    // if the file size didnt change
    // return nothing to indicate the file is not being transfered
    if(rate == 0.0){
        return std::nullopt;
    }

    // if no errors and rate has a value not equal to zero
    // return the transfer rate in MB/s
    return rate;
}

void sfct_api::ext::copy_symlink(path src_link,path dst,fs::copy_options co) noexcept
{
    auto target = ext::read_symlink(src_link);

    if(target.has_value()){
        ext::copy_entry(target.value(),dst,co);
    }
}

bool sfct_api::ext::is_entry_available(path entry) noexcept
{
    auto fs = ext::file_status(entry);
    if(fs.has_value()){
        switch(fs.value().type()){
            case std::filesystem::file_type::none:
                // skip for now
                break;
            case std::filesystem::file_type::not_found:
                // skip for now
                break;
            case std::filesystem::file_type::regular:
                return ext::private_open_file(entry);
                break;
            case std::filesystem::file_type::directory:
                // do nothing
                break;
            case std::filesystem::file_type::symlink:{
                auto target = ext::read_symlink(entry);
                if(target.has_value()){
                    return ext::private_open_file(target.value());
                }
                else{
                    return false;
                }
                break;
            } 
            case std::filesystem::file_type::block:
                return ext::private_open_file(entry);
                break;
            case std::filesystem::file_type::character:
                return ext::private_open_file(entry);
                break;
            case std::filesystem::file_type::fifo:
                return ext::private_open_file(entry);
                break;
            case std::filesystem::file_type::socket:
                return ext::private_open_file(entry);
                break;
            case std::filesystem::file_type::unknown:
                // skip
                break;
            default:
                // do nothing
                break;
        }
    }

    return false;
}

bool sfct_api::ext::is_entry_in_transit(path entry) noexcept
{
    try{
		auto t1 = ext::last_write_time(entry);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        auto t2 = ext::last_write_time(entry);
        
        // Check if either t1 or t2 is std::nullopt
        if (!t1.has_value() || !t2.has_value()) {
            // Handle the case where one or both timestamps are not available
            // For example, return false assuming the file is not in transit if we cannot get the timestamp
            // Or handle it differently based on your application's needs
            return false;
        }
        
        if(t1.value() == t2.value()){
            return false;
        }
        else{
            return true;
        }
	}
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << "\n";

		return false;
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << "Runtime error :" << e.what() << "\n";
		
		return false;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << "Allocation error: " << e.what() << "\n";

		return false;
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << "\n";

		return false;
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught \n";

		return false;
	}
}

void sfct_api::ext::copy_entry(path src, path dst, fs::copy_options co) noexcept
{
    try{
		std::error_code e;
        fs::copy(src,dst,co,e);
        if(e){
            application::logger log(e,application::Error::WARNING,src);
            log.to_console();
            log.to_log_file();
        }
	}
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << "\n";
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << "Runtime error :" << e.what() << "\n";
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << "Allocation error: " << e.what() << "\n";
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << "\n";
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught \n";
	}
}

std::optional<std::shared_ptr<std::unordered_map<sfct_api::fs::path,sfct_api::fs::path>>> sfct_api::ext::are_directories_synced(path src, path dst,bool recursive_sync) noexcept
{
    try{
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
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << "Runtime error :" << e.what() << "\n";
		
		return std::nullopt;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << "Allocation error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << "\n";

		return std::nullopt;
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught \n";

		return std::nullopt;
	}
    
}

void sfct_api::ext::log_error_code(const std::error_code &e,path p) noexcept
{
    if(e){
        application::logger log(e,application::Error::WARNING,p);
        log.to_console();
        log.to_log_file();
    }
}

application::directory_info sfct_api::ext::get_directory_info(const application::copyto &dir) noexcept
{
    try{
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
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << "\n";

		return application::directory_info{};
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << "Runtime error :" << e.what() << "\n";
		
		return application::directory_info{};
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << "Allocation error: " << e.what() << "\n";

		return application::directory_info{};
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << "\n";

		return application::directory_info{};
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught \n";

		return application::directory_info{};
	}
}

std::optional<sfct_api::fs::path> sfct_api::ext::read_symlink(path src_link) noexcept
{
    auto _cs = private_read_symlink(src_link);
    if(_cs.has_value()){
        if(_cs.value().e){
            application::logger log(_cs.value().e,application::Error::WARNING,src_link);
            log.to_console();
            log.to_log_file();
            return std::nullopt;
        }
        return _cs.value().target;
    }
    return std::nullopt;
}

bool sfct_api::ext::entry_check(path entry) noexcept
{
    if(!ext::is_entry_available(entry)){
        while(ext::is_entry_in_transit(entry)){}
    }
    
    return ext::is_entry_available(entry);
}

void sfct_api::ext::rename_entry(path old_entry, path new_entry) noexcept
{
    std::error_code e;
    fs::rename(old_entry,new_entry,e);
    if(e){
        application::logger log(e,application::Error::WARNING,old_entry);
        log.to_console();
        log.to_log_file();
    }
}

std::optional<application::copy_sym_ext> sfct_api::ext::private_read_symlink(path src_link) noexcept
{
    try{
		application::copy_sym_ext _cs;
        _cs.target = fs::read_symlink(src_link,_cs.e);
        return _cs;
	}
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << "Runtime error :" << e.what() << "\n";
		
		return std::nullopt;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << "Allocation error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << "\n";

		return std::nullopt;
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught \n";

		return std::nullopt;
	}
}

bool sfct_api::ext::private_open_file(path filepath) noexcept
{
    try{
		std::fstream file;
        file.open(filepath, std::ifstream::in | std::ifstream::binary);
        if(file.is_open()){
            file.close();
            return true;
        }
        return false;
	}
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << "\n";

		return false;
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << "Runtime error :" << e.what() << "\n";
		
		return false;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << "Allocation error: " << e.what() << "\n";

		return false;
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << "\n";

		return false;
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught \n";

		return false;
	}
}

std::optional<application::remove_file_ext> sfct_api::ext::private_remove_all(path dir) noexcept
{
    try{
		application::remove_file_ext _rfe;
        _rfe.files_removed = fs::remove_all(dir,_rfe.e);
        return _rfe;
	}
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << "Runtime error :" << e.what() << "\n";
		
		return std::nullopt;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << "Allocation error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << "\n";

		return std::nullopt;
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught \n";

		return std::nullopt;
	}
}

application::remove_file_ext sfct_api::ext::private_remove_entry(path entry) noexcept
{
    application::remove_file_ext _rfe;
    _rfe.rv = fs::remove(entry,_rfe.e);
    return _rfe;
}

std::optional<application::path_ext> sfct_api::ext::private_get_relative_path(path entry, path base) noexcept
{
    try{
		application::path_ext _p;
        _p.p = fs::relative(entry,base,_p.e);
        return _p;
	}
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << "Runtime error :" << e.what() << "\n";
		
		return std::nullopt;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << "Allocation error: " << e.what() << "\n";

		return std::nullopt;
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << "\n";

		return std::nullopt;
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught \n";

		return std::nullopt;
	}
}

application::file_size_ext sfct_api::ext::private_get_file_size(path entry) noexcept
{
    application::file_size_ext _fse;
    _fse.size = fs::file_size(entry,_fse.e);
    return _fse;
}

application::copy_file_ext sfct_api::ext::private_copy_file(path src, path dst, fs::copy_options co) noexcept
{
    application::copy_file_ext _cfe;
    _cfe.rv = fs::copy_file(src,dst,co,_cfe.e);
    return _cfe;
}



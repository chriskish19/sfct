#pragma once
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include "logger.hpp"
#include "AppMacros.hpp"
#include "obj.hpp"
#include "TM.hpp"
#include "benchmark.hpp"
#include <unordered_set>
#include "args.hpp"
#include <functional>


// INFO:
// Functions prefixed with MT are multithreaded
// The goals for this api are: 
// 1. manage complexity
// 2. limit spaghetti
// 3. create a reliable abstraction layer


namespace sfct_api{
    // using a namespace alias for std::filesystem
    namespace fs = std::filesystem;

    // using a type alias for const std::filesystem::path&
    // path is a const std::filesystem::path reference
    using path = const std::filesystem::path&;

    /// @brief extension functions for sfct_api that houses private functions not callable outside the api
    /// this class is useful because some functions in the api depend on functions themselves but I dont want those functions to be in 
    /// the api itself, those functions will be private in the ext class. ext functions designed to be used only within the sfct_api.
    /// ext functions are not thoroughly checked, which is why its not recommended to use ext functions outside sfct_api unless checks are performed already.
    /// the purpose of ext is to improve speed by reducing file path checks. Before ext sfct_api would use functions within the api itself that have file
    /// path checks so in a function that uses other functions their might be double or triple checks for a file path which is unnecessary overhead.
    class ext{
        public:
            /// @brief wrapper for private_get_relative_path, if private_get_relative_path fails the error is logged and nothing is returned.
            /// @param entry any path
            /// @param base any path
            /// @return relative path
            static std::optional<fs::path> get_relative_path(path entry,path base);

            /// @brief wrapper for private_get_file_size, if private_get_file_size fails the error is logged and nothing is returned.
            /// @param entry any path
            /// @return the file size if there is no error, and nothing if there is an error.
            static std::optional<std::uintmax_t> get_file_size(path entry);

            /// @brief wrapper for private_copy_file(src,dst,co,error_code). If it fails the error is logged and false is returned.
            /// @param src any path
            /// @param dst any path 
            /// @param co any copy options 
            /// @return true if no error, false for error
            static bool copy_file(path src,path dst,fs::copy_options co);

            /// @brief removes root path of entry and combines it with base
            /// @param entry any path
            /// @param base any path
            /// @return 
            /// Example:
            /// entry: C:/test
            /// base: D:/high
            /// returned: D:/high/test
            static fs::path combine_path_tree(path entry,path base);

            /// @brief gets the relative path and then creates the new directory. If src and dst are on different root drives, use src_base as the base path 
            /// and src as the source file path. If src_base is not specified and src and dst are on different drives the function will return the dst path with the last folder of src.
            /// This function is meant to be called within sfct_api only since it accepts any path for all three parameters and there is no checks. 
            /// The function will not work as intended without proper path checking. 
            /// @param src any path 
            /// @param dst any path
            /// @param src_base any path (optional)
            /// @param create_dir creates the new relative directory. (optional)
            /// @return the new relative path.
            /// For example: 
            /// src = C:/test/a 
            /// dst = D:/home
            /// src_base = C:/test
            /// returned: D:/home/a  
            static std::optional<fs::path> create_relative_path(path src,path dst,path src_base=fs::path(),bool create_dir=true);

            /// @brief given a path the function attempts to create the directories in the path.
            /// @param dir any path, does no check wether dir is a directory
            /// @return false for not creating the directory, nothing for an error, and true for creating the directory.
            static std::optional<bool> create_directory_paths(path dir);

            /// @brief gets the last folder name in entry and returns it as a path. 
            /// @param entry any path
            /// @return the last folder in the path entry, if entry has no parent path nothing is returned.
            /// Example:
            /// entry = C:/test/parent/myfile.txt
            /// returned: parent
            static std::optional<fs::path> get_last_folder(path entry);

            /// @brief wrapper for private_remove_entry(). If there is an error it is logged and false is returned.
            /// @param entry any path
            /// @return true for successful removal and false for no removal.
            static bool remove_entry(path entry);

            /// @brief wrapper for private_remove_all(). If there is an error it is logged.
            /// @param dir any path
            /// @return the number of removed files
            static std::uintmax_t remove_all(path dir);

            /// @brief Gets the transfer speed of a file if its currently being copied to filepath
            /// it does a quick check difference in file size over a 10ms interval and then calculates the speed in MB/s
            /// @param filepath any path
            /// @return the rate in MB/s
            /// returns nothing if errors occurred
            /// returns nothing if there was no change in file size and rate is 0.0
            static std::optional<double_t> file_get_transfer_rate(path filepath);

            /// @brief copies the actual file that src_link points to, to dst. Uses ext::copy_file().
            /// @param src_link any path
            /// @param dst any path
            /// @param co any copy options
            /// @return ext::copy_file().
            static bool copy_symlink(path src_link,path dst,fs::copy_options co);

            /// @brief checks a file if it is currently being used. Can be any type of file.
            /// @param entry any path
            /// @return if the entry is in use it returns false, if it is not in use it returns true.
            static bool is_entry_available(path entry);

            /// @brief checks the last write times in a 250ms interval. Then compares the times to see if they are equal, if they are it returns false else true.
            /// @param entry any path
            /// @return true if the entry is being transferred into entry path, false if it isnt.
            static bool is_entry_in_transit(path entry);

            /// @brief wrapper for std::filesystem::copy(). 
            /// @param src any path
            /// @param dst any path
            /// @param co any copy_options
            /// @attention there was an error it is logged.
            static void copy_entry(path src,path dst,fs::copy_options co);

            /// @brief checks if dst is missing files found in src.
            /// @param src any path
            /// @param dst any path
            /// @param recursive_sync (optional) checks subtree
            /// @return destination paths and source paths in an unordered_map.
            /// the missing file paths not found in dst but exist in src. if no missing files are found nothing is returned.
            /// an unordered_map key is dst and value is src
            static std::optional<std::shared_ptr<std::unordered_map<fs::path,fs::path>>> are_directories_synced(path src,path dst,bool recursive_sync=true);
        private:
            /// @brief opens a file at filepath
            /// @param filepath any path
            /// @return true if the file was opened, false if it failed to open.
            static bool private_open_file(path filepath);

            /// @brief wrapper for std::filesystem::remove_all().
            /// @param dir any path
            /// @return a remove_file_ext object which contains the error code, return value boolean, and the number of files removed.
            static application::remove_file_ext private_remove_all(path dir);
            
            /// @brief wrapper for std::filesystem::remove().
            /// @param entry any path
            /// @return a remove_file_ext object which contains the error code and return value boolean.
            static application::remove_file_ext private_remove_entry(path entry);

            /// @brief wrapper for std::filesystem::relative(entry,base)
            /// @param entry: any path 
            /// @param base: any path
            /// @return a path_ext object which contains the relative path and error code.
            static application::path_ext private_get_relative_path(path entry,path base);

            /// @brief wrapper for std::filesystem::file_size(path)
            /// @param entry: any path 
            /// @return a file_size_ext object which contains the size and error code.
            static application::file_size_ext private_get_file_size(path entry);

            /// @brief wrapper for std::filesystem::copy_file(src,dst,co,error_code)
            /// @param src: any path
            /// @param dst: any path
            /// @param co: any copy options
            /// @return a copy_file_ext object which contains the error code and returned value from the function std::filesystem::copy_file()
            static application::copy_file_ext private_copy_file(path src,path dst,fs::copy_options co);
    };


    /// @brief wrapper for ext::is_entry_avaliable(). First checks if the entry exists. 
    /// @param entry any path
    /// @return true if the entry is available, false if in use.
    bool is_entry_available(path entry);

    /// @brief waits in 250ms intervals until an entry has been completly copied into entry path. Then checks if the entry is available.
    /// Checks if entry exists on the system if it doesnt it returns false.
    /// @param entry any 
    /// @return true if the entry is available and false if it isnt. False if entry doesnt exist in the system.
    bool entry_check(path entry); 

    /// @brief Checks if dir is a valid directory that exists on the system
    /// @param dir directory path
    /// @return false for not a directory and true if it is
    bool check_directory(path dir);

    /// @brief if the source path doesnt exist the directory is created 
    /// @param src source path: can be a file entry or directory
    /// @return if the source path already exists the function returns false.
    /// false if there was an error, which will be logged.
    /// true if source path gets created.
    bool create_directory_paths(path src);

    /// @brief wrapper for ext::get_relative_path(). Checks for file path to be a regular file on the system. Checks for base path to be a directory on the system.
    /// @param file: must be a valid file on the system
    /// @param base: must be a valid directory on the system
    /// @return: a new relative path
    /// if there was an error it returns nothing but logs the error.
    std::optional<fs::path> get_relative_file_path(path file,path base);

    /// @brief wrapper for ext::get_relative_path(). Checks for entry path to be on the system. Checks for base path to be on the system.
    /// @param entry: must be a valid path on the system 
    /// @param base: must be a valid directory on the system
    /// @return a new relative path
    /// if there was an error it returns nothing but logs the error.
    std::optional<fs::path> get_relative_path(path entry,path base);

    /// @brief wrapper for create_relative_path(). Checks if src exists and if it doesnt returns nothing. 
    /// Checks if src_base is on the system if its specified and if its not on the system returns nothing. 
    /// Use this function when you have a src file path and want the src tree directory created at a dst directory path.
    /// for example src = "C:/test/myFolder/myfile.txt" and dst = "D:/test". The returned path = "D:/test/myFolder".
    /// if dst does not exist it is created using create_directory_paths(path src). 
    /// @param src source file path or directory
    /// @param dst destination file path or directory
    /// @param src_base (optional). a base path of src. for example you iterate recursively through a directory 
    /// src_base would be the iterator starting directory. And any paths inside would be src.
    /// @return destination file path
    std::optional<fs::path> create_file_relative_path(path src,path dst,path src_base=fs::path(),bool create_dir=true);

    /// @brief Given a src file path and a dst path, the src file is copied into dst path.
    /// if the destination path does not exists it is created.
    /// @param src source file path
    /// @param dst destination path
    /// @param co copy options:
    /// none: No special copy options are specified. If the target file already exists, the operation will fail.
    /// skip_existing: Skip the copy operation if the target file already exists. This prevents overwriting existing files.
    /// overwrite_existing: Overwrite the target file if it already exists. This will replace the contents of the target file with the source file.
    /// update_existing: Only overwrite the target file if it already exists and the source file is newer than the target file. This is useful for updating files with a newer version while leaving unchanged files as is.
    /// @return boolean: if src is not a file path or a regular file the function returns false.
    /// if the copy operation fails the function returns false.
    /// if src and dst pass the checks and the copy operation succeeds the function returns true.
    bool copy_file_create_relative_path(path src,path dst,fs::copy_options co);

    /// @brief Create the src directory tree at dst (directories only)
    /// @param src src directory, must be a valid directory existing on the system.
    /// @param dst dst directory, must be a valid directory existing on the system.
    /// @return boolean: if src and dst are valid directories the function will return true else it returns false.
    /// directories may or may not be created, it will appear in the log file or console if a directory fails to be created.
    bool create_directory_tree(path src,path dst);

    /// @brief wrapper for ext::file_get_transfer_rate().
    /// @param src file path: must be a regular file existing on the system
    /// @return calls ext::file_get_transfer_rate()
    std::optional<double_t> file_get_transfer_rate(path src);

    /// @brief wrapper for ext::copy_file(). Checks if the dst directory path is valid on the system. Checks if the src path is a valid file on the system.
    /// @param src source file path
    /// @param dst destination path, could be a directory or include a file name in the path.
    /// @param co any copy options
    /// @return if src or dst is not valid on the system false is returned.
    /// when it calls ext::copy_file() it returns: true if no error, false if error.
    bool copy_file(path src,path dst,fs::copy_options co);

    /// @brief copies a file to a dst path that is created. 
    /// @param src source file: must be a regular file on the system
    /// @param dst destination path can be any path
    /// @param co any copy options
    /// @return if it fails to create the directory, false is returned.
    /// if src is not a regular file on the system false is returned.
    /// ext::copy_file(): true is returned for no error and false for error.
    bool copy_file_create_path(path src,path dst,fs::copy_options co);

    /// @brief wrapper for ext::remove_all(). given a directory, all the files in the directory are deleted recursively
    /// @param dir must be a valid directory on the system
    /// @return the number of removed files
    std::uintmax_t remove_all(path dir);
    
    /// @brief wrapper for ext::remove_entry(). Removes a file or entry given a path.
    /// @param entry path must exist on the system
    /// @return ext::remove_entry(): true for removal and false for no removal. False if path does not exist on the system.
    bool remove_entry(path entry);

    /// @brief wrapper for ext::copy_symlink(). Copies the target file that sym_link points to, to dst.
    /// checks that src_link is valid. Checks that dst has a valid directory path in it.
    /// @param src_link must be a valid sym_link on the system
    /// @param dst must be a valid directory or file path
    /// @param co any copy options
    /// @return true for no error and false if there is an error. Calls private_copy_file() under the hood which will log the error if there is one.
    bool copy_symlink(path src_link,path dst,fs::copy_options co);

    /// @brief converts cs commands to fs::copy_options
    /// @param commands 
    /// @return corresponding fs::copy_options
    fs::copy_options get_copy_options(application::cs commands) noexcept;

    /// @brief checks if recursive flag is set
    /// @param commands 
    /// @return true if it is and false if it is not set.
    bool recursive_flag_check(application::cs commands) noexcept;

    /// @brief wrapper for ext::copy_entry(). Copies any type of file or a whole directory.
    /// @param src must exist
    /// @param dst if it does not exist it will be created
    /// @param co any copy options
    void copy_entry(path src,path dst,fs::copy_options co);

    /// @brief wrapper for ext::are_directories_synced(). very slow function.
    /// @param src must be a directory on the system
    /// @param dst must be a directory on the system
    /// @param recursive_sync (optional) checks subtree.
    /// @return destination paths and source paths in an unordered_map.
    /// the missing file paths not found in dst but exist in src. if no missing files are found nothing is returned.
    /// an unordered_map key is dst and value is src
    std::optional<std::shared_ptr<std::unordered_map<fs::path,fs::path>>> are_directories_synced(path src,path dst,bool recursive_sync=true);
}
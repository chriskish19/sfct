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
            /// @param entry: any path
            /// @param base: any path
            /// @return relative path
            static std::optional<fs::path> get_relative_path(path entry,path base);

            /// @brief wrapper for private_get_file_size, if private_get_file_size fails the error is logged and nothing is returned.
            /// @param entry: any path
            /// @return: the file size if there is no error, and nothing if there is an error.
            static std::optional<std::uintmax_t> get_file_size(path entry);

            /// @brief wrapper for private_copy_file(src,dst,co,error_code).
            /// @param src: any path
            /// @param dst: any path 
            /// @param co: any copy options 
            /// @return true if no error, false for error
            static bool copy_file(path src,path dst,fs::copy_options co);

            /// @brief removes root path of entry and combines it with base
            /// @param entry 
            /// @param base 
            /// @return 
            /// Example:
            /// entry: C:/test
            /// base: D:/high
            /// returned: D:/high/test
            static fs::path combine_path_tree(path entry,path base);

            /// @brief 
            /// @param src 
            /// @param dst
            /// @param src_base 
            /// @return 
            static std::optional<fs::path> create_file_relative_path(path src,path dst,path src_base=fs::path());

            /// @brief 
            /// @param dir 
            /// @return 
            static std::optional<bool> create_directory_paths(path dir);

            /// @brief 
            /// @param entry 
            /// @return 
            static fs::path get_last_folder(path entry);

            /// @brief 
            /// @param file 
            /// @return 
            static bool remove_file(path file);

            /// @brief 
            /// @param dir 
            /// @return 
            static std::uintmax_t remove_all(path dir);

            /// @brief 
            /// @param filepath 
            /// @return 
            static std::optional<double_t> file_get_transfer_rate(path filepath);

            /// @brief 
            /// @param src 
            /// @return 
            static bool copy_symlink(path src_link,path dst,fs::copy_options co);

            /// @brief 
            /// @param filepath 
            /// @return 
            static bool is_file_available(path filepath);

            /// @brief 
            /// @param filepath 
            /// @return 
            static bool is_file_in_transit(path filepath);
        private:
            /// @brief 
            /// @param dir 
            /// @return 
            static application::remove_file_ext private_remove_all(path dir);
            
            /// @brief 
            /// @param file 
            /// @return 
            static application::remove_file_ext private_remove_file(path file);

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


    /// @brief checks a file if it is currently being used it returns false, if it is not in use it returns true
    /// @param filepath 
    /// @return boolean
    bool is_file_available(path filepath);

    /// @brief Waits until a file is available, pool checks the file every second. Will pool check indefinitely if the file never becomes available.
    /// @param filepath 
    bool file_check(path filepath);

    /// @brief Checks if dir is a valid directory that exists on the system
    /// @param dir directory path
    /// @return false for not a directory and true if it is
    bool check_directory(path dir);

    /// @brief if the source path doesnt exist the directory is created 
    /// @param src source path: can be a file entry or directory
    /// @return 
    bool create_directory_paths(path src);

    /// @brief 
    /// @param file: must be a valid file on the system
    /// @param base: must be a valid directory on the system
    /// @return: a new relative path
    std::optional<fs::path> get_relative_file_path(path file,path base);

    /// @brief 
    /// @param entry: must be a valid path on the system 
    /// @param base: must be a valid directory on the system
    /// @return a new relative path
    std::optional<fs::path> get_relative_path(path entry,path base);

    /// @brief Use this function when you have a src file path and want the src tree directory created at a dst directory path.
    /// for example src = "C:/test/myFolder/myfile.txt" and dst = "D:/test". The returned path = "D:/test/myFolder".
    /// if dst does not exist it is created using create_directory_paths(path src). 
    /// @param src source file path or directory
    /// @param dst destination directory
    /// @return destination file path
    std::optional<fs::path> create_file_relative_path(path src,path dst,path src_base=fs::path());

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

    /// @brief Gets the transfer speed of a file if its currently being copied to src path
    /// it does a quick check difference in file size over a 10ms interval and then calculates the speed in MB/s
    /// @param src file path: must be a regular file existing on the system
    /// @return rate of transfer in MB/s
    /// returns nothing if errors occurred
    /// returns nothing if there was no change in file size and rate is 0.0
    std::optional<double_t> file_get_transfer_rate(path src);

    /// @brief 
    /// @param src 
    /// @param dst 
    /// @param co 
    /// @return 
    bool copy_file(path src,path dst,fs::copy_options co);

    /// @brief 
    /// @param src 
    /// @param dst 
    /// @param co 
    /// @return 
    bool copy_file_create_path(path src,path dst,fs::copy_options co);

    /// @brief 
    /// @param dir 
    /// @return 
    std::uintmax_t remove_all(path dir);
    
    /// @brief 
    /// @param file 
    /// @return 
    bool remove_file(path file);

    /// @brief 
    /// @param src_link 
    /// @param dst 
    /// @param co 
    /// @return 
    bool copy_symlink(path src_link,path dst,fs::copy_options co);
}
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

    /// @brief checks a file if it is currently being used it returns false, if it is not in use it returns true
    /// @param filepath 
    /// @return boolean
    bool IsFileAvailable(path filepath);

    /// @brief Waits until a file is available, pool checks the file every second. Will pool check indefinitely if the file never becomes available.
    /// @param filepath 
    void FileCheck(path filepath);

    /// @brief Checks if dir is a valid directory that exists on the system
    /// @param dir directory path
    /// @return false for not a directory and true if it is
    bool CheckDirectory(path dir);

    /// @brief if the source path doesnt exist the directory is created 
    /// @param src source path: can be a file entry or directory
    /// @return boolean. True for sucessful creation. If the directory already exists it returns true. Returns false if the directory fails to be created.
    bool CDirectory(path src);

    /// @brief Removes the file root path and then combines it with base to form a new path
    /// @param file: must be a valid file on the system
    /// @param base: must be a valid directory on the system
    /// @return: a new relative path
    /// Example:
    /// file: C:/test/file.txt
    /// base: D:/high
    /// returned: D:/high/test/file.txt
    std::optional<fs::path> GetRelativeFilePath(path file,path base);

    /// @brief Removes the entry root path and then combines it with base to form a new path
    /// @param entry: must be a valid path on the system 
    /// @param base: must be a valid directory on the system
    /// @return a new relative path
    /// Example:
    /// file: C:/test
    /// base: D:/high
    /// returned: D:/high/test
    std::optional<fs::path> GetRelativePath(path entry,path base);

    /// @brief Use this function when you have a src file path and want the src tree directory created at a dst directory path.
    /// for example src = "C:/test/myFolder/myfile.txt" and dst = "D:/test". The returned path = "D:/test/myFolder".
    /// if dst does not exist it is created using CDirectory(path src). 
    /// @param src source file path or directory
    /// @param dst destination directory
    /// @return destination file path
    std::optional<fs::path> CreateFileRelativePath(path src,path dst);

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
    bool CopyFileCreatePath(path src,path dst,fs::copy_options co);

    /// @brief Create the src directory tree at dst (directories only)
    /// @param src src directory, must be a valid directory existing on the system.
    /// @param dst dst directory, must be a valid directory existing on the system.
    /// @return boolean: if src and dst are valid directories the function will return true else it returns false.
    /// directories may or may not be created, it will appear in the log file or console if a directory fails to be created.
    bool CreateDirectoryTree(path src,path dst);

    /// @brief Gets the transfer speed of a file if its currently being copied to src path
    /// it does a quick check difference in file size over a 10ms interval and then calculates the speed in MB/s
    /// @param src file path: must be a regular file existing on the system
    /// @return rate of transfer in MB/s
    /// returns nothing if errors occurred
    /// returns nothing if there was no change in file size and rate is 0.0
    std::optional<double_t> FileGetTransferRate(path src);

    /// @brief 
    /// @param d1 
    /// @param d2 
    /// @return 
    std::optional<std::shared_ptr<std::unordered_set<fs::path>>> GetDifferenceBetweenDirectoriesSingle(path d1,path d2);
}
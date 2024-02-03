#pragma once
#include <memory>
#include <vector>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include "obj.hpp"
#include "ConsoleTM.hpp"
#include "logger.hpp"
#include "AppMacros.hpp"
#include "benchmark.hpp"
#include <unordered_set>
#include <string>
#include <optional>


//////////////////////////////////////////////////////////////////////////
// This header provides helper functions needed throughout the program
//////////////////////////////////////////////////////////////////////////

namespace application{
    // checks if a file is available
    // returns true if it is and false if not
    inline bool FileReady(const std::filesystem::path& src){
        if(std::filesystem::is_regular_file(src)){
            std::fstream file;
            file.open(src, std::ifstream::in | std::ifstream::binary);
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

    // checks all files in a single directory(non recursive) if they are available
    // sends info to the message stream
    inline std::uintmax_t single_check(const std::filesystem::path& src){
        std::uintmax_t totalsize{};

        if(!std::filesystem::exists(src)){
            logger log(App_MESSAGE("invalid path"),Error::DEBUG,src);
            log.to_console();
            log.to_log_file();
            return totalsize;
        }
        
        m_MessageStream.SetMessage(App_MESSAGE("Checking files"));
        m_MessageStream.ReleaseBuffer();

        std::filesystem::directory_iterator dit(src);
        std::filesystem::directory_iterator dit_end;

        while(dit != dit_end){
            totalsize += dit->file_size();

            // loop through the directory
            m_MessageStream.SetMessage(App_MESSAGE("Waiting for file to become available: ") + STRING(dit->path().filename()));
            while(!FileReady(dit->path())){
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            m_MessageStream.SetMessage(App_MESSAGE("File is available: ") + STRING(dit->path().filename()));

            dit++;
        }

        m_MessageStream.ReleaseBuffer();

        return totalsize;
    }

    // checks all files in a directory tree recursively if they are available
    // sends info to the message stream
    // returns the total size of the directory in bytes
    inline std::uintmax_t recursive_check(const std::filesystem::path& src){
        std::uintmax_t totalsize{};
        if(!std::filesystem::exists(src)){
            logger log(App_MESSAGE("invalid path"),Error::DEBUG,src);
            log.to_console();
            log.to_log_file();
            return totalsize;
        }
        
        m_MessageStream.SetMessage(App_MESSAGE("Checking files"));
        m_MessageStream.ReleaseBuffer();
        
        // checks that each file is avaliable and can be copied before attempting to copy
        std::filesystem::recursive_directory_iterator rdit(src);
        std::filesystem::recursive_directory_iterator rdit_end;
        while(rdit != rdit_end){
           totalsize += rdit->file_size();
           
            // loop through the directory
            m_MessageStream.SetMessage(App_MESSAGE("Waiting for file to become available: ") + STRING(rdit->path().filename()));
            while(!FileReady(rdit->path())){
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            m_MessageStream.SetMessage(App_MESSAGE("File is available: ") + STRING(rdit->path().filename()));

            rdit++;
        }

        m_MessageStream.ReleaseBuffer();

        return totalsize;
    }

    // convert commands to copy options
    inline std::filesystem::copy_options GetCopyOptions(cs commands) noexcept{
        using fs_co = std::filesystem::copy_options;
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

    inline void FullCopy(const std::vector<copyto>& dirs){
        for(const auto& dir:dirs){
            std::uintmax_t totalsize{};
            if((dir.commands & cs::recursive) != cs::none){
                totalsize = recursive_check(dir.source);
            }
            else if((dir.commands & cs::single) != cs::none){
                totalsize = single_check(dir.source);
            }
            
            benchmark speed;
            speed.start_clock();
            std::filesystem::copy(dir.source,dir.destination,dir.co);
            speed.end_clock();
            double_t rate = speed.speed(totalsize);

            m_MessageStream.SetMessage(App_MESSAGE("Speed in MB/s: ") + TOSTRING(rate));
            m_MessageStream.ReleaseBuffer();
        }
    }

    // this can be run asynchronously to get a directory size in bytes
    inline std::uintmax_t GetDirSize(const copyto& dir){
        std::uintmax_t totalsize;
        if((dir.commands & cs::recursive) != cs::none){
            for(const auto& entry:std::filesystem::recursive_directory_iterator(dir.source)){
                totalsize += entry.file_size();
            }
        }
        else if((dir.commands & cs::single) != cs::none){
            for(const auto& entry:std::filesystem::directory_iterator(dir.source)){
                totalsize += entry.file_size();
            }
        }
        return totalsize;
    }

    inline double_t GetAverageFileSize(const copyto& dir){
        std::uintmax_t totalsize{},fileCount{};
        if((dir.commands & cs::recursive) != cs::none){
            for(const auto& entry:std::filesystem::recursive_directory_iterator(dir.source)){
                totalsize += entry.file_size();
                fileCount++;
            }
        }
        else if((dir.commands & cs::single) != cs::none){
            for(const auto& entry:std::filesystem::directory_iterator(dir.source)){
                totalsize += entry.file_size();
                fileCount++;
            }
        }
        double_t avg_size = static_cast<double_t>(totalsize / fileCount);
        // average size in bytes
        return avg_size;
    }

    // returns a directory_info object that has files count, total size and average size
    inline directory_info GetDI(const copyto& dir){
        directory_info counts{};
        if((dir.commands & cs::recursive) != cs::none){
            for(const auto& entry:std::filesystem::recursive_directory_iterator(dir.source)){
                counts.TotalSize += entry.file_size();
                counts.FileCount++;
            }
        }
        else if((dir.commands & cs::single) != cs::none){
            for(const auto& entry:std::filesystem::directory_iterator(dir.source)){
                counts.TotalSize += entry.file_size();
                counts.FileCount++;
            }
        }
        counts.AvgFileSize = static_cast<double_t>(counts.TotalSize / counts.FileCount);
        return counts;
    }

    // checks if the directory exists and if it doesnt it attempts to create the directory
    inline void CDirectory(const std::filesystem::path& dir) {
        std::filesystem::path directoryPath = dir;

        // If the path has a filename, use the parent directory
        if (dir.has_filename()) {
            directoryPath = dir.parent_path();
        }

        if (!std::filesystem::is_directory(directoryPath) && !std::filesystem::create_directories(directoryPath)) {
            logger log(App_MESSAGE("Failed to create directories"), Error::WARNING, directoryPath);
            log.to_console();
            log.to_log_file();
            log.to_output();
        }
    }

    inline bool CheckDirectory(const std::filesystem::path& dir){
        if(!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)){
            logger log(App_MESSAGE("Not a valid directory"),Error::WARNING,dir);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return false;
        }
        return true;
    }


    // copy a directory
    // when finished the speed is displayed to the message stream
    inline bool CopyDir(const copyto& dir){
        // check that it is a valid directory
        if(!CheckDirectory(dir.source) || !CheckDirectory(dir.destination)){
            return false;
        }
        
        std::uintmax_t totalsize{};
        if((dir.commands & cs::recursive) != cs::none){
            totalsize = recursive_check(dir.source);
        }
        else if((dir.commands & cs::single) != cs::none){
            totalsize = single_check(dir.source);
        }
        
        benchmark speed;
        speed.start_clock();
        std::filesystem::copy(dir.source,dir.destination,dir.co);
        speed.end_clock();
        double_t rate = speed.speed(totalsize);

        m_MessageStream.SetMessage(App_MESSAGE("Speed in MB/s: ") + TOSTRING(rate));
        m_MessageStream.ReleaseBuffer();

        return true;
    }

    // this function is expensive but useful, it returns a set of directories that are different between two directories non recursive.
    // if comparing d1 C:/test and d2 D:/test it will return the unique entrys directories or files ect.
    // say for example C:/test/truth.mp3 and D:/test/lie.mp3 both C:/test/truth.mp3 and D:/test/lie.mp3 paths will be in the returned set
    // another example C:/test/truth.mp3 and D:/test/truth.mp3 neither will be in the returned set because both entries exist in the d1 and d2.
    inline std::optional<std::shared_ptr<std::unordered_set<std::filesystem::path>>> DirectoryDifferenceSingle(const std::filesystem::path& d1, const std::filesystem::path& d2){
        if(!CheckDirectory(d1) || !CheckDirectory(d2)){
            return std::nullopt;
        }

        std::unordered_set<std::filesystem::path> d1paths,d2paths,unique_paths;

        for(const auto& entry:std::filesystem::directory_iterator(d1)){
            auto relativePath = std::filesystem::relative(entry.path(),d1);
            d1paths.emplace(relativePath);
        }

        for(const auto& entry:std::filesystem::directory_iterator(d2)){
            auto relativePath = std::filesystem::relative(entry.path(),d2);
            if(d1paths.find(relativePath) == d1paths.end()){
                unique_paths.emplace(entry.path());
                d2paths.emplace(relativePath);
            }
        }

        for(const auto& entry:std::filesystem::directory_iterator(d1)){
            auto relativePath = std::filesystem::relative(entry.path(),d1);
            if(d2paths.find(relativePath)==d2paths.end()){
                unique_paths.emplace(entry.path());
            }
        }


        if(unique_paths.empty()){
            return std::nullopt;
        }

        return std::make_shared<std::unordered_set<std::filesystem::path>>(unique_paths);
    } 

    // this function is really expensive depending on the directory size ,its the recursive version
    // see above DirectoryDifferenceSingle() for explaination
    inline std::optional<std::shared_ptr<std::unordered_set<std::filesystem::path>>> DirectoryDifferenceRecursive(const std::filesystem::path& d1, const std::filesystem::path& d2){
        if(!CheckDirectory(d1) || !CheckDirectory(d2)){
            return std::nullopt;
        }

        std::unordered_set<std::filesystem::path> d1paths,d2paths,unique_paths;

        for(const auto& entry:std::filesystem::recursive_directory_iterator(d1)){
            auto relativePath = std::filesystem::relative(entry.path(),d1);
            d1paths.emplace(relativePath);
        }

        for(const auto& entry:std::filesystem::recursive_directory_iterator(d2)){
            auto relativePath = std::filesystem::relative(entry.path(),d2);
            if(d1paths.find(relativePath) == d1paths.end()){
                unique_paths.emplace(entry.path());
                d2paths.emplace(relativePath);
            }
        }

        for(const auto& entry:std::filesystem::recursive_directory_iterator(d1)){
            auto relativePath = std::filesystem::relative(entry.path(),d1);
            if(d2paths.find(relativePath)==d2paths.end()){
                unique_paths.emplace(entry.path());
            }
        }


        if(unique_paths.empty()){
            return std::nullopt;
        }

        return std::make_shared<std::unordered_set<std::filesystem::path>>(unique_paths);
    } 

    // given two paths find if sub_of_dir is a subtree of dir
    // doesnt check if the paths are valid on the system
    // its a string checking function
    inline bool FindDirectoryPaths(const std::filesystem::path& dir,const std::filesystem::path& sub_of_dir){
        STRING s_dir(dir);
        STRING s_sub_of_dir(sub_of_dir);

        size_t found_pos = s_sub_of_dir.find(dir);
        return found_pos != std::string::npos;
    }
}
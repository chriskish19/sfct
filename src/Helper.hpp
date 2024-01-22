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
    inline void single_check(const std::filesystem::path& src){
        if(!std::filesystem::exists(src)){
            logger log(App_MESSAGE("invalid path"),Error::DEBUG,src);
            log.to_console();
            log.to_log_file();
            return;
        }
        
        m_MessageStream.SetMessage(App_MESSAGE("Checking files"));
        m_MessageStream.ReleaseBuffer();

        std::filesystem::directory_iterator dit(src);
        std::filesystem::directory_iterator dit_end;

        while(dit != dit_end){
            // loop through the directory
            m_MessageStream.SetMessage(App_MESSAGE("Waiting for file to become available: ") + STRING(dit->path().filename()));
            while(!FileReady(dit->path())){
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            m_MessageStream.SetMessage(App_MESSAGE("File is available: ") + STRING(dit->path().filename()));

            dit++;
        }

        m_MessageStream.ReleaseBuffer();
    }

    // checks all files in a directory tree recursively if they are available
    // sends info to the message stream
    inline void recursive_check(const std::filesystem::path& src){
        if(!std::filesystem::exists(src)){
            logger log(App_MESSAGE("invalid path"),Error::DEBUG,src);
            log.to_console();
            log.to_log_file();
            return;
        }
        
        m_MessageStream.SetMessage(App_MESSAGE("Checking files"));
        m_MessageStream.ReleaseBuffer();
        
        // checks that each file is avaliable and can be copied before attempting to copy
        std::filesystem::recursive_directory_iterator rdit(src);
        std::filesystem::recursive_directory_iterator rdit_end;
        while(rdit != rdit_end){
            // loop through the directory
            m_MessageStream.SetMessage(App_MESSAGE("Waiting for file to become available: ") + STRING(rdit->path().filename()));
            while(!FileReady(rdit->path())){
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            m_MessageStream.SetMessage(App_MESSAGE("File is available: ") + STRING(rdit->path().filename()));

            rdit++;
        }

        m_MessageStream.ReleaseBuffer();
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
            if((dir.commands & cs::recursive) != cs::none){
                recursive_check(dir.source);
            }
            else if((dir.commands & cs::single) != cs::none){
                single_check(dir.source);
            }
            std::filesystem::copy(dir.source,dir.destination,dir.co);
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

    inline const std::filesystem::path exe_path = std::filesystem::current_path();
}
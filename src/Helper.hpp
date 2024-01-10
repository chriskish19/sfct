#pragma once
#include <memory>
#include <vector>
#include "FileParse.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>



namespace application{
    inline CONSOLETM m_MessageStream;

    inline bool FileReady(std::filesystem::path src){
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
    
    
    inline void FullCopy(std::shared_ptr<std::vector<copyto>> data){
        m_MessageStream.SetMessage(App_MESSAGE("Checking files before copying"));
        
        // checks that each file is avaliable and can be copied before attempting to copy
        for(const auto& dir:*data){
            std::filesystem::recursive_directory_iterator rdit(dir.fs_source);
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

        }

        
        for(const auto& dir:*data){
            m_MessageStream.SetMessage(App_MESSAGE("Copying Directory: ") + STRING(dir.fs_source));
            std::filesystem::copy(dir.fs_source,dir.fs_destination,std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing);
        }
    }
}
#pragma once
#include <memory>
#include <vector>
#include "FileParse.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>



namespace application{
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
        // checks that each file is avaliable and can be copied before attempting to copy
        for(const auto& dir:*data){
            std::filesystem::recursive_directory_iterator rdit(dir.fs_source);
            std::filesystem::recursive_directory_iterator rdit_end;
            while(rdit != rdit_end){
                // loop through the directory
                
                while(!FileReady(rdit->path())){
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                rdit++;
            }

        }

        for(const auto& dir:*data){
            std::filesystem::copy(dir.fs_source,dir.fs_destination,std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing);
        }
    }
}
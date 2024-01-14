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
    
    
    inline void FullCopy(const copyto& dir){
        m_MessageStream.SetMessage(App_MESSAGE("Checking files before copying"));
        
        if(dir.commands.find(cs::copy)==dir.commands.end() && dir.commands.find(cs::fast_copy)==dir.commands.end()){
            logger log(App_MESSAGE("No copy commands found"),Error::DEBUG);
            log.to_console();
            log.to_log_file();
            return;
        }



        if(dir.commands.find(cs::single)!=dir.commands.end()){
            std::filesystem::directory_iterator dit(dir.source);
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
        else if(dir.commands.find(cs::recursive)!=dir.commands.end()){
            // checks that each file is avaliable and can be copied before attempting to copy
            std::filesystem::recursive_directory_iterator rdit(dir.source);
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

            if(dir.commands.find(cs::update)!=dir.commands.end()){
                m_MessageStream.SetMessage(App_MESSAGE("Copying Directory: ") + STRING(dir.source));
                std::filesystem::copy(dir.source,dir.destination,std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing);
            }
            else if(dir.commands.find(cs::overwrite)!=dir.commands.end()){
                m_MessageStream.SetMessage(App_MESSAGE("Copying Directory: ") + STRING(dir.source));
                std::filesystem::copy(dir.source,dir.destination,std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
            }

            m_MessageStream.ReleaseBuffer();
        }




        
        
        
    }
}
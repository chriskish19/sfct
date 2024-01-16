#pragma once
#include <filesystem>
#include "args.hpp"

/////////////////////////////////////////////////////////////////
// This header contains common structures that are used throughout the program.
/////////////////////////////////////////////////////////////////


namespace application{
     struct copyto{
        // source directory path to monitor or copy from
        std::filesystem::path source;

        // destination directory path to copy or sync to
        std::filesystem::path destination; 

        // holds the arguments whether to copy or monitor, sync, update ect.         
        cs commands = cs::none; 

        // holds the arguments in std::filesystem::copy_options format                    
        std::filesystem::copy_options co = std::filesystem::copy_options::none;           
    };
}
#pragma once
#include <filesystem>
#include "args.hpp"

/////////////////////////////////////////////////////////////////
// This header contains common structures that are used throughout the program.
/////////////////////////////////////////////////////////////////


namespace application{
     struct copyto{
        std::filesystem::path source;               // source directory path to monitor or copy from
        std::filesystem::path destination;          // destination directory path to copy or sync to
        cs commands;                                // holds the arguments whether to copy or monitor, sync, update ect.
    };
}
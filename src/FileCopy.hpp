#pragma once
#include <filesystem>
#include "TM.hpp"
#include "FileParse.hpp"
#include "logger.hpp"

namespace application{
    class FileCopy{
    public:
        FileCopy(std::filesystem::copy_options co,std::shared_ptr<std::vector<copyto>> directories);

        // this function gets a file ready to be copied each call
        // its meant to be called in a loop multiple times
        void copy_file();

        
    private:
        // flags for copying
        std::filesystem::copy_options m_CO;

        // the list of directories to copy from and to
        std::shared_ptr<std::vector<copyto>> m_Directories;
    };
}
#pragma once
#include "TM.hpp"
#include "logger.hpp"
#include "FileParse.hpp"
#include "ConsoleTM.hpp"
#include "DirectorySignal.hpp"
#include <filesystem>
#include "AppMacros.hpp"


#if WINDOWS_BUILD
#include "WinHelper.hpp"
#endif

namespace application{
    class ConsoleApp{
    public:    
        ConsoleApp();

        void Go();
    private:
        // name of the file to edit and store the copy directories
        std::string m_FileName{"sfct_list.txt"};

        // parsed directories
        std::shared_ptr<std::vector<copyto>> m_data;

        FileParse m_List{m_FileName};

        std::unique_ptr<DirectorySignal> m_Monitor;
    };
}

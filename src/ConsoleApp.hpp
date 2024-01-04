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
        
        ~ConsoleApp();
    private:
        // using pointers and memory allocated on the heap to avoid high stack usage
        std::unique_ptr<TM> m_CopyWorkers{std::make_unique<TM>()};
        std::unique_ptr<FileParse> m_List;
        

        // name of the file to edit and store the copy directories
        std::string m_FileName{"sfct_list.txt"};

        // keyword used to parse the data
        std::string m_Keyword{"copy"};

        // parsed directories
        std::shared_ptr<std::vector<copyto>> m_data;

        std::unique_ptr<DirectorySignal> m_Monitor;

        std::shared_ptr<std::queue<copyto>> m_FileQueue;

        void process_queue();

        void InitialCopy();

        std::filesystem::copy_options m_co{std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing};
    };
}

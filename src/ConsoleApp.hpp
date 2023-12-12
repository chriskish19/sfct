#pragma once
#include "TM.hpp"
#include "logger.hpp"
#include "FileParse.hpp"
#include "FileCopy.hpp"
#include "ConsoleTM.hpp"


namespace application{
    class ConsoleApp{
    public:    
        ConsoleApp();

        void Go();
        
    private:
        // using pointers and memory allocated on the heap to avoid high stack usage
        std::unique_ptr<TM> m_CopyWorkers{std::make_unique<TM>()};
        std::unique_ptr<TM> m_MessageWorkers{std::make_unique<TM>()};
        std::unique_ptr<FileParse> m_List;
        std::unique_ptr<FileCopy> m_Copier;
        std::unique_ptr<std::vector<ConsoleTM>> m_MessageStreams{std::make_unique<std::vector<ConsoleTM>>()}; 

        // name of the file to edit and store the copy directories
        std::string m_FileName{"sfct_list.txt"};

        // keyword used to parse the data
        std::string m_Keyword{"copy"};
    };
}

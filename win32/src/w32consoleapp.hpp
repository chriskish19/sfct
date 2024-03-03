#pragma once
#include "w32cpplib.hpp"
#include "w32tm.hpp"
#include "w32logger.hpp"
#include "w32fileparse.hpp"
#include "w32directorysignal.hpp"
#include "w32appmacros.hpp"
#include "w32benchmark.hpp"
#include "w32constants.hpp"
#include "w32directorycopy.hpp"

/////////////////////////////////////////////////////////////////
// This header is responsible for the main object used to run the program.
// ConsoleApp is meant to be instantiated in main.cpp with a call to the function Go().
/////////////////////////////////////////////////////////////////

namespace application{
    class ConsoleApp{
    public:
        // main app constructor init objects here    
        ConsoleApp(); 

        // main app loop                                  
        void Go(); 

        // destructor
        ~ConsoleApp()= default;
        
        // Copy constructor
        ConsoleApp(const ConsoleApp&) = delete;

        // Copy assignment operator
        ConsoleApp& operator=(const ConsoleApp&) = delete;

        // Move constructor
        ConsoleApp(ConsoleApp&&) = delete;

        // Move assignment operator
        ConsoleApp& operator=(ConsoleApp&&) = delete;                                     
    private:
        // name of the file to get the commands from
        std::string m_FileName{"sfct_list.txt"};

        // the parsed commands and directories in copyto struct objects
        std::shared_ptr<std::vector<copyto>> m_data; 

        // the object responsible for parsing the txt file   
        FileParse m_List{m_FileName};

        // the object that monitors directories                   
        std::unique_ptr<DirectorySignal> m_Monitor;

        
        std::shared_ptr<std::vector<copyto>> m_monitor_dirs{std::make_shared<std::vector<copyto>>()};
        std::shared_ptr<std::vector<copyto>> m_fast_copy_dirs{std::make_shared<std::vector<copyto>>()};
        std::shared_ptr<std::vector<copyto>> m_copy_dirs{std::make_shared<std::vector<copyto>>()};
        std::vector<copyto> m_bench_dirs;
    };
}

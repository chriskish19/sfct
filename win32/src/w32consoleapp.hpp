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

//////////////////////////////////////////////////////////////////////////////////////////
// This header is responsible for the main object used to run the program.              //
// ConsoleApp is meant to be instantiated in main.cpp with a call to the function Go(). //
//////////////////////////////////////////////////////////////////////////////////////////

namespace application{
    class ConsoleApp{
    public:
        /// @brief main app constructor init objects here    
        ConsoleApp() noexcept; 

        /// @brief main app function                                  
        void Go() noexcept; 

        /// @brief default destructor
        ~ConsoleApp()= default;
        
        /// @brief delete the Copy constructor
        ConsoleApp(const ConsoleApp&) = delete;

        /// @brief delete the Copy assignment operator
        ConsoleApp& operator=(const ConsoleApp&) = delete;

        /// @brief delete the Move constructor
        ConsoleApp(ConsoleApp&&) = delete;

        /// @brief delete the Move assignment operator
        ConsoleApp& operator=(ConsoleApp&&) = delete;                                     
    private:
        /// @brief name of the file to get the commands from
        std::string m_FileName{"sfct_list.txt"};

        /// @brief the parsed commands and directories in copyto struct objects
        std::shared_ptr<std::vector<copyto>> m_data; 

        /// @brief the object responsible for parsing the txt file   
        FileParse m_List{m_FileName};

        /// @brief the object that monitors directories                   
        std::unique_ptr<DirectorySignal> m_Monitor;

        /// @brief the directories that are to be monitored
        std::shared_ptr<std::vector<copyto>> m_monitor_dirs{std::make_shared<std::vector<copyto>>()};

        /// @brief the directories that are to be fast copied
        std::shared_ptr<std::vector<copyto>> m_fast_copy_dirs{std::make_shared<std::vector<copyto>>()};

        /// @brief the directories that are to be copied
        std::shared_ptr<std::vector<copyto>> m_copy_dirs{std::make_shared<std::vector<copyto>>()};

        /// @brief the directories that are to be benchmarked
        std::vector<copyto> m_bench_dirs;

        /// @brief this function should be called first in the main app constructor, it initializes the app,
        /// extracts the data and fills the vectors by commands 
        /// @return true if no issues occur. False if the file is not present. False if an exception is thrown.
        bool initialize_app() noexcept;

        /// @brief return value from initialize_app() function.
        bool m_init_success{false};
    };
}

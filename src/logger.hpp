#pragma once
#include "AppMacros.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <format>
#include <mutex>
#include <source_location>


// include the windows header on windows builds
#if WINDOWS_BUILD
    #include <Windows.h>
    #define App_MESSAGE(x) L##x
#else
    #define App_MESSAGE(x) x
#endif


namespace application{
    // used in the logger class to classify errors
    enum class Error {
        // if its fatal it will affect the programs execution and most likley an exception will be thrown and the program may exit
        FATAL,

        // small errors that have no effect on execution flow and no exceptions are thrown
        DEBUG,

        // for information to the user
        INFO,

        // more significant errors than a debug message but does not exit the program or throw an exception
        WARNING
    };

    // code for outputting to a log file
    // the file will be created in the current working directory and is called "Applog.txt"
    inline OFSTREAM logFile{std::filesystem::current_path()/"Applog.txt",std::ios::out};
    inline std::mutex logfile_mtx;

/////////////////////////////////////
/* Windows version of logger class */
/////////////////////////////////////
#if WINDOWS_BUILD
    // simple logger class that handles writing to a log file and posting messages
    class logger{
    public:
        logger(const std::wstring& s, Error type, const std::source_location& location = std::source_location::current());

        // special logger constructor useful for working with file paths
        logger(const std::wstring& s,Error type, const std::filesystem::path& filepath,const std::source_location& location = std::source_location::current());

        // log windows errors with this constructor
        logger(Error type, const std::source_location& location = std::source_location::current(), DWORD Win32error = GetLastError());

        // output mMessage to output window in visual studio
        void to_output() const;

        // output mMessage to console
        void to_console() const;

        // output mMessage to a log file
        void to_log_file() const;
    private:
        // default initialization for logger class
        // timestamps mMessage with the current date and time
        void initLogger(); 

        // adds the time to mMessage
        void time_stamp();

        // adds the location to mMessage
        void location_stamp();

        // adds the type of error to the begining of mMessage 
        void initErrorType();

        // the main log message
        std::wstring mMessage;

        const std::source_location m_location;

        const Error m_type;
    };
#endif

///////////////////////////////////
/* Linux version of Logger class */
///////////////////////////////////
#if LINUX_BUILD

    class logger{
    public:
        



    };

#endif


}


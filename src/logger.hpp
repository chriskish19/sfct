#pragma once
#include "AppMacros.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <format>



// include the windows header on windows builds
#ifdef WINDOWS_BUILD
    #include <Windows.h>
#endif

// these macros are for the logger class
// they help pin point errors by giving the line number and file name
// in which they occured
#define App_WSTRINGIFY(x) L#x
#define App_LOCATION std::wstring(L"Line: " App_WSTRINGIFY(__LINE__) L" File: " __FILE__)


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
    
    // simple logger class that handles writing to a log file and posting messages
    // to the console with detailed info, it's not designed to be used with multiple threads, yet anyway.
    // TODO: make it thread safe
    class logger{
    public:
        // standard logger constructor should work cross platform
        logger(const std::wstring& s, Error type, const std::wstring& location);

        // special logger constructor useful for working with file paths
        logger(const std::wstring& s,Error type, const std::filesystem::path& filepath,const std::wstring& location);

        // output mMessage to console
        void to_console() const;

        // output mMessage to a log file
        void to_log_file() const;

    private:
        // default initialization for logger class
        // timestamps mMessage with the current date and time
        void initLogger(); 

        // adds the type of error to the begining of mMessage 
        void initErrorType(Error type = Error::INFO);

        // the main log message
        std::wstring mMessage;

        // code for outputting to a log file
        // the file will be created in the current working directory and is called "Applog.txt"
        inline static std::filesystem::path logfilepath{ std::filesystem::current_path()/"Applog.txt" };
        inline static std::wofstream logFile{logfilepath,std::ios::out};


// TODO: add linux and macos specific error handling
// Windows specific error handling
#ifdef WINDOWS_BUILD
        // log windows errors with this constructor
        logger(Error type, const std::wstring& location, DWORD Win32error = GetLastError());

        // output mMessage to output window in visual studio
        void to_output() const;

        // default initialization code for logger class on windows
        void initLoggerWIN32();

        // adds time stamp to the begining of mMessage on windows
        void timeStampWIN32();

        // true for Console subsystem and false for not
        static bool IsConsoleSubsystem() noexcept;

        // true for windows subsystem and false for not
        static bool IsWindowsSubsystem() noexcept;

        // is the /SUBSYSTEM under linker settings set to console or Windows
        inline static bool SubSysConsole{ IsConsoleSubsystem() };
        inline static bool SubSysWindows{ IsWindowsSubsystem() };

        // true for logger class initialized
        inline static bool Logger_init{ false };
#endif
    };
}


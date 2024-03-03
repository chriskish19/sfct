#pragma once
#include "w32cpplib.hpp"
#include <Windows.h>
#include "w32appmacros.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
// This header is responsible for logging messages to the console, logfile and output window  //                                                 
////////////////////////////////////////////////////////////////////////////////////////////////

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
    inline OFSTREAM logFile{sfct_api::get_current_path()/"Applog.txt",std::ios::out};
    inline std::mutex logfile_mtx;

/////////////////////////////////////
/* Windows version of logger class */
/////////////////////////////////////

    // simple logger class that handles writing to a log file and posting messages
    class logger{
    public:
        // default destructor
        ~logger()= default;
        
        // Copy constructor
        logger(const logger&) = delete;

        // Copy assignment operator
        logger& operator=(const logger&) = delete;

        // Move constructor
        logger(logger&&) = delete;

        // Move assignment operator
        logger& operator=(logger&&) = delete;

        /// @brief logger constructor that takes an std::error_code, filepath, and location
        /// @param ec std::error code object
        /// @param type type of error, INFO, FATAL, ect..
        /// @param filepath the file path which is part of the error
        /// @param location the file, line number, column number, and function that logger() was constructed in.
        logger(const std::error_code& ec,Error type,const std::filesystem::path& filepath,const std::source_location& location = std::source_location::current());

        /// @brief standard logger constructor with custom message
        /// @param s custom message
        /// @param type type of error, INFO, FATAL, ect..
        /// @param location the file, line number, column number, and function that logger() was constructed in.
        logger(const std::wstring& s, Error type, const std::source_location& location = std::source_location::current());

        /// @brief special logger constructor useful for working with file paths
        /// @param s custom message
        /// @param type error severity, INFO ect..
        /// @param filepath the file path which is part of the error
        /// @param location the file, line number, column number, and function that logger() was constructed in.
        logger(const std::wstring& s,Error type, const std::filesystem::path& filepath,const std::source_location& location = std::source_location::current());

        /// @brief log windows errors with this constructor
        /// @param type type of error, INFO, FATAL, ect..
        /// @param location the file, line number, column number, and function that logger() was constructed in.
        /// @param Win32error the windows specific error code
        logger(Error type, const std::source_location& location = std::source_location::current(), DWORD Win32error = GetLastError());

        /// @brief output mMessage to output window in visual studio
        void to_output() const;

        /// @brief output mMessage to console
        void to_console() const;

        /// @brief output mMessage to a log file
        void to_log_file() const;
    private:
        /// @brief default initialization for logger class
        /// timestamps mMessage with the current date and time
        void initLogger(); 

        /// @brief adds the time to mMessage
        void time_stamp();

        /// @brief adds the location to mMessage
        void location_stamp();

        /// @brief adds the type of error to the begining of mMessage 
        void initErrorType();

        /// @brief the main log message
        std::wstring mMessage;

        /// @brief the file name and path
        const std::source_location m_location;

        /// @brief severity of error
        const Error m_type;
    };
} 




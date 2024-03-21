#pragma once
#include "w32cpplib.hpp"
#include <Windows.h>
#include "w32appmacros.hpp"
#include <comdef.h>

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
    inline OFSTREAM logFile{std::filesystem::current_path()/"Applog.txt",std::ios::out};
    inline std::mutex logfile_mtx;

/////////////////////////////////////
/* Windows version of logger class */
/////////////////////////////////////

    // simple logger class that handles writing to a log file and posting messages
    class logger{
    public:
        /// @brief default destructor
        ~logger()= default;
        
        /// @brief delete the Copy constructor
        logger(const logger&) = delete;

        /// @brief delete Copy assignment operator
        logger& operator=(const logger&) = delete;

        /// @brief delete the Move constructor
        logger(logger&&) = delete;

        /// @brief delete Move assignment operator
        logger& operator=(logger&&) = delete;

        /// @brief logger constructor that takes an std::error_code, filepath, and location
        /// @param ec std::error code object
        /// @param type type of error, INFO, FATAL, ect..
        /// @param filepath the file path which is part of the error
        /// @param location the file, line number, column number, and function that logger() was constructed in.
        logger(const std::error_code& ec,Error type,const std::filesystem::path& filepath,const std::source_location& location = std::source_location::current()) noexcept;

        /// @brief standard logger constructor with custom message
        /// @param s custom message
        /// @param type type of error, INFO, FATAL, ect..
        /// @param location the file, line number, column number, and function that logger() was constructed in.
        logger(const std::wstring& s, Error type, const std::source_location& location = std::source_location::current()) noexcept;

        /// @brief special logger constructor useful for working with file paths
        /// @param s custom message
        /// @param type error severity, INFO ect..
        /// @param filepath the file path which is part of the error
        /// @param location the file, line number, column number, and function that logger() was constructed in.
        logger(const std::wstring& s,Error type, const std::filesystem::path& filepath,const std::source_location& location = std::source_location::current()) noexcept;

        /// @brief log windows errors with this constructor
        /// @param type type of error, INFO, FATAL, ect..
        /// @param location the file, line number, column number, and function that logger() was constructed in.
        /// @param Win32error the windows specific error code
        logger(Error type, const std::source_location& location = std::source_location::current(), DWORD Win32error = GetLastError()) noexcept;

		/// @brief log d2d1 HRESULT errors with this constructor
		/// @param type type of error, INFO, FATAL, ect..
		/// @param hr windows error result
		/// @param location the file, line number, column number, and function that logger() was constructed in.
		logger(Error type, HRESULT hr, const std::source_location& location = std::source_location::current());

        /// @brief output mMessage to output window in visual studio
        void to_output() const noexcept;

        /// @brief output mMessage to console
        void to_console() const noexcept;

        /// @brief output mMessage to a log file
        void to_log_file() const noexcept;
    private:
        /// @brief default initialization for logger class
        /// timestamps mMessage with the current date and time
        void initLogger() noexcept; 

        /// @brief adds the time to mMessage
        void time_stamp() noexcept;

        /// @brief adds the location to mMessage
        void location_stamp() noexcept;

        /// @brief adds the type of error to the beginning of mMessage 
        void initErrorType() noexcept;

        /// @brief the main log message
        std::wstring mMessage;

        /// @brief the file name and path
        const std::source_location m_location;

        /// @brief severity of error
        const Error m_type;
    };
} 




#include "logger.hpp"


/////////////////////////////////////////////////
/* Windows version of logger class definitions */
/////////////////////////////////////////////////
#if WINDOWS_BUILD
application::logger::logger(const std::wstring& s, Error type, const std::wstring& location){
    initLogger();
    initErrorType(type);

    // mMessage is timestamped and has error type now add location and s to the end
    mMessage += location + L" Message: " + s;
}

application::logger::logger(Error type, const std::wstring& location, DWORD Win32error){
    initLogger();
    initErrorType(type);

    LPWSTR errorMsgBuffer = nullptr;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        Win32error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&errorMsgBuffer,
        0,
        NULL
    );

    std::wstring win32error_str; // this is initialized to "" by the string class so if errorMsgBuffer is nullptr nothing is added to mMessage
    if (errorMsgBuffer) {
        win32error_str = std::wstring{ errorMsgBuffer };
        LocalFree(errorMsgBuffer);
    }
    else {
        logger log(L"Format message failed", Error::WARNING, App_LOCATION);
        log.to_console();
        log.to_output();
        log.to_log_file();
    }
    
    // mMessage is timestamped and has error type now add win32error and location to the end
    mMessage += win32error_str + location;
}

void application::logger::to_console() const{
    std::wcout << mMessage << std::endl;
}

void application::logger::to_output() const{
    OutputDebugStringW(mMessage.c_str());
}

void application::logger::to_log_file() const{
    std::lock_guard<std::mutex> local_lock(logfile_mtx);
    
    // if logFile is not open send info to console and output window
    if (!logFile.is_open()) {
        logger log(L"failed to open logFile", Error::WARNING, App_LOCATION);
        log.to_console();
        log.to_output();
    }
    
    // write mMessage to the log.txt file
    logFile << mMessage << std::endl;
    
    // if it fails to write mMessage to log.txt, log the fail to the console and output window
    if (logFile.fail()) {
        logger log(L"failed to write to log file", Error::WARNING, App_LOCATION);
        log.to_console();
        log.to_output();
    }

    // important for log files where the latest information should be preserved in case the program crashes or is terminated before exiting normally.
    logFile.flush();
}

void application::logger::initErrorType(Error type) {
    switch (type) {
        case Error::FATAL: { mMessage = L"[FATAL ERROR]" + mMessage; } break;
        case Error::DEBUG: { mMessage = L"[DEBUG ERROR]" + mMessage; } break;
        case Error::INFO: { mMessage = L"[INFO]" + mMessage; } break;
        case Error::WARNING: { mMessage = L"[WARNING]" + mMessage; } break;
        default: { mMessage = L"[UNKNOWN]" + mMessage; } break;
    }
}

void application::logger::initLogger(){
    //Geting Current time
    auto clock = std::chrono::system_clock::now();

    // convert to a string to be displayed in the console
    std::wstring CurrentTime{std::format(L"{:%F %T}", clock)};

    // add brackets for easy reading
    CurrentTime.insert(CurrentTime.begin(),L'[');
    CurrentTime.push_back(L']');

    // add the time to m_Message
    mMessage = CurrentTime + mMessage;
}

application::logger::logger(const std::wstring& s,Error type,const std::filesystem::path& filepath,const std::wstring& location){
    initLogger();
    initErrorType(type);

    // mMessage is timestamped and has error type now add location and s to the end
    mMessage += location + L" Message: " + s;

    // convert the path to a wide string
    std::wstring filepath_wstr{filepath.c_str()};

    // add to filepath_wstr to make it more readable
    filepath_wstr.insert(filepath_wstr.begin(),L'{');
    filepath_wstr.push_back(L'}'); 
}
#endif




///////////////////////////////////////////////
/* Linux version of logger class definitions */
///////////////////////////////////////////////

#if LINUX_BUILD

#endif
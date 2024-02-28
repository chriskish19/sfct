#include "logger.hpp"


/////////////////////////////////////////////////
/* Windows version of logger class definitions */
/////////////////////////////////////////////////
#if WINDOWS_BUILD
application::logger::logger(const std::error_code &ec, Error type, const std::filesystem::path &filepath, const std::source_location &location)
    : m_location(location), m_type(type)
{
    initLogger();
    std::string s_error_message(ec.message());
    std::wstring ws_error_message(s_error_message.begin(),s_error_message.end());
    mMessage += ws_error_message + filepath.wstring();
}

application::logger::logger(const std::wstring &s, Error type, const std::source_location &location)
    : m_location(location), m_type(type)
{
    initLogger();
    mMessage += L" Message: " + s;
}

application::logger::logger(Error type, const std::source_location& location, DWORD Win32error)
:m_location(location),m_type(type){
    initLogger();

    LPWSTR errorMsgBuffer = nullptr;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        Win32error,
        0,
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
        std::wcout << L"Format message failed" << "\n";
    }
    
    // mMessage is timestamped and has error type now add win32error and location to the end
    mMessage += win32error_str;
}

void application::logger::to_console() const{
    std::wcout << mMessage << std::endl;
    
    if(std::wcout.fail()){
        std::wcout.clear();
        std::wcout << "\n";
    }
        
}

void application::logger::to_output() const{
    OutputDebugStringW(mMessage.c_str());
}

void application::logger::to_log_file() const{
    std::lock_guard<std::mutex> local_lock(logfile_mtx);
    
    // if logFile is not open send info to console and output window
    if (!logFile.is_open()) {
        std::wcout << L"log file failed to open" << std::endl;
    }
    
    
    // write mMessage to the log.txt file
    logFile << mMessage << std::endl;
    
    // if it fails to write mMessage to log.txt, log the fail to the console and output window
    if (logFile.fail()) {
        std::wcout << L"failed to write to log file" << std::endl;
    }
}

void application::logger::initErrorType() {
    switch (m_type) {
        case Error::FATAL: { mMessage = L"[FATAL ERROR]" + mMessage; } break;
        case Error::DEBUG: { mMessage = L"[DEBUG ERROR]" + mMessage; } break;
        case Error::INFO: { mMessage = L"[INFO]" + mMessage; } break;
        case Error::WARNING: { mMessage = L"[WARNING]" + mMessage; } break;
        default: { mMessage = L"[UNKNOWN]" + mMessage; } break;
    }
}

void application::logger::initLogger(){
    time_stamp();
    initErrorType();
    location_stamp();
}

application::logger::logger(const std::wstring& s,Error type,const std::filesystem::path& filepath,const std::source_location& location)
:m_location(location),m_type(type){
    initLogger();
    mMessage += L" Message: " + s;

    mMessage += filepath.wstring();
}

void application::logger::time_stamp(){
    //Geting Current time
    auto clock = std::chrono::system_clock::now();

    // add the time to the message
    mMessage = std::format(L"[{:%F %T}] {}", clock, mMessage);
}

void application::logger::location_stamp(){
    std::string file_name(m_location.file_name());
    std::string function_name(m_location.function_name());
    std::string line(std::to_string(m_location.line()));

    std::wstring w_file_name(file_name.begin(),file_name.end());
    std::wstring w_function_name(function_name.begin(),function_name.end());
    std::wstring w_line(line.begin(),line.end());

    mMessage += std::format(L"File: {} Line: {} Function: {}",w_file_name,w_line,w_function_name);
}
#endif




///////////////////////////////////////////////
/* Linux version of logger class definitions */
///////////////////////////////////////////////

#if LINUX_BUILD

#endif
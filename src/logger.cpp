#include "logger.hpp"

application::logger::logger(const std::wstring& s, Error type, const std::wstring& location){
    initLogger();
    initErrorType(type);

    // mMessage is timestamped and has error type now add location and s to the end
    mMessage += location + L" Message: " + s;
}

#ifdef WINDOWS_BUILD
application::logger::logger(Error type, const std::wstring& location, DWORD Win32error = GetLastError()){
    initLoggerWIN32();
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
#endif


void application::logger::to_console() const{
    std::wcout << mMessage << std::endl;
}

#ifdef WINDOWS_BUILD
void application::logger::to_output() const{
    OutputDebugStringW(mMessage.c_str());
}
#endif

void application::logger::to_log_file() const{
    // if logFile is not open send info to console and output window
    if (!logFile.is_open()) {
        logger log(L"failed to open logFile", Error::WARNING, App_LOCATION);
        log.to_console();
        #ifdef WINDOWS_BUILD
        log.to_output();
        #endif
    }
    
    // write mMessage to the log.txt file
    logFile << mMessage << std::endl;
    
    // if it fails to write mMessage to log.txt, log the fail to the console and output window
    if (logFile.fail()) {
        logger log(L"failed to write to log file", Error::WARNING, App_LOCATION);
        log.to_console();
        #ifdef WINDOWS_BUILD
        log.to_output();
        #endif
    }

    // important for log files where the latest information should be preserved in case the program crashes or is terminated before exiting normally.
    logFile.flush();
}

#ifdef WINDOWS_BUILD
void application::logger::initLoggerWIN32(){
    // since any header could use a similar logger we need to guard agianst reintializing the console
    // each time a logger is constructed
    if (!Logger_init && SubSysWindows && GetConsoleWindow()==nullptr) {
        AllocConsole();

        // Redirect the CRT standard input, output, and error handles to the console
        FILE* stream;

        freopen_s(&stream, "CONIN$", "r", stdin);
        freopen_s(&stream, "CONOUT$", "w", stdout);
        freopen_s(&stream, "CONOUT$", "w", stderr);

        std::cout << "Outputting to the new console!" << std::endl;
    }

    // set to true since logger class has been initialized
    Logger_init = true;

    // add time to mMessage
    timeStampWIN32();
}

void application::logger::timeStampWIN32(){
    //Geting Current time
    auto clock = std::chrono::system_clock::now();

    // convert to a std::time_t object which _wctime_s accepts
    std::time_t CurrentTime = std::chrono::system_clock::to_time_t(clock);

    // buffer for _wctime_s
    wchar_t TimeBuff[30];

    // Converts the time_t object CurrentTime into a wchar_t 
    _wctime_s(TimeBuff, sizeof(TimeBuff) / sizeof(wchar_t), &CurrentTime);

    // put the TimeBuff into a wide string to make it easy to modify
    std::wstring CurrentTime_wstr{ TimeBuff };

    // _wctime_s puts a newline char at the end of the time stamp
    // I dont want a newline
    if (CurrentTime_wstr.ends_with(L'\n')) {
        CurrentTime_wstr.erase(std::prev(CurrentTime_wstr.end()));
    }

    // add brackets for easy reading
    CurrentTime_wstr.insert(CurrentTime_wstr.begin(), '[');
    CurrentTime_wstr.push_back(']');

    // mMessage now has the time stamp
    mMessage = CurrentTime_wstr + mMessage;
}
#endif

void application::logger::initErrorType(Error type){
    switch (type) {
        case Error::FATAL: { mMessage = L"[FATAL ERROR]" + mMessage; } break;
        case Error::DEBUG: { mMessage = L"[DEBUG ERROR]" + mMessage; } break;
        case Error::INFO: { mMessage = L"[INFO]" + mMessage; } break;
        case Error::WARNING: { mMessage = L"[WARNING]" + mMessage; } break;
        default: { mMessage = L"[UNKNOWN]" + mMessage; } break;
    }
}


#ifdef WINDOWS_BUILD
bool application::logger::IsConsoleSubsystem(){
    // Get the HMODULE of the current process.
    HMODULE hModule = GetModuleHandle(NULL);

    // Get the DOS header.
    PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;

    // From the DOS header, get the NT (PE) header.
    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)hModule + pDOSHeader->e_lfanew);

    // Check the Subsystem value in the Optional Header.
    return pNTHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI;
}

bool application::logger::IsWindowsSubsystem(){
    // Get the HMODULE of the current process.
    HMODULE hModule = GetModuleHandle(NULL);

    // Get the DOS header.
    PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;

    // From the DOS header, get the NT (PE) header.
    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)hModule + pDOSHeader->e_lfanew);

    // Check the Subsystem value in the Optional Header.
    return pNTHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI;
}
#endif

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
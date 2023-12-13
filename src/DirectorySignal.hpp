#pragma once
#include <filesystem>
#include "AppMacros.hpp"
#include "logger.hpp"



/////////////////////////////////////////////////////////////////////
/* Future TODO:                                                    */
/* 1. Linux version of the class                                   */
/* 2. MacOS version of the class                                   */
/*                                                                 */
/* INFO:                                                           */
/* The purpose of this class is to monitor a directory for changes */
/////////////////////////////////////////////////////////////////////






// Windows specific version of the DirectorySignal class
#ifdef WINDOWS_BUILD
#include <Windows.h>


namespace application{
    class DirectorySignal{
    public:
        DirectorySignal(std::filesystem::path path_to_watch);
        ~DirectorySignal();


        bool signal();

    private:
        void process_changes();


        // m_hDirectory is a handle to the opened directory. 
        // In Windows programming, handles are used to identify and manage resources, such as files, directories, processes, and more.
        HANDLE m_hDirectory;

        BYTE m_buffer[4096];
        DWORD m_bytesRead;
    };
}

#endif

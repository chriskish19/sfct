#pragma once
#include <filesystem>
#include "appMacros.hpp"
#include "Logger.hpp"
#include "FileParse.hpp"
#include "ConsoleTM.hpp"
#include "helper.hpp"

/////////////////////////////////////////////////////////////////////
// This header is responsible for monitoring directories for changes
//
// Future TODO:                                                    
// 1. Linux version of the class                                   
// 2. MacOS version of the class                                                                                                  
/////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
/* Windows specific version of the DirectorySignal class */
///////////////////////////////////////////////////////////
#if WINDOWS_BUILD
#include <Windows.h>
namespace application{
    struct DS_resources {
        HANDLE m_hDir;
        BYTE m_buffer[10485760]; // 10MB buffer, do not allocate this on the stack
        OVERLAPPED m_ol;
        copyto directory;
    }; 

    class DirectorySignal{
    public:
        DirectorySignal(std::shared_ptr<std::vector<copyto>> dirs_to_watch);
        ~DirectorySignal();
        void monitor();
        DWORD GetNotifyFilter(){return m_NotifyFilter;}
        HANDLE GetCompletionPort(){return m_hCompletionPort;}
    private:
        DWORD m_NotifyFilter{FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME};
        HANDLE m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        std::vector<DS_resources*> m_pMonitors;
        std::shared_ptr<std::vector<copyto>> m_Dirs;
    };
}
#endif

/////////////////////////////////////////////
/* Linux version of directory signal class */
/////////////////////////////////////////////
#if LINUX_BUILD
namespace application{
    class DirectorySignal{

    };
}

#endif
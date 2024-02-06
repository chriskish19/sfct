#pragma once
#include <filesystem>
#include "appMacros.hpp"
#include "Logger.hpp"
#include "FileParse.hpp"
#include "ConsoleTM.hpp"
#include "helper.hpp"
#include <queue>
#include "constants.hpp"
#include <unordered_set>
#include "sfct_api.hpp"
#include "queue_system.hpp"


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
        BYTE m_buffer[MonitorBuffer]; // 10MB buffer, do not allocate this on the stack, value defined in constants.hpp
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
        bool no_watch{false};
        std::queue<std::filesystem::path> m_directory_remove;   // directories set for deletion
        
        // loop through the queue and delete directories
        void EmptyQueue(); 

        // check if the monitored directory buffer has overflowed
        bool Overflow(DWORD bytes_returned,DS_resources* p_monitor);

        // calls ReadDirectoryChanges
        void UpdateWatcher(DS_resources* p_monitor);

        // checks for recursive flag
        bool RecursiveFlagCheck(cs command);

        // go through all the notifications from the watched directory
        void ProcessDirectoryChanges(FILE_NOTIFY_INFORMATION* pNotify,DS_resources* pMonitor);

        queue_system<file_queue_info> m_queue_processer;
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
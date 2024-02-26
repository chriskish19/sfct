#pragma once
#include <filesystem>
#include "appMacros.hpp"
#include "Logger.hpp"
#include "FileParse.hpp"
#include "ConsoleTM.hpp"
#include <queue>
#include "constants.hpp"
#include <unordered_set>
#include "sfct_api.hpp"
#include "queue_system.hpp"
#include "timer.hpp"
#include "TM.hpp"
#include <future>


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
        DirectorySignal(std::shared_ptr<std::vector<copyto>> dirs_to_watch) noexcept;
        ~DirectorySignal();
        DirectorySignal(const DirectorySignal&) = delete;
        DirectorySignal& operator=(const DirectorySignal&) = delete;
        
        void monitor() noexcept;
        DWORD GetNotifyFilter() noexcept {return m_NotifyFilter;}
        HANDLE GetCompletionPort() noexcept {return m_hCompletionPort;}
    private:
        DWORD m_NotifyFilter{FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_SIZE};
        HANDLE m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        std::vector<DS_resources*> m_pMonitors;
        std::shared_ptr<std::vector<copyto>> m_dirs;
        bool no_watch{false}; 

        // check if the monitored directory buffer has overflowed
        bool Overflow(DWORD bytes_returned) noexcept;

        // calls ReadDirectoryChanges
        void UpdateWatcher(DS_resources* p_monitor) noexcept;

        // go through all the notifications from the watched directory
        void ProcessDirectoryChanges(FILE_NOTIFY_INFORMATION* pNotify,DS_resources* pMonitor) noexcept;

        queue_system<file_queue_info> m_queue_processor;
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
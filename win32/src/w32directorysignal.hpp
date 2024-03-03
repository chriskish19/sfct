#pragma once
#include <filesystem>
#include "w32logger.hpp"
#include "w32fileparse.hpp"
#include "w32consoletm.hpp"
#include <queue>
#include "w32constants.hpp"
#include <unordered_set>
#include "w32sfct_api.hpp"
#include "w32queue_system.hpp"
#include "w32timer.hpp"
#include "w32tm.hpp"
#include <future>
#include <Windows.h>

///////////////////////////////////////////////////////////////////////
// This header is responsible for monitoring directories for changes //                                                                                                 
///////////////////////////////////////////////////////////////////////

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

        // Copy constructor
        DirectorySignal(const DirectorySignal&) = delete;

        // Copy assignment operator
        DirectorySignal& operator=(const DirectorySignal&) = delete;
        
        // Move constructor
        DirectorySignal(DirectorySignal&&) = delete;

        // Move assignment operator
        DirectorySignal& operator=(DirectorySignal&&) = delete;

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

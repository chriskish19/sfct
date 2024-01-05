#pragma once
#include <filesystem>
#include "AppMacros.hpp"
#include "logger.hpp"
#include <vector>
#include "FileParse.hpp"
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ConsoleTM.hpp"
#include "Helper.hpp"


/////////////////////////////////////////////////////////////////////
/* Future TODO:                                                    */
/* 1. Linux version of the class                                   */
/* 2. MacOS version of the class                                   */
/*                                                                 */
/* INFO:                                                           */
/* The purpose of this class is to monitor a directory for changes */
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
        std::filesystem::path src_path;
        std::filesystem::path dest_path;
    }; 

    class DirectorySignal{
    public:
        DirectorySignal(std::shared_ptr<std::vector<copyto>> dirs_to_watch);
        ~DirectorySignal();


        void monitor();

        DWORD GetNotifyFilter(){return m_NotifyFilter;}
        HANDLE GetCompletionPort(){return m_hCompletionPort;}

        std::shared_ptr<std::queue<copyto>> GetFileQueueSP(){return m_ReadyFiles;}
    private:

        DWORD m_NotifyFilter{FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME};


        HANDLE m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

        std::vector<DS_resources*> m_pMonitors;

        // files queued for copying
        std::shared_ptr<std::queue<copyto>> m_ReadyFiles{std::make_shared<std::queue<copyto>>()};

        std::shared_ptr<std::vector<copyto>> m_Dirs;

        CONSOLETM m_MessageStream;

        std::filesystem::copy_options m_co{std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing};
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
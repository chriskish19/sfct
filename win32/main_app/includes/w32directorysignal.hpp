#pragma once
#include "w32cpplib.hpp"
#include "w32logger.hpp"
#include "w32fileparse.hpp"
#include "w32consoletm.hpp"
#include "w32constants.hpp"
#include "w32sfct_api.hpp"
#include "w32queue_system.hpp"
#include "w32timer.hpp"
#include "w32tm.hpp"
#include <windows.h>

///////////////////////////////////////////////////////////////////////
// This header is responsible for monitoring directories for changes //                                                                                                 
///////////////////////////////////////////////////////////////////////

namespace application{
    /// @brief Directory signal resources structure.
    /// Used in the monitor function in the class DirectorySignal, it is the object returned by
    /// GetQueuedCompletionStatus() when a change is detected in the monitored directory.
    struct DS_resources {
        HANDLE m_hDir;                      // handle to the source directory to be monitored  
        BYTE m_buffer[MonitorBuffer];       // 10MB buffer, do not allocate this on the stack, value defined in constants.hpp
        OVERLAPPED m_ol;                    // for multithreading
        copyto directory;                   // the directories, copy options and cs commands
    }; 

    /// @brief Encapsulates monitoring multiple directories.
    /// Any objects allocated with new are destroyed in the destructor.
    class DirectorySignal{
    public:
        /// @brief Sets up the directories to watch by calling windows api functions.
        /// @param dirs_to_watch The directories, copy options and cs commands.
        DirectorySignal(std::shared_ptr<std::vector<copyto>> dirs_to_watch) noexcept;
        
        /// @brief Cleans up any objects allocated with new.
        ~DirectorySignal() noexcept;

        /// @brief Copy constructor.
        DirectorySignal(const DirectorySignal&) = delete;

        /// @brief Copy assignment operator.
        DirectorySignal& operator=(const DirectorySignal&) = delete;
        
        /// @brief Move constructor.
        DirectorySignal(DirectorySignal&&) = delete;

        /// @brief Move assignment operator.
        DirectorySignal& operator=(DirectorySignal&&) = delete;

        /// @brief The main monitor loop.
        void monitor() noexcept;
    private:
        /// @brief Tells the windows api function ReadDirectoryChanges() to respond to these changes.
        /// 
        /// FILE_NOTIFY_CHANGE_FILE_NAME: 
        /// Any file name change in the watched directory or subtree causes a change notification wait operation to return. 
        /// Changes include renaming, creating, or deleting a file. 
        /// 
        /// FILE_NOTIFY_CHANGE_DIR_NAME: 
        /// Any directory-name change in the watched directory or subtree causes a change notification wait operation to return. 
        /// Changes include creating or deleting a directory. 
        /// 
        /// FILE_NOTIFY_CHANGE_SIZE: 
        /// Any file-size change in the watched directory or subtree causes a change notification wait operation to return. 
        /// The operating system detects a change in file size only when the file is written to the disk. 
        /// For operating systems that use extensive caching, detection occurs only when the cache is sufficiently flushed. 
        DWORD m_NotifyFilter{FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_SIZE};

        /// @brief 
        /// CreateIoCompletionPort: 
        /// This is the Windows API function that creates an I/O completion port or associates an existing file handle with an existing I/O completion port.
        /// 
        /// INVALID_HANDLE_VALUE: 
        /// By passing this value as the first parameter, you're instructing CreateIoCompletionPort to create a new I/O completion 
        /// port rather than associating it with an existing file handle. 
        /// If you were associating an existing file handle (such as a handle to a file, socket, or pipe), you would pass that handle instead.
        ///
        /// NULL: 
        /// This specifies that the newly created I/O completion port is not being associated with an existing I/O completion port. 
        /// If you were associating a file handle with an existing I/O completion port, you would pass the handle to the existing I/O completion port here.
        ///
        /// 0: 
        /// The third parameter is a completion key that gets associated with the handle. 
        /// Since this call is to create a new completion port (and not associating a handle), this value is ignored. 
        /// When associating a file handle with the port, this key can be returned by GetQueuedCompletionStatus to help identify the source of the I/O operation.
        /// 
        /// 0: 
        /// The fourth parameter specifies the number of threads allowed to execute concurrently on the completion port. 
        /// A value of 0 means that the system allows a concurrency value equal to the number of processor cores on the system. 
        /// This is often the recommended setting for most applications to maximize efficiency without creating too many threads, which could lead to context-switching overhead.
        HANDLE m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

        /// @brief Keeps track of heap allocated DS_resources objects.
        /// Then on exit the destructor is called and cleans up the memory.
        std::vector<DS_resources*> m_pMonitors;

        /// @brief the vector of directories to be monitored
        std::shared_ptr<std::vector<copyto>> m_dirs;
        
        /// @brief causes the program to skip monitoring if true
        bool no_watch{false}; 

        /// @brief Check if the monitored directory buffer has overflowed.
        /// @param bytes_returned number of bytes returned by GetQueueCompletionStatus()
        /// zero indicates buffer overflow
        bool Overflow(DWORD bytes_returned) noexcept;

        /// @brief Calls ReadDirectoryChanges() from the windows api.
        /// @param p_monitor pointer to the buffer and monitored directory
        void UpdateWatcher(DS_resources* p_monitor) noexcept;

        /// @brief Go through all the notifications from the watched directory.
        /// @param pNotify pointer to the notifcations
        /// @param pMonitor pointer to the buffer and monitored directory
        void ProcessDirectoryChanges(FILE_NOTIFY_INFORMATION* pNotify,DS_resources* pMonitor) noexcept;

        /// @brief processes the entries on a seperate thread
        queue_system<file_queue_info> m_queue_processor;
    };
}

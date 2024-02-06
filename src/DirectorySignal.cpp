#include "DirectorySignal.hpp"

//////////////////////////////////////////////////////////
/* Windows version of diretory signal class definitions */
//////////////////////////////////////////////////////////
#if WINDOWS_BUILD

application::DirectorySignal::DirectorySignal(std::shared_ptr<std::vector<copyto>> dirs_to_watch)
:m_Dirs(dirs_to_watch){
    
    if(!dirs_to_watch){
        logger log(App_MESSAGE("nullptr"),Error::FATAL);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("");
    }
    
    if(dirs_to_watch->empty()){
        no_watch = true;
    }

    for(const auto &dir:*dirs_to_watch){
        HANDLE hDir = CreateFile(
            dir.source.c_str(), FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL);

        if (hDir == INVALID_HANDLE_VALUE) {
            logger log(Error::WARNING);
            log.to_console();
            log.to_log_file();
            log.to_output();
            continue;
        }

        DS_resources* monitor = new DS_resources{hDir, {}, {}, {{dir.source},{dir.destination},{dir.commands},{dir.co}}};
        
        
        if(!CreateIoCompletionPort(hDir, m_hCompletionPort, (ULONG_PTR)monitor, 0)){
            logger log(Error::WARNING);
            log.to_console();
            log.to_log_file();
            log.to_output();
        }

        UpdateWatcher(monitor);
        m_pMonitors.push_back(monitor);
    }
}

application::DirectorySignal::~DirectorySignal(){
    // Cleanup
    for (auto monitor : m_pMonitors) {
        CloseHandle(monitor->m_hDir);
        if(monitor){
            delete monitor;
        }
    }
    CloseHandle(m_hCompletionPort);
}

void application::DirectorySignal::monitor(){
    // no monitor directories set so exit the monitor function
    if(no_watch) return;
    
    // Process notifications
    DWORD bytesTransferred;
    DS_resources* pMonitor;
    LPOVERLAPPED pOverlapped;

    m_MessageStream.SetMessage(App_MESSAGE("Waiting in Queue"));
    m_MessageStream.ReleaseBuffer();
    
    while (GetQueuedCompletionStatus(m_hCompletionPort, &bytesTransferred, (PULONG_PTR)&pMonitor, &pOverlapped, INFINITE)) {
        Overflow(bytesTransferred,pMonitor);
        
        // Process change notification in pMonitor->buffer
        // Pointer to the first notification
        FILE_NOTIFY_INFORMATION* pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&pMonitor->m_buffer);
        ProcessDirectoryChanges(pNotify,pMonitor);
        EmptyQueue();

        UpdateWatcher(pMonitor);
        m_MessageStream.SetMessage(App_MESSAGE("Waiting in Queue"));
        m_MessageStream.ReleaseBuffer();
    }
}

void application::DirectorySignal::EmptyQueue()
{
    while(!m_directory_remove.empty()){
        std::filesystem::path dest_to_remove = m_directory_remove.front();
        if(std::filesystem::exists(dest_to_remove)){
            m_MessageStream.SetMessage(App_MESSAGE("Removing Directory: ") + STRING(dest_to_remove));
            uintmax_t files_removed = std::filesystem::remove_all(dest_to_remove);
            if(!files_removed){
                logger log(App_MESSAGE("Failed to remove directory"),Error::WARNING,dest_to_remove);
                log.to_console(); 
                log.to_log_file();
                log.to_output();
            }
            else{
                m_MessageStream.SetMessage(App_MESSAGE("Directory Removed along with containing files: ") + 
                                            STRING(dest_to_remove) + App_MESSAGE(" Total files removed: ") +
                                            TOSTRING(files_removed));
            }   
        }
        m_directory_remove.pop();
    }
}

bool application::DirectorySignal::Overflow(DWORD bytes_returned, DS_resources* p_monitor)
{
    if(bytes_returned == MonitorBuffer){
        m_MessageStream.SetMessage(App_MESSAGE("The monitoring buffer has overflowed"));
        return true;
    }
    return false;
}

void application::DirectorySignal::UpdateWatcher(DS_resources* p_monitor)
{
    if(!ReadDirectoryChangesW(
            p_monitor->m_hDir, 
            &p_monitor->m_buffer, 
            sizeof(p_monitor->m_buffer), 
            RecursiveFlagCheck(p_monitor->directory.commands), // watch subtree
            m_NotifyFilter,
            NULL, 
            &p_monitor->m_ol, 
            NULL)){
                logger log(Error::WARNING);
                log.to_console();
                log.to_log_file();
                log.to_output();
            }
}

bool application::DirectorySignal::RecursiveFlagCheck(cs command)
{
    return ((command & cs::recursive) != cs::none);
}

void application::DirectorySignal::ProcessDirectoryChanges(FILE_NOTIFY_INFORMATION *pNotify,DS_resources* pMonitor)
{
    // Process each notification
    do{
        // Extract the file name included directories in the monitored tree
        std::wstring fileName(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));
        
        // setup a file_queue_info structure
        file_queue_info entry;

        // fill the structure with data
        entry.src = pMonitor->directory.source/fileName;
        entry.dst = pMonitor->directory.destination/fileName;
        entry.co = pMonitor->directory.co;
        entry.fs = std::filesystem::status(entry.src);

        // Process the file change
        switch (pNotify->Action) {
            case FILE_ACTION_MODIFIED:{
                entry.fqs = file_queue_status::file_updated;
                break;
            }
            case FILE_ACTION_ADDED:{
                entry.fqs = file_queue_status::file_added;
                break;
            }
            case FILE_ACTION_REMOVED:{
                entry.fqs = file_queue_status::file_removed;
                break;
            }
            case FILE_ACTION_RENAMED_OLD_NAME:
                // for future
                break;
            case FILE_ACTION_RENAMED_NEW_NAME:
                // for future
                break;
            default:
                break;
        }

        // add the entry to queue system
        m_queue_processer.add_to_queue(entry);

        // check if there is no more data in pNotify
        if(pNotify->NextEntryOffset==0){
            break;
        }

        pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<unsigned char*>(pNotify) + pNotify->NextEntryOffset);
    }while(true); 
}


#endif

/////////////////////////////////////////////////////////////
/* Linux version of the directory signal class definitions */
/////////////////////////////////////////////////////////////
#if LINUX_BUILD
#endif
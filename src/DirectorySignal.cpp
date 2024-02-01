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
        if(RecursiveFlagCheck(dir.commands)){
            std::unordered_set<std::filesystem::path> paths;
            for(const auto& entry:std::filesystem::recursive_directory_iterator(dir.destination)){
                paths.emplace(entry.path());
            }
            m_dst_init_mp.emplace(dir.destination,paths);
        }
        else{
            std::unordered_set<std::filesystem::path> paths;
            for(const auto& entry:std::filesystem::directory_iterator(dir.destination)){
                paths.emplace(entry.path());
            }
            m_dst_init_mp.emplace(dir.destination,paths);
        }
        
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
        if(!pMonitor){
            logger log(Error::FATAL);
            log.to_console();
            log.to_log_file();
            log.to_output();
            throw std::runtime_error("");
        }
        
        if(Overflow(bytesTransferred,pMonitor)){
            continue;
        }
        
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
        
        std::optional<std::shared_ptr<std::unordered_set<std::filesystem::path>>> p_dir_changes;

        std::filesystem::path src(p_monitor->directory.source);
        std::filesystem::path dst(p_monitor->directory.destination);
        std::filesystem::copy_options co = p_monitor->directory.co;

        if(RecursiveFlagCheck(p_monitor->directory.commands)){
            p_dir_changes = DirectoryDifferenceRecursive(src,dst);
        }
        else{
            p_dir_changes = DirectoryDifferenceSingle(src,dst);
        }

        if(p_dir_changes.has_value()){
            // for now we ignore dst differences due to the fact of deleting files could be problematic if a directory is specified and it has alot of files already in it
            // I want sync to work only when it is monitoring src, any prevoius files in the dst directory are ignored and left alone
            auto found = m_dst_init_mp.find(dst);
            if(found != m_dst_init_mp.end()){
                // inital files and entries in dst
                // we want to ignore all intial files
                for(const auto& entry:found->second){
                    auto found_entry = p_dir_changes.value()->find(entry);
                    if(found_entry != p_dir_changes.value()->end()){
                        p_dir_changes.value()->erase(found_entry);
                    }
                }
            }

            // now we can check if the files are avaliable
            // and copy them or delete them
            for(const auto& path:*p_dir_changes.value()){
                sfct_api::FileCheck(path);

                if(FindDirectoryPaths(src,path)){
                    // copy
                    if(std::filesystem::is_regular_file(path)){
                        auto relativePath = std::filesystem::relative(path,src);
                        std::filesystem::path dst_file_path(dst/relativePath);
                        std::filesystem::copy_file(path,dst_file_path,co);
                    }
                    else if(std::filesystem::is_directory(path)){
                        
                    }
                    
                }
                else if(FindDirectoryPaths(dst,path)){
                    // delete
                    std::filesystem::remove(path);
                }
            }


        }

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
        // Extract the file name
        std::wstring fileName(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));
        std::filesystem::path src(pMonitor->directory.source/fileName);
        std::filesystem::path dest(pMonitor->directory.destination/fileName);
        std::filesystem::path dest_dir(pMonitor->directory.destination/fileName);
        dest_dir.remove_filename();
        CDirectory(dest_dir);
        
        // Process the file change
        switch (pNotify->Action) {
            case FILE_ACTION_MODIFIED:
            case FILE_ACTION_ADDED:{
                // unfortunately there is no way to know if the file has completed the copy into the source
                // directory from the terminal in windows or another program so I use a pool checker
                // which is not ideal but no other way exists
                // im looking into using NTFS change journals
                m_MessageStream.SetMessage(App_MESSAGE("Checking for availability: ") + fileName);
                while(!FileReady(src)){
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }


                m_MessageStream.SetMessage(App_MESSAGE("Copying File: ") + fileName);
                if(std::filesystem::is_regular_file(src)){
                    if(!std::filesystem::copy_file(src,dest,pMonitor->directory.co)){
                        m_MessageStream.SetMessage(App_MESSAGE("Skipping File: ") + fileName);
                    }
                }
                else if(std::filesystem::is_directory(src)){
                    if(!std::filesystem::create_directory(dest)){
                        logger log(App_MESSAGE("Failed to create directories"),Error::WARNING,dest_dir);
                        log.to_console();
                        log.to_log_file();
                        log.to_output();
                    }
                }
                break;
            }
            case FILE_ACTION_REMOVED:
                if((pMonitor->directory.commands & cs::sync) != cs::none){
                    if(std::filesystem::is_regular_file(dest) && std::filesystem::exists(dest)){
                        m_MessageStream.SetMessage(App_MESSAGE("Removing File: ") + fileName);
                        if(!std::filesystem::remove(dest)){
                            logger log(App_MESSAGE("Failed to remove file"),Error::WARNING,dest);
                            log.to_console();
                            log.to_log_file();
                            log.to_output();
                        }
                    }
                    else if(std::filesystem::is_directory(dest)){
                        m_directory_remove.emplace(dest);
                    }
                }
                break;
            case FILE_ACTION_RENAMED_OLD_NAME:
                // for future
                break;
            case FILE_ACTION_RENAMED_NEW_NAME:
                // for future
                break;
            default:
                break;
        }

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
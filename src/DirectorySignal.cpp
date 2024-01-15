#include "DirectorySignal.hpp"


//////////////////////////////////////////////////////////
/* Windows version of diretory signal class definitions */
//////////////////////////////////////////////////////////


#if WINDOWS_BUILD
application::DirectorySignal::DirectorySignal(std::shared_ptr<std::vector<copyto>> dirs_to_watch)
:m_Dirs(dirs_to_watch){

    if(!dirs_to_watch){
        logger log(L"nullptr",Error::FATAL);
        log.to_console();
        log.to_log_file();
        log.to_output();
        throw std::runtime_error("nullptr");
    }

    for(const auto &dir:*dirs_to_watch){
        if(dir.commands.find(cs::monitor)!=dir.commands.end()){
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

            DS_resources* monitor = new DS_resources{hDir, {}, {}, {{dir.source},{dir.destination},{dir.commands}}};
            
            
            if(!CreateIoCompletionPort(hDir, m_hCompletionPort, (ULONG_PTR)monitor, 0)){
                logger log(Error::WARNING);
                log.to_console();
                log.to_log_file();
                log.to_output();
            }
            
            
            if(!ReadDirectoryChangesW(
                hDir, 
                &monitor->m_buffer, 
                sizeof(monitor->m_buffer), 
                TRUE,
                m_NotifyFilter,
                NULL, 
                &monitor->m_ol, 
                NULL)){
                    logger log(Error::WARNING);
                    log.to_console();
                    log.to_log_file();
                    log.to_output();
                }

            m_pMonitors.push_back(monitor);
        }
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
    // Process notifications
    DWORD bytesTransferred;
    DS_resources* pMonitor;
    LPOVERLAPPED pOverlapped;

    m_MessageStream.SetMessage(App_MESSAGE("Waiting in Queue"));
    m_MessageStream.ReleaseBuffer();
    
    while (GetQueuedCompletionStatus(m_hCompletionPort, &bytesTransferred, (PULONG_PTR)&pMonitor, &pOverlapped, INFINITE)) {
        // Process change notification in pMonitor->buffer
        // Pointer to the first notification
        FILE_NOTIFY_INFORMATION* pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&pMonitor->m_buffer);
        
        // Process each notification
        do{
            // Extract the file name
            std::wstring fileName(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));
            
            std::filesystem::path src(pMonitor->directory.source/fileName);
            std::filesystem::path dest(pMonitor->directory.destination/fileName);

            std::filesystem::path dest_dir(pMonitor->directory.destination/fileName);


            dest_dir.remove_filename();

            
            if(!std::filesystem::exists(dest_dir)){
                if(std::filesystem::create_directories(dest_dir)){
                    logger log(App_MESSAGE("Failed to create directories"),Error::WARNING,dest_dir);
                    log.to_console();
                    log.to_log_file();
                    log.to_output();
                }
            }
            

            // unfortunately there is no way to know if the file has completed the copy into the source
            // directory from file explorer.exe or the terminal in windows so I use a pool checker
            // which is not ideal but no other way exists
            if(std::filesystem::is_regular_file(src)){
                std::fstream file;
                while(!file.is_open()){
                    m_MessageStream.SetMessage(App_MESSAGE("Waiting for added files"));
                    file.open(src, std::ifstream::in | std::ifstream::binary);
                    if(!file.is_open()){
                        std::this_thread::sleep_for(std::chrono::seconds(5));
                    }
                }
                file.close();
            }
            

            m_MessageStream.SetMessage(App_MESSAGE("Copying File: ") + fileName);

            // Process the file change
            switch (pNotify->Action) {
                case FILE_ACTION_MODIFIED:
                case FILE_ACTION_ADDED:{
                    if(std::filesystem::is_regular_file(src)){
                        std::filesystem::copy_file(src,dest,m_co);
                    }
                    else if(std::filesystem::is_directory(src)){
                        std::filesystem::create_directory(dest);
                    }
                    break;
                }
                case FILE_ACTION_REMOVED:
                    // Handle file removed
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

        
        if(bytesTransferred == sizeof(pMonitor->m_buffer)){
            m_MessageStream.SetMessage(App_MESSAGE("Performing a full copy/update to all directories as the monitoring buffer has overflowed"));
            application::FullCopy(m_Dirs);
        }
        

        if(!ReadDirectoryChangesW(
            pMonitor->m_hDir, 
            &pMonitor->m_buffer, 
            sizeof(pMonitor->m_buffer), 
            TRUE,
            m_NotifyFilter,
            NULL, 
            &pMonitor->m_ol, 
            NULL)){
                logger log(Error::WARNING);
                log.to_console();
                log.to_log_file();
                log.to_output();
            }

            m_MessageStream.SetMessage(App_MESSAGE("Waiting in Queue"));
            m_MessageStream.ReleaseBuffer();
    }
}
#endif




/////////////////////////////////////////////////////////////
/* Linux version of the directory signal class definitions */
/////////////////////////////////////////////////////////////

#if LINUX_BUILD



#endif
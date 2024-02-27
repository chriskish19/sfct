#include "DirectorySignal.hpp"

//////////////////////////////////////////////////////////
/* Windows version of diretory signal class definitions */
//////////////////////////////////////////////////////////
#if WINDOWS_BUILD

application::DirectorySignal::DirectorySignal(std::shared_ptr<std::vector<copyto>> dirs_to_watch) noexcept
:m_dirs(dirs_to_watch){

    try{
        if(!dirs_to_watch){
            logger log(App_MESSAGE("nullptr"),Error::FATAL);
            log.to_console();
            log.to_log_file();
            // create an object to safely exit the DirectorySignal class
            m_dirs = std::make_shared<std::vector<copyto>>();
        }
        
        if(m_dirs->empty()){
            no_watch = true;
        }

        STDOUT << "\n";
        for(const auto &dir:*m_dirs){
            STDOUT << App_MESSAGE("Monitoring directory: ") << dir.source << App_MESSAGE(" to: ") << dir.destination << "\n";
            
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
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";
    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught \n";
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

void application::DirectorySignal::monitor() noexcept{
    // no monitor directories set so exit the monitor function
    if(no_watch) return;

    std::jthread q_sys_thread([this]() {
        &application::queue_system<file_queue_info>::process, &m_queue_processor;
    });

    // Process notifications
    DWORD bytesTransferred;
    DS_resources* pMonitor;
    LPOVERLAPPED pOverlapped;
    
    timer t;
    std::atomic<bool> start_timer{false};
    std::condition_variable timer_thread_notify_cv;
    std::thread timer_thread(&timer::notify_timer,&t,30.0,&m_queue_processor.m_ready_to_process,&m_queue_processor.m_local_thread_cv,&start_timer,&timer_thread_notify_cv);

    while (GetQueuedCompletionStatus(m_hCompletionPort, &bytesTransferred, (PULONG_PTR)&pMonitor, &pOverlapped, INFINITE)) {
        Overflow(bytesTransferred);
        
        // Process change notification in pMonitor->buffer
        // Pointer to the first notification
        FILE_NOTIFY_INFORMATION* pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&pMonitor->m_buffer);
        ProcessDirectoryChanges(pNotify,pMonitor);

        UpdateWatcher(pMonitor);

        start_timer = true;
        timer_thread_notify_cv.notify_one();
    }



    t.end_notify_timer();
    if(timer_thread.joinable()){
        timer_thread_notify_cv.notify_one();
        timer_thread.join();
    }

    m_queue_processor.exit();
    if(q_sys_thread.joinable()){
        m_queue_processor.m_local_thread_cv.notify_one();
        q_sys_thread.join();
    }
}

bool application::DirectorySignal::Overflow(DWORD bytes_returned) noexcept
{
    if(bytes_returned == 0){
        STDOUT << App_MESSAGE("The monitoring buffer has overflowed");
        return true;
    }
    return false;
}

void application::DirectorySignal::UpdateWatcher(DS_resources* p_monitor) noexcept
{
    if(!ReadDirectoryChangesW(
            p_monitor->m_hDir, 
            &p_monitor->m_buffer, 
            sizeof(p_monitor->m_buffer), 
            sfct_api::recursive_flag_check(p_monitor->directory.commands), // watch subtree
            m_NotifyFilter,
            NULL, 
            &p_monitor->m_ol, 
            NULL)){
                try{
                    logger log(Error::WARNING);
                    log.to_console();
                    log.to_log_file();
                    log.to_output();
                }
                catch (const std::filesystem::filesystem_error& e) {
                    // Handle filesystem related errors
                    std::cerr << "Filesystem error: " << e.what() << "\n";
                }
                catch(const std::runtime_error& e){
                    // the error message
                    std::cerr << "Runtime error: " << e.what() << "\n";
                }
                catch(const std::bad_alloc& e){
                    // the error message
                    std::cerr << "Allocation error: " << e.what() << "\n";
                }
                catch (const std::exception& e) {
                    // Catch other standard exceptions
                    std::cerr << "Standard exception: " << e.what() << "\n";
                } catch (...) {
                    // Catch any other exceptions
                    std::cerr << "Unknown exception caught \n";
                }
            }
}

void application::DirectorySignal::ProcessDirectoryChanges(FILE_NOTIFY_INFORMATION *pNotify,DS_resources* pMonitor) noexcept
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
        auto gfs_src = sfct_api::get_file_status(entry.src);
        auto gfs_dst = sfct_api::get_file_status(entry.dst);

        // if gfs_src or gfs_dst is std::nullopt then entry.fs_src and entry.fs_dst will be std::filesystem::file_type::none
        // The none value is used for default-constructed file_status objects or when the file type has not been determined, 
        // whereas not_found indicates that a path does not exist or cannot be accessed.

        if(gfs_src.has_value()){
            entry.fs_src = gfs_src.value();
        }
        
        if(gfs_dst.has_value()){
            entry.fs_dst = gfs_dst.value();
        }
        
        entry.co = pMonitor->directory.co;
        entry.commands = pMonitor->directory.commands;
        entry.main_dst = pMonitor->directory.destination;
        entry.main_src = pMonitor->directory.source;

        
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
                if((pMonitor->directory.commands & cs::sync) != cs::none){
                    entry.fqs = file_queue_status::file_removed;
                }
                else{
                    entry.fqs = file_queue_status::none;
                }
                break;
            }
            case FILE_ACTION_RENAMED_OLD_NAME:
                entry.fqs = file_queue_status::rename_old;
                break;
            case FILE_ACTION_RENAMED_NEW_NAME:
                entry.fqs = file_queue_status::rename_new;
                break;
            default:
                // do nothing
                break;
        }

        // add the entry to queue system
        m_queue_processor.add_to_queue(entry);

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
#include "w32directorysignal.hpp"

application::DirectorySignal::DirectorySignal(std::shared_ptr<std::vector<copyto>> dirs_to_watch) noexcept
:m_dirs(dirs_to_watch){

    try{
        if(!dirs_to_watch){
            // log that dirs_to_watch is nullptr
            logger log(App_MESSAGE("nullptr"),Error::FATAL);
            log.to_console();
            log.to_log_file();
            
            // create an object to safely exit the DirectorySignal class
            // this is necessary else the whole program would crash when it reaches
            // for(const auto &dir:*m_dirs) this for loop
            m_dirs = std::make_shared<std::vector<copyto>>();
        }
        
        // if m_dirs is empty no_watch is set to true
        // this causes monitor function to return and not monitor
        if(m_dirs->empty()){
            no_watch = true;
        }

        // add a newline before displaying the directories being monitored
        STDOUT << "\n";

        // loop through m_dirs vector and create the neccessary resources to monitor each dir (directory)
        for(const auto &dir:*m_dirs){
            // indicate which directories are being monitored by sending that info to the console
            STDOUT << App_MESSAGE("Monitoring directory: ") << dir.source << App_MESSAGE(" to: ") << dir.destination << "\n";
            
            // create the handle to the monitored directory
            HANDLE hDir = CreateFile(
                dir.source.c_str(), FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL, OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                NULL);

            // if it fails to create a handle log the windows error
            if (hDir == INVALID_HANDLE_VALUE) {
                logger log(Error::WARNING);
                log.to_console();
                log.to_log_file();
                log.to_output();

                // then try the next dir in the vector of copyto objects
                continue;
            }

            // heap allocate a useful pointer that tells which directory has notifications pending
            // when a file entry is added, changed, moved or renamed
            DS_resources* monitor = new DS_resources{hDir, {}, {}, {{dir.source},{dir.destination},{dir.commands},{dir.co}}};
            
            // link the handle to the monitored directory to the monitor pointer
            // so when there is a notification pending the right monitor pointer is returned
            // if CreateIoCompletionPort returns NULL log the error
            if(!CreateIoCompletionPort(hDir, m_hCompletionPort, (ULONG_PTR)monitor, 0)){
                logger log(Error::WARNING);
                log.to_console();           
                log.to_log_file();
                log.to_output();
                
                // clean up heap allocated DS_resource since the CompletionPort failed to be created
                delete monitor;

                // close the handle to the monitored directory
                CloseHandle(hDir);

                // try the next dir(copyto object directory)
                continue;
            }

            // calls ReadDirectoryChanges() to begin monitoring the directory
            UpdateWatcher(monitor);

            // keep track of the monitor pointers
            // these are cleaned up in the destructor and handles are all closed
            m_pMonitors.push_back(monitor);
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";

        // if an exception is thrown in the constructor
        // we do not want to attempt to call the monitor() function
        // as it would be undefined behaviour since the DirectorySignal object would be
        // malformed so to speak
        no_watch = true;
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";

        // if an exception is thrown in the constructor
        // we do not want to attempt to call the monitor() function
        // as it would be undefined behaviour since the DirectorySignal object would be
        // malformed so to speak
        no_watch = true;
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";

        // if an exception is thrown in the constructor
        // we do not want to attempt to call the monitor() function
        // as it would be undefined behaviour since the DirectorySignal object would be
        // malformed so to speak
        no_watch = true;
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";

        // if an exception is thrown in the constructor
        // we do not want to attempt to call the monitor() function
        // as it would be undefined behaviour since the DirectorySignal object would be
        // malformed so to speak
        no_watch = true;

    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught \n";

        // if an exception is thrown in the constructor
        // we do not want to attempt to call the monitor() function
        // as it would be undefined behaviour since the DirectorySignal object would be
        // malformed so to speak
        no_watch = true;
    }

}

application::DirectorySignal::~DirectorySignal() noexcept{
    // Cleanup
    for (auto monitor : m_pMonitors) {
        if(monitor){
            CloseHandle(monitor->m_hDir);
            delete monitor;
        }
    }
    CloseHandle(m_hCompletionPort);
}

void application::DirectorySignal::monitor() noexcept{
    // no monitor directories set so exit the monitor function
    if(no_watch) return;

    // create a jthread that runs process() of queue_system class object
    // this is where the notifications are processed where the actual copying, moving, renaming ,or deletion of file entries takes places
    std::jthread q_sys_thread(&application::queue_system<file_queue_info>::process, &m_queue_processor);

    // Process notifications
    DWORD bytesTransferred;         // data notifications in bytes returned
    DS_resources* pMonitor;         // monitor pointer with the directories that have notifications waiting to be processed
    LPOVERLAPPED pOverlapped;       // used by GetQueuedCompletionStatus() to keep track of asynchronous operations
    
    // Delay the processing using a timer
    timer t;                                            // timer class object which has useful functions for timing
    std::atomic<bool> start_timer{false};               // signals ability to start the timer
    std::condition_variable timer_thread_notify_cv;     // notifies the thread to start the timer
    
    // create a jthread which is an auto joining thread which when out of scope will cause the thread to join
    // using the m_queue_processor.m_ready_to_process and m_queue_processor.m_local_thread_cv to signal the waiting thread in the queue_system class to begin processing entries
    // 
    std::jthread timer_thread(&timer::notify_timer,&t,30.0,&m_queue_processor.m_ready_to_process,&m_queue_processor.m_local_thread_cv,&start_timer,&timer_thread_notify_cv);

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
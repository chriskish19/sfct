#pragma once
#include "w32cpplib.hpp"
#include "w32obj.hpp"
#include "w32sfct_api.hpp"
#include "w32tm.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This header defines the template class queue_system.                                                             //
// queue_system<file_queue_info> is used in the DirectorySignal class to process all its notifications.             //
// The process() function is designed to be run on a dedicated thread. While the add_to_queue() function            //
// is called from different thread, it adds data to the queue. The queue is then processed in the function          //
// process(). The template with the typename data_t is meant to be a skeleton for template specialization types.    //
// The template class with the type file_queue_info is the class type used in the DirectorySignal class.            //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace application{
    /**
     * @brief A thread-safe queue system for managing and processing tasks in a multi-threaded environment.
     *
     * This class encapsulates a system for queuing and processing tasks of a generic type data_t in a multi-threaded context.
     * It provides mechanisms for adding tasks to a queue, processing these tasks in a dedicated thread, and safely managing
     * the lifecycle of task processing including synchronization and controlled shutdown. The class explicitly deletes copy
     * and move constructors and assignment operators to ensure each instance uniquely manages its resources and thread safety
     * considerations. It's designed to be flexible for various types of task processing by specializing the process_entry
     * function for the specific needs of the task type data_t.
     */
    template<typename data_t>
    class queue_system{
    public:
        /// @brief default constructor
        queue_system()= default;

        /// @brief default destructor
        ~queue_system()= default;
        
        /// @brief Copy constructor
        queue_system(const queue_system&) = delete;

        /// @brief Copy assignment operator
        queue_system& operator=(const queue_system&) = delete;

        /// @brief Move constructor
        queue_system(queue_system&&) = delete;

        /// @brief Move assignment operator
        queue_system& operator=(queue_system&&) = delete;



        /**
         * @brief Manages task processing in a multi-threaded environment.
         *
         * This function runs in a dedicated thread and continuously processes tasks from a queue.
         * It operates in two main modes: actively processing tasks when available and waiting for new tasks
         * when the queue is empty. The function checks for tasks in the primary queue and processes them
         * sequentially. If the primary queue is empty, it waits for a signal that new tasks are ready,
         * ensuring efficient CPU usage. Additionally, it handles a secondary queue of tasks that are
         * pending but not immediately ready for processing, swapping them into the primary queue when they
         * become ready. The loop runs until the `m_running` flag is set to false, allowing for a controlled
         * shutdown of the processing thread.
         */
        void process(){
            // Continuously run this loop until m_running is set to false
            while(m_running.load()){

                // Check if there are tasks ready to be processed
                if(m_ready_to_process.load()){

                    // Process all tasks in the primary queue
                    while(!m_queue.empty()){

                        // Retrieve the front task
                        data_t entry = m_queue.front();

                        // Process the retrieved task
                        process_entry(entry);

                        // Remove the processed task from the queue
                        m_queue.pop();
                    }
                    // Reset the flag indicating tasks have been be processed
                    m_ready_to_process = false;

                    // Notify the main thread that processing is complete
                    m_main_thread_cv.notify_one();
                }
                else{
                    
                    // Handle tasks that are waiting to be processed

                    // Check if there are tasks in the still waiting queue
                    if(!m_still_wait_data.empty()){

                        // Swap the still waiting tasks to the wait queue for processing
                        m_wait_data.swap(m_still_wait_data);

                        // Process all tasks in the wait queue
                        while(!m_wait_data.empty()){
                            // Retrieve the front task from the wait queue
                            data_t entry = m_wait_data.front();

                            // Process the retrieved task
                            process_entry(entry);

                            // Remove the processed task from the wait queue
                            m_wait_data.pop();
                        }
                    }
                    
                    // Synchronization block
                    {   
                        // Lock the thread until new tasks are ready for processing
                        std::unique_lock<std::mutex> local_lock(m_local_thread_guard);

                        // Wait for the condition that tasks are ready to be processed
                        m_local_thread_cv.wait(local_lock, [this] {return m_ready_to_process.load();});
                    }
                    
                    // Safely swap the task buffers to make new tasks available for processing
                    std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);
                    m_queue.swap(m_queue_buffer);
                }
            }
        }

        /**
         * @brief Adds a new task to the queue buffer.
         *
         * This function safely adds a new task to the queue buffer that is intended to be processed by worker threads.
         * It uses a mutex to ensure thread-safe access to the queue buffer, preventing data races when multiple threads
         * attempt to add tasks to the queue simultaneously.
         *
         * @param entry The task to be added to the queue. It's passed as a const reference to avoid unnecessary copying,
         *              assuming `data_t` might be a complex type or large object.
         */
        void add_to_queue(const data_t& entry){
            // Lock the mutex using a lock_guard which provides exception-safe locking.
            // The lock is automatically released when the lock_guard goes out of scope (i.e., at the end of this function).
            std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);

            // Safely add the new entry to the queue buffer.
            // emplace is used instead of push to construct the object in-place,
            m_queue_buffer.emplace(entry);
        }

        /**
         * @brief Gracefully terminates the processing thread(s).
         *
         * This function is designed to ensure a safe and orderly shutdown of the processing thread(s).
         * It first waits for any currently processing tasks to complete, ensuring that the system
         * is not in the middle of a task processing cycle. Once it's safe to proceed, it sets a flag
         * to stop the running loop in the processing thread(s), effectively signaling them to exit.
         */
        void exit(){
            // Acquire a lock on the main thread mutex to safely check and wait on the condition variable.
            // This lock ensures exclusive access to shared resources that the condition variable might depend on.
            std::unique_lock<std::mutex> local_lock(m_main_thread_guard);

            // Wait for the condition that no tasks are currently being processed.
            // This is important to ensure that we do not exit while tasks are still being processed.
            // The lambda function here checks the condition, and the wait call will block until the condition is true.
            m_main_thread_cv.wait(local_lock, [this] {return !m_ready_to_process.load(); });

            // Once it is safe (no tasks are being processed), set the running flag to false.
            // This will cause the processing loop(s) to exit on their next iteration.
            m_running = false;
        }

        // used to notify the waiting thread
        std::condition_variable m_local_thread_cv;

        // boolean that represents the ability to start processing
        std::atomic<bool> m_ready_to_process{false};

        // keeps process() running
        std::atomic<bool> m_running{true};
    private:
        // data ready to be processed
        std::queue<data_t> m_queue; 

        // A temporary buffer holding tasks before they are moved to the main queue for processing.
        std::queue<data_t> m_queue_buffer;

        // data to be proccessed later
        std::queue<data_t> m_wait_data;

        // data that still has to be processed later
        std::queue<data_t> m_still_wait_data;

        // Mutex for protecting access to the queue buffer, ensuring thread-safe interactions.
        std::mutex m_queue_buffer_mtx;

        // Mutex used by the local processing thread for synchronization.
        std::mutex m_local_thread_guard;
		
        // Mutex used by the main thread to synchronize with the processing thread.
        std::mutex m_main_thread_guard;

        // Condition variable used to signal the main thread from the processing thread.
		std::condition_variable m_main_thread_cv;

        /**
         * @brief Processes a single task.
         * This function is a placeholder meant to be overridden or specialized to handle task processing.
         * It defines how individual tasks of type data_t should be processed.
         * @param entry The task to be processed.
         */
        void process_entry(const data_t& entry){
            // Implementation should be provided based on the specific processing needs of data_t.
        }
    };


    template<>
    class queue_system<file_queue_info>{
    public:
        /// @brief default constructor
        queue_system() = default;

        /// @brief default destructor
        ~queue_system()= default;
        
        /// @brief delete the Copy constructor
        queue_system(const queue_system&) = delete;

        /// @brief delete Copy assignment operator
        queue_system& operator=(const queue_system&) = delete;

        /// @brief delete Move constructor
        queue_system(queue_system&&) = delete;

        /// @brief delete Move assignment operator
        queue_system& operator=(queue_system&&) = delete;


        /// @brief This function is designed to be run on a dedicated thread.
        /// Any code that may throw is handled within a try catch block.
        /// This function processes file_queue_info objects which are defined in the header file obj.hpp. 
        /// As an example ill explain its use in the DirectorySignal class:
        /// The DirectorySignal class has an instance of queue_system and it runs the process() function on a dedicated thread.
        /// This thread is called q_sys_thread. Its a jthread object. 
        /// The main thread runs the monitor() function which has a while loop that processes notifications from the windows api functions that are used to monitor directories. 
        /// That loop is where file_queue_info objects get created and the function add_to_queue() is called. 
        /// Once all the current notifications are processed, a timer begins and when it completes it sets m_ready_to_process to true.
        /// It uses m_local_thread_cv to notify q_sys_thread in the process() function to wake up and check the atomic boolean m_ready_to_process.
        /// q_sys_thread now moves on to swapping m_queue_buffer with m_queue and continues through the rest of the code in the loop.
        /// This process continues indefinitely until the member function exit() is called by the main thread in DirectorySignal classes monitor() function.
        void process() noexcept{
            // m_running can be modified by another thread and changed to false.
            // In the exit() function it is set to false when called.
            while(m_running.load()){
                
                // if queue has been filled with data then this atomic boolean will be true
                // it is meant to be set by a thread elsewhere. In this project it is set in 
                // the DirectorySignal class monitor() function by a timer thread.
                if(m_ready_to_process.load()){

                    // process all the entries in the queue until it returns empty
                    while(!m_queue.empty()){

                        // get the top entry in the queue
                        file_queue_info entry = m_queue.front();
                        
                        // if this throws we just skip the entry
                        // and send info to the console about the error
                        // that occurred, only under exceptional curcumstances would this throw
                        // but since I want a very robust program I want handle even the rarest of exceptions.
                        // I think it would throw only if the system memory was exhausted.
                        try{
                            m_all_seen_entries.insert(entry);
                        }
                        catch(const std::runtime_error& e){
                            std::cerr << "Runtime error: " << e.what() << "\n";
                        }
                        catch(const std::bad_alloc& e){
                            std::cerr << "Allocation error: " << e.what() << "\n";
                        }
                        catch (const std::exception& e) {
                            std::cerr << "Standard exception: " << e.what() << "\n";
                        } 
                        catch (...) {
                            std::cerr << "Unknown exception caught \n";
                        }

                        // process the entry 
                        process_entry(entry);

                        // remove top entry
                        m_queue.pop();
                    }

                    // the queue has been processed now set m_ready_to_process to false
                    // this signifies that the queue has been processed
                    m_ready_to_process = false;

                    // this is to notify the waiting thread in the exit() function that its safe to exit
                    // this may or may not do anything as there must be a waiting thread in the exit() function to receive 
                    // the notify signal
                    m_main_thread_cv.notify_one();
                }
                else{
                    
                    // this check is necessary as we only want to swap if there is data on the m_still_wait_data queue
                    if(!m_still_wait_data.empty()){

                        // m_still_wait_data is exchanged with m_wait_data
                        m_wait_data.swap(m_still_wait_data);

                        // attempt to process m_wait_data which are entries that were unable to be processed initially
                        // could be files that were in use or being actively transfered
                        while(!m_wait_data.empty()){

                            // get the top entry
                            file_queue_info entry = m_wait_data.front();

                            // try to process the entry again, if it fails to be processed agian it will end up in m_still_wait_data queue
                            // and the process will repeat aslong as more data is added to m_queue_buffer in future.
                            // Since the thread in this loop will only get woken up by a notify through m_local_thread_cv some entries may sit 
                            // in m_still_wait_data queue indefinitely. 
                            // If the program is closed then those entries are lost and do not get processed.
                            process_entry(entry);

                            // remove the top entry
                            m_wait_data.pop();
                        }
                    }
                    
                    // double check for missed entries if the files were moved(or cut and pasted in windows explorer within same root directory)
                    // this will get called on first run of the process() function but it wont matter since m_new_main_directory_entries will be empty
                    // so the for range loop will be skipped and then the m_new_main_directory_entries will get cleared. The function exits without doing
                    // anything.
                    check();
                    
                    // On first run of the process() function the thread will end up here waiting for a notification.
                    // we attempt to cause the thread to wait, if that throws we exit the process() function.
                    // queue system will not work correctly if the thread fails to wait so we must exit.
                    try{
                        std::unique_lock<std::mutex> local_lock(m_local_thread_guard);
                        m_local_thread_cv.wait(local_lock, [this] {return m_ready_to_process.load();});
                    }
                    catch(const std::runtime_error& e){
                        std::cerr << "Runtime error: " << e.what() << "\n";

                        // end the while loop in the function process()
                        m_running = false;
                    }
                    catch(const std::bad_alloc& e){
                        std::cerr << "Allocation error: " << e.what() << "\n";

                        // end the while loop in the function process()
                        m_running = false;
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Standard exception: " << e.what() << "\n";

                        // end the while loop in the function process()
                        m_running = false;
                    } 
                    catch (...) {
                        std::cerr << "Unknown exception caught \n";

                        // end the while loop in the function process()
                        m_running = false;
                    }

                    // we are about to swap m_queue with m_queue_buffer.
                    // in order to prevent memory errors we need to use a mutex and lock it
                    // m_queue_buffer_mtx is used in the function add_to_queue() which is run on a different
                    // thread so in this way m_queue_buffer is protected
                    std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);

                    // exchange the data in m_queue_buffer with m_queue
                    // m_queue is initialized as empty so m_queue_buffer will be empty
                    // after the swap function()
                    m_queue.swap(m_queue_buffer);
                }
            }
        }

        /// @brief this function adds a file_queue_info entry to the member variable m_queue_buffer which is an object of 
        /// the stl <queue> class. This function is designed to be called from another thread. So thread A adds file_queue_info entries
        /// to the m_queue_buffer while thread B runs process().
        /// @param entry data for copying, renaming or deleting file entries
        void add_to_queue(const file_queue_info& entry) noexcept{
            // this code is unlikely to throw an exception only under exceptional circumstances
            // but for robustness its wrapped in a try catch block
            // if an exception is thrown it skips the entry and the function ends
            try{
                // prevent concurrent access to m_queue_buffer which would cause
                // memory errors otherwise, since m_queue_buffer is accessed by multiple threads
                // a lock guard is required
                std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);
                
                // add the entry to the queue buffer
                m_queue_buffer.emplace(entry);
            }
            catch(const std::runtime_error& e){
                std::cerr << "Runtime error :" << e.what() << "\n";
            }
            catch(const std::bad_alloc& e){
                std::cerr << "Allocation error: " << e.what() << "\n";
            }
            catch (const std::exception& e) {
                std::cerr << "Standard exception: " << e.what() << "\n";
            } 
            catch (...) {
                std::cerr << "Unknown exception caught \n";
            }
        }

        /// @brief causes the thread that called this function to wait until the remaining buffer is processed
        /// then sets m_running to false which ends the process() function
        void exit() noexcept{
            // this code is unlikely to throw an exception only under exceptional circumstances
            // but for robustness its wrapped in a try catch block
            try{
                // wait to process remaining buffer in process() function
                std::unique_lock<std::mutex> local_lock(m_main_thread_guard);
                m_main_thread_cv.wait(local_lock, [this] {return !m_ready_to_process.load(); });

                // ends the while loop in the function process()
                m_running = false;
            }
            catch(const std::runtime_error& e){
                std::cerr << "Runtime error :" << e.what() << "\n";
                m_running = false;
            }
            catch(const std::bad_alloc& e){
                std::cerr << "Allocation error: " << e.what() << "\n";
                m_running = false;
            }
            catch (const std::exception& e) {
                std::cerr << "Standard exception: " << e.what() << "\n";
                m_running = false;
            } 
            catch (...) {
                std::cerr << "Unknown exception caught \n";
                m_running = false;
            }
        }


        // used to notify the waiting thread in the process() function
        std::condition_variable m_local_thread_cv;

        // signifies that m_queue is ready to be processed
        std::atomic<bool> m_ready_to_process{false};

        // used to keep the process() function operating
        std::atomic<bool> m_running{true};
    private:
        // data that is ready to be processed
        std::queue<file_queue_info> m_queue; 

        // buffer for m_queue data
        std::queue<file_queue_info> m_queue_buffer;

        // data to be proccessed later
        std::queue<file_queue_info> m_wait_data;

        // data that still has to be processed later
        std::queue<file_queue_info> m_still_wait_data;

        // protects m_queue_buffer from multiple threads access
        std::mutex m_queue_buffer_mtx;

        // used with m_local_thread_cv to lock and wait
        std::mutex m_local_thread_guard;
		
        // used in the exit() function to lock and wait for process() function to finish processing its queue
        std::mutex m_main_thread_guard;

        // used in the exit() function to cause the main thread(or which ever thread called exit() could be a dedicated thread)
        // to wait for process() function to finish processing the queue.
        // it is notified by the process() function thread when the queue has been processed and once this occurs
        // the gate opens and the waiting thread wakes up and sets m_running to false which causes the process() function to end.
		std::condition_variable m_main_thread_cv;

        // new directories added to the queue which are 1 level sub-directories of entry.main_src
        // this is needed because DirectorySignal doesnt produce notifications for all the sub-directories if 
        // they are moved into the monitored directory(cut and paste in windows explorer). 
        std::vector<file_queue_info> m_new_main_directory_entries;

        // all file_queue_info entries processed by process_entry() function
        // this is needed to avoid unnecessarily reprocessing the same entry twice when we call the check() function.
        std::unordered_set<file_queue_info> m_all_seen_entries;
        
        // all entry.main_src directory entries (1 level sub-directories of entry.main_src)
        std::unordered_set<file_queue_info> m_all_seen_main_directory_entries;

        /// @brief checks for missed entries.
        /// This is needed because the class DirectorySignal in the monitor() function will only produce notifications/entries if the files
        /// were copied into the monitored directory. If the file entries were moved into the monitored directory and the root directory is the same only the
        /// child directory entries and files will get entries created and added to m_queue_buffer. Any sub directories and files within them will get ignored.
        /// So the check() function will recursively iterate through the sub directories that were missed and produce new entries to be processed.
        void check() noexcept{
            // only two functions might throw, std::filesystem::recursive_directory_iterator(entry->src) and m_all_seen_entries.insert(_file_info).second 
            // m_all_seen_entries.insert(_file_info).second is unlikely to throw only on exceptional circumstances probably on system memory exhaustion
            // std::filesystem::recursive_directory_iterator(entry->src) will throw if it cant access the directory, it doesnt exist or its not a valid directory.
            // if an exception is thrown we log it to the console and exit the check() function
            try{
                // exit the check early
                // currently not used
                bool exit__{false};

                // m_new_main_directory_entries is a vector of file_queue_info objects
                // It contains newly added directories in the monitored directory(main_src) 
                for(auto entry{m_new_main_directory_entries.begin()};entry != m_new_main_directory_entries.end() && !exit__;entry++){
                    // this is is needed since the user could have removed a directory entry
                    // std::filesystem::recursive_directory_iterator would throw if entry->src didnt exist
                    if(sfct_api::exists(entry->src)){
                        
                        // iterate through the directory recursively
                        // On windows I think std::filesystem will lock the directory so the user cannot delete or modify it 
                        // once the recursive_directory_iterator is initialized. I havent found any documentation for this, only 
                        // in testing I tried to delete a directory that was being copied. It was being copied using sfct copy command. Otherwise if the
                        // user was able to delete the directory while it was being traversed by std::filesystem::recursive_directory_iterator
                        // it would throw an exception or fail in some way.
                        for(const auto& rdi_entry:std::filesystem::recursive_directory_iterator(entry->src)){
                            // Create the new destination path but dont create the needed directories.
                            // The directories will get created as the directory entries are processed in the process_entry() function.
                            // This relies on the behavior of std::filesystem::recursive_directory_iterator and the way it iterates
                            // through the directory tree, Depth-First Traversal. This means an entry for a directory is always generated before 
                            // entering the directory which is why we don't need to create the directories for each dst_path.
                            auto dst_path = sfct_api::create_file_relative_path(rdi_entry.path(),entry->dst,entry->src,false);
                            
                            // if it returns a nullopt we just skip the entry and try the next
                            if(dst_path.has_value()){
                                // create a file_queue_info entry
                                file_queue_info _file_info;
                                _file_info.src = rdi_entry.path();              // set the src path
                                _file_info.dst = dst_path.value();              // set the dst path

                                // the reason only src and dst are set in _file_info is, that is all that is needed to check
                                // if its in the m_all_seen_entries set. This is because the src and dst values are used 
                                // to produce the hash value for the unordered_set.


                                // true if not in the set
                                // false if in the set
                                if(m_all_seen_entries.insert(_file_info).second){
                                    // if _file_info is not in the set we fill the whole entry with data and process it
                                    _file_info.co = entry->co;                                          // std::filesystem copy options
                                    _file_info.commands = entry->commands;                              // sfct commands
                                    _file_info.fqs = file_queue_status::file_added;                     // set as a file_added
                                    // auto gfs_dst = sfct_api::get_file_status(dst_path.value());      // this is only used if a file is removed   
                                    auto gfs_src = sfct_api::get_file_status(rdi_entry.path());         // determines what type of file entry
                                    _file_info.main_dst = entry->main_dst;                              // upper most destination directory
                                    _file_info.main_src = entry->main_src;                              // upper most source directory

                                    // not needed
                                    //if(gfs_dst.has_value()){
                                    //    _file_info.fs_dst = gfs_dst.value();
                                    //}

                                    // if gfs_src was successfull set the value
                                    // if this fails the entry will get skipped
                                    if(gfs_src.has_value()){
                                        _file_info.fs_src = gfs_src.value();
                                    }
                                    
                                    // send the entry for processing
                                    process_entry(_file_info);
                                }
                                else{
                                    // lets not check everything
                                    // if one entry exists most likely they all do
                                    // but may change this in the future for more robust checking
                                    // exit__ = true;
                                }
                            }
                        }
                    }
                }

                // since we checked all the new main directories we can reset it
                m_new_main_directory_entries.clear(); 
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << "\n";
            }
            catch(const std::runtime_error& e){
                std::cerr << "Runtime error :" << e.what() << "\n";
            }
            catch(const std::bad_alloc& e){
                std::cerr << "Allocation error: " << e.what() << "\n";
            }
            catch (const std::exception& e) {
                std::cerr << "Standard exception: " << e.what() << "\n";
            } 
            catch (...) {
                std::cerr << "Unknown exception caught \n";
            }
        }

        // for renaming a path, this holds the old path name
        std::filesystem::path m_rename_old;
        
        /// @brief this function will check each entry and perform the correct function, creating directories,
        /// copying files, renaming a file, or removing files.
        /// @param entry file_queue_info object
        void process_entry(const file_queue_info& entry) noexcept{
            
            // send info to the console for each entry
            sfct_api::to_console(App_MESSAGE("Processing entry: "),entry.src);
            
            
            switch(entry.fqs){
                case file_queue_status::file_added:{
                    switch(entry.fs_src.type()){
                        case std::filesystem::file_type::none:
                            // skip for now
                            break;
                        case std::filesystem::file_type::not_found:
                            // skip for now
                            break;
                        case std::filesystem::file_type::regular:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::directory:{
                            // we must check if the src exists since the user could have removed it
                            // during the time before processing. If it doesnt exist we skip it.
                            if(sfct_api::exists(entry.src)){
                                sfct_api::create_directory_paths(entry.dst);
                                
                                // it is possible for this code to throw
                                // I think if the system can't allocate memory it will throw
                                // also if entry.src.parent_path() cannot obtain the parent path it will throw a 
                                // filesystem_error. In the case it throws which ever function doesnt matter we hit the break
                                // statement and process_entry() function ends.
                                try{
                                    // if entry.src is a directory and if its path is a parent of entry.main_src
                                    // Example:
                                    // path a = C:\test and path b = C:\test\mydir then path b's parent path is path a.
                                    // this is because we only want the directories in entry.main_src to be added to 
                                    // m_new_main_directory_entries. They only get added if 
                                    if(entry.src.parent_path() == entry.main_src && 
                                        // checks for recursive command in the entry
                                        // if the command "single" is set instead of recursive we don't need to check anything
                                        // and therefore the directory entry will not be added to m_new_main_directory_entries
                                        sfct_api::recursive_flag_check(entry.commands) && 

                                        // true if not in the set
                                        m_all_seen_main_directory_entries.insert(entry).second){
                                        
                                        // add the entry to the vector to be used in the check() function
                                        // which will check these directories for missed entries
                                        m_new_main_directory_entries.push_back(entry);
                                    }
                                }
                                catch (const std::filesystem::filesystem_error& e) {
                                    std::cerr << "Filesystem error: " << e.what() << "\n";
                                }
                                catch(const std::runtime_error& e){
                                    std::cerr << "Runtime error: " << e.what() << "\n";
                                }
                                catch(const std::bad_alloc& e){
                                    std::cerr << "Allocation error: " << e.what() << "\n";
                                }
                                catch (const std::exception& e) {
                                    std::cerr << "Standard exception: " << e.what() << "\n";
                                } 
                                catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::symlink:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::block:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::character:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::fifo:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::socket:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::unknown:
                            // do nothing
                            break;
                        default:
                            // do nothing
                            break;
                    }
                    break;
                }
                case file_queue_status::file_updated:{
                    switch(entry.fs_src.type()){
                        case std::filesystem::file_type::none:
                            // skip for now
                            break;
                        case std::filesystem::file_type::not_found:
                            // skip for now
                            break;
                        case std::filesystem::file_type::regular:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            break;
                        }
                        case std::filesystem::file_type::directory:
                            // do nothing
                            break;
                        case std::filesystem::file_type::symlink:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::block:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::character:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::fifo:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::socket:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                add_to_still_wait_data(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::unknown:
                            // do nothing
                            break;
                        default:
                            // do nothing
                            break;
                    }
                    break;
                }
                case file_queue_status::file_removed:{
                    switch(entry.fs_dst.type()){
                        case std::filesystem::file_type::none:
                            // skip for now
                            break;
                        case std::filesystem::file_type::not_found:
                            // skip for now
                            break;
                        case std::filesystem::file_type::regular:{
                            // check the status, if the status is success then erase the entry
                            if(sfct_api::remove_entry(entry.dst).s == application::remove_file_ext::remove_file_status::removal_success){
                                m_all_seen_entries.erase(entry);
                            }
                            break;
                        }
                        case std::filesystem::file_type::directory:{
                            // build a vector of entries of the entry.dst directory
                            // this is needed because sfct_api::remove_all() can fail 
                            // we only want to remove entries from m_all_seen_entries if the directory was sucessfully removed
                            std::vector<file_queue_info> temp_to_be_removed;
                            
                            // if an exception is thrown we need to skip code that is part of normal operation
                            // this allows parts of the code to be conditionally executed
                            bool exception_thrown{false};

                            // only applies if recursive flag is set and entry.dst exists and m_all_seen_entries has entries
                            if(sfct_api::recursive_flag_check(entry.commands) && sfct_api::exists(entry.dst) && !m_all_seen_entries.empty()){
                                // if an exception is thrown(any exception except std::filesystem error) m_all_seen_entries gets a reset.
                                // this is okay since erase() can be safely called on an unordered_set where it does not exist.
                                // So any future deletions will just be ignored since the m_all_seen_entries is empty.
                                // But as a side effect when check() is called in process() under these conditions:
                                // 1. if file entries were added and removed in the same buffer zone(the time between adding to the queue buffer and process() waking up
                                // to process the queue).
                                // 2. if recursive flag is set
                                // 3. the added file entries include directories in main_src and those directories have sub-directories with file entries
                                // Under these conditions then when process() calls check() double entries will get created that have previously been processed.
                                // This results in re-copying. Especially if the overwrite command is set.
                                try{
                                    for(const auto& rdi_entry_:std::filesystem::recursive_directory_iterator(entry.dst)){
                                        // we need to recreate the src path to produce the correct hash in order to erase it from
                                        // m_all_seen_entries
                                        auto recreated_src = sfct_api::create_file_relative_path(rdi_entry_.path(),entry.src,entry.dst,false);
                                        if(recreated_src.has_value()){
                                            file_queue_info _file_info;
                                            _file_info.src = recreated_src.value();
                                            _file_info.dst = rdi_entry_.path();
                                            temp_to_be_removed.push_back(_file_info);
                                        }
                                    }
                                }
                                catch(const std::filesystem::filesystem_error& e){
                                    // a filesystem error exception was just caught
                                    // this is probably caused by the user deleting files while 
                                    // entries are being processed in the try block or some other issue.

                                    // set exception_thrown boolean to true
                                    // this skips code for normal operation
                                    exception_thrown = true;

                                    // notify the user of the exception and what went wrong
                                    std::cerr << "Filesystem error: " << e.what() << "\n";
                                    

                                    // To handle this we will attempt to remove the directory by using
                                    // sfct_api::remove_all(). This function will handle any exceptions
                                    // and errors and returns a nice enum to use indicating what happened.
                                    remove_all_ext _rae = sfct_api::remove_all(entry.dst);


                                    // process the removal status
                                    switch(_rae.s){
                                        case remove_all_ext::remove_all_status::error_code_present:{
                                            // the error code will be logged and displayed in the console
                                            // by sfct_api::remove_all().

                                            // we want to make sure to erase entry only if entry.dst has been removed
                                            // in this case it most likely has not been removed because an error usually indicates
                                            // a problem with removal. entry.dst can only be removed if all its contents
                                            // (sub-directories and file entries) are completely deleted.
                                            if(!sfct_api::exists(entry.dst)){
                                                // we want to erase the main directory entry 
                                                // entry may or may not be present in m_all_seen_entries
                                                // this is okay since erase() function can handle it
                                                m_all_seen_entries.erase(entry);
                                            
                                                // entry may or may not be present
                                                m_all_seen_main_directory_entries.erase(entry);
                                            }
                                            

                                            // temp_to_be_removed may have some entries before the exception happened
                                            // lets remove them from m_all_seen_entries making sure to check that they have been removed.
                                            // If temp_to_be_removed is empty() this for loop will be skipped and nothing will happen
                                            // so no need to check if temp_to_be_removed has data in it
                                            for(const auto& temp_entry:temp_to_be_removed){
                                                if(!sfct_api::exists(temp_entry.dst)){
                                                    m_all_seen_entries.erase(temp_entry);
                                                }
                                            }

                                            // now we have the problem of m_all_seen_entries having entries that don't
                                            // exist, one solution is to check m_all_seen_entries and erase any entries that don't exist
                                            for(auto it = m_all_seen_entries.begin(); it != m_all_seen_entries.end(); /* no increment here */) {
                                                if(!sfct_api::exists(it->dst)) {
                                                    it = m_all_seen_entries.erase(it);
                                                } else {
                                                    ++it;
                                                }
                                            }


                                            // send info to the user indicating that an attempt was made to remove the directory along with 
                                            // the number of file entries that were removed
                                            sfct_api::to_console(App_MESSAGE("Attempted to remove directory but an error occurred "),entry.dst,_rae.files_removed);


                                            break;
                                        }
                                        case remove_all_ext::remove_all_status::exception_thrown:{
                                            // an exception was thrown and handled within sfct_api::remove_all()
                                            // the exception info will be displayed in the console by sfct_api::remove_all().

                                            // we want to make sure to erase entry only if entry.dst has been removed
                                            // in this case it most likely has not been removed because an exception usually indicates
                                            // a problem with removal. entry.dst can only be removed if all its contents
                                            // (sub-directories and file entries) are completely deleted.
                                            if(!sfct_api::exists(entry.dst)){
                                                // we want to erase the main directory entry 
                                                // entry may or may not be present in m_all_seen_entries
                                                // this is okay since erase() function can handle it
                                                m_all_seen_entries.erase(entry);
                                            
                                                // entry may or may not be present
                                                m_all_seen_main_directory_entries.erase(entry);
                                            }
                                            

                                            // temp_to_be_removed may have some entries before the exception happened(not the exception from sfct_api::remove_all())
                                            // lets remove them from m_all_seen_entries making sure to check that they have been removed.
                                            // If temp_to_be_removed is empty() this for loop will be skipped and nothing will happen
                                            // so no need to check if temp_to_be_removed has data in it
                                            for(const auto& temp_entry:temp_to_be_removed){
                                                if(!sfct_api::exists(temp_entry.dst)){
                                                    m_all_seen_entries.erase(temp_entry);
                                                }
                                            }

                                            // now we have the problem of m_all_seen_entries having entries that don't
                                            // exist, one solution is to check m_all_seen_entries and erase any entries that don't exist
                                            for(auto it = m_all_seen_entries.begin(); it != m_all_seen_entries.end(); /* no increment here */) {
                                                if(!sfct_api::exists(it->dst)) {
                                                    it = m_all_seen_entries.erase(it);
                                                } else {
                                                    ++it;
                                                }
                                            }


                                            // send info to the user indicating that an attempt was made to remove the directory along with 
                                            // the number of file entries that were removed
                                            sfct_api::to_console(App_MESSAGE("Attempted to remove directory but an exception occurred "),entry.dst,_rae.files_removed);


                                            break;
                                        }
                                        case remove_all_ext::remove_all_status::invalid_directory:{
                                            // directory was removed by the user and does not exist
                                            // or the entry is not a directory. The latter is only possible if
                                            // entry was not constructed properly. Lets assume sfct_api::get_file_status 
                                            // was called and entry is properly constructed, in that case it means the user removed
                                            // the directory while being processed which caused an exception to be thrown. 
                                            

                                            // This should not occur on windows as the directory becomes locked once a recursive_directory_iterator is
                                            // initialized. So this case statement should never happen, only in exceptional circumstances. 
                                            // There is the case where the file lock system on windows can be bypassed. For example
                                            // using the terminal to force delete files can cause the lock to be overridden. So if during iterating
                                            // the directory a user uses the terminal to force delete the directory its iterating through then an exception
                                            // would be thrown by the filesystem. Then sfct_api::remove_all() would return an invalid_directory since it does
                                            // not exist and the code execution flow would end up in this case statement.
                                            

                                            // we want to erase the main directory entry 
                                            // entry may or may not be present in m_all_seen_entries
                                            // this is okay since erase() function can handle it
                                            m_all_seen_entries.erase(entry);
                                            
                                            // entry may or may not be present
                                            m_all_seen_main_directory_entries.erase(entry);

                                            // temp_to_be_removed may have some entries before the exception happened
                                            // lets remove them from m_all_seen_entries
                                            for(const auto& temp_entry:temp_to_be_removed){
                                                m_all_seen_entries.erase(temp_entry);
                                            }

                                            // now we have the problem of m_all_seen_entries having entries that don't
                                            // exist, one solution is to check m_all_seen_entries and erase any entries that don't exist
                                            for(auto it = m_all_seen_entries.begin(); it != m_all_seen_entries.end(); /* no increment here */) {
                                                if(!sfct_api::exists(it->dst)) {
                                                    it = m_all_seen_entries.erase(it);
                                                } else {
                                                    ++it;
                                                }
                                            }

                                            break;
                                        }
                                        case remove_all_ext::remove_all_status::removal_success:{
                                            // we want to erase the main directory entry 
                                            // entry may or may not be present in m_all_seen_entries
                                            // this is okay since erase() function can handle it
                                            m_all_seen_entries.erase(entry);
                                            
                                            // entry may or may not be present
                                            m_all_seen_main_directory_entries.erase(entry);

                                            // erase any temporary entries before the exception occurred
                                            for(const auto& temp_entry:temp_to_be_removed){
                                                m_all_seen_entries.erase(temp_entry);
                                            }

                                            // now we have the problem of m_all_seen_entries having entries that don't
                                            // exist, one solution is to check m_all_seen_entries and erase any entries that don't exist
                                            for(auto it = m_all_seen_entries.begin(); it != m_all_seen_entries.end(); /* no increment here */) {
                                                if(!sfct_api::exists(it->dst)) {
                                                    it = m_all_seen_entries.erase(it);
                                                } else {
                                                    ++it;
                                                }
                                            }

                                            // send info to the user indicating the directory was removed along with the number of file entries
                                            sfct_api::to_console(App_MESSAGE("Directory removed "),entry.dst,_rae.files_removed);

                                            break;
                                        }
                                        default:
                                            // skip
                                            break;
                                    }
                                }
                                catch(const std::runtime_error& e){
                                    std::cerr << "Runtime error: " << e.what() << "\n";

                                    exception_thrown = true;
                                    m_all_seen_entries.clear();
                                }
                                catch(const std::bad_alloc& e){
                                    std::cerr << "Allocation error: " << e.what() << "\n";

                                    exception_thrown = true;
                                    m_all_seen_entries.clear();
                                }
                                catch(const std::exception& e){
                                    std::cerr << "Standard exception: " << e.what() << "\n";

                                    exception_thrown = true;
                                    m_all_seen_entries.clear();
                                } 
                                catch(...){
                                    std::cerr << "Unknown exception caught \n";

                                    exception_thrown = true;
                                    m_all_seen_entries.clear();
                                }
                            }
                            

                            // normal execution block
                            if(!exception_thrown){
                                
                                remove_all_ext _rae = sfct_api::remove_all(entry.dst);
                            
                                switch(_rae.s){
                                    case remove_all_ext::remove_all_status::error_code_present:{
                                        // the error code will be logged and displayed in the console
                                        // by sfct_api::remove_all().

                                        // we want to make sure to erase entry only if entry.dst has been removed
                                        // in this case it most likely has not been removed because an error usually indicates
                                        // a problem with removal. entry.dst can only be removed if all its contents
                                        // (sub-directories and file entries) are completely deleted.
                                        if(!sfct_api::exists(entry.dst)){
                                            // we want to erase the main directory entry 
                                            // entry may or may not be present in m_all_seen_entries
                                            // this is okay since erase() function can handle it
                                            m_all_seen_entries.erase(entry);
                                        
                                            // entry may or may not be present
                                            m_all_seen_main_directory_entries.erase(entry);
                                        }
                                        

                                        // since we do not know which files have been removed because the operation
                                        // errored. We must check and erase only if they no longer exist.
                                        for(const auto& temp_entry:temp_to_be_removed){
                                            if(!sfct_api::exists(temp_entry.dst)){
                                                m_all_seen_entries.erase(temp_entry);
                                            }
                                        }

                                        // send info to the user indicating that an attempt was made to remove the directory along with 
                                        // the number of file entries that were removed
                                        sfct_api::to_console(App_MESSAGE("Attempted to remove directory but an error occurred "),entry.dst,_rae.files_removed);

                                        break;
                                    } 
                                    case remove_all_ext::remove_all_status::exception_thrown:{
                                        // an exception was thrown and handled within sfct_api::remove_all()
                                        // the exception info will be displayed in the console by sfct_api::remove_all().
                                        
                                        // we want to make sure to erase entry only if entry.dst has been removed
                                        // in this case it most likely has not been removed because an exception usually indicates
                                        // a problem with removal. entry.dst can only be removed if all its contents
                                        // (sub-directories and file entries) are completely deleted.
                                        if(!sfct_api::exists(entry.dst)){
                                            // we want to erase the main directory entry 
                                            // entry may or may not be present in m_all_seen_entries
                                            // this is okay since erase() function can handle it
                                            m_all_seen_entries.erase(entry);
                                        
                                            // entry may or may not be present
                                            m_all_seen_main_directory_entries.erase(entry);
                                        }
                                        
                                        // since we do not know which files have been removed because the operation
                                        // threw an exception. We must check and erase only if they no longer exist.
                                        for(const auto& temp_entry:temp_to_be_removed){
                                            if(!sfct_api::exists(temp_entry.dst)){
                                                m_all_seen_entries.erase(temp_entry);
                                            }
                                        }

                                        // send info to the user indicating that an attempt was made to remove the directory along with 
                                        // the number of file entries that were removed
                                        sfct_api::to_console(App_MESSAGE("Attempted to remove directory but an exception occurred "),entry.dst,_rae.files_removed);

                                        break;
                                    }
                                    case remove_all_ext::remove_all_status::invalid_directory:{
                                        // directory was removed by the user and does not exist
                                        // or the entry is not a directory. The latter is only possible if
                                        // entry was not constructed properly. Lets assume sfct_api::get_file_status 
                                        // was called and entry is properly constructed, in that case it means the user removed
                                        // the directory prior to processing. 

                                        // we want to erase the main directory entry 
                                        // entry may or may not be present in m_all_seen_entries
                                        // this is okay since erase() function can handle it
                                        m_all_seen_entries.erase(entry);
                                        
                                        // entry may or may not be present
                                        m_all_seen_main_directory_entries.erase(entry);

                                        // now we have the problem of m_all_seen_entries having entries that don't
                                        // exist, one solution is to check m_all_seen_entries and erase any entries that don't exist
                                        for(auto it = m_all_seen_entries.begin(); it != m_all_seen_entries.end(); /* no increment here */) {
                                            if(!sfct_api::exists(it->dst)) {
                                                it = m_all_seen_entries.erase(it);
                                            } else {
                                                ++it;
                                            }
                                        }

                                        break;
                                    }
                                    case remove_all_ext::remove_all_status::removal_success:{
                                        // everything went according to plan
                                        
                                        // we want to erase the main directory entry 
                                        // entry may or may not be present in m_all_seen_entries
                                        // this is okay since erase() function can handle it
                                        m_all_seen_entries.erase(entry);
                                        
                                        // entry may or may not be present
                                        m_all_seen_main_directory_entries.erase(entry);

                                        // erase the entries
                                        for(const auto& temp_entry:temp_to_be_removed){
                                            m_all_seen_entries.erase(temp_entry);
                                        }

                                        // send info to the user indicating the directory was removed along with the number of file entries
                                        sfct_api::to_console(App_MESSAGE("Directory removed "),entry.dst,_rae.files_removed);

                                        break;
                                    }
                                    default:
                                        // skip
                                        break;
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::symlink:{
                            if(sfct_api::remove_entry(entry.dst).s == application::remove_file_ext::remove_file_status::removal_success){
                                m_all_seen_entries.erase(entry);
                            }
                            break;
                        }
                        case std::filesystem::file_type::block:{
                            if(sfct_api::remove_entry(entry.dst).s == application::remove_file_ext::remove_file_status::removal_success){
                                m_all_seen_entries.erase(entry);
                            }
                            break;
                        }
                        case std::filesystem::file_type::character:{
                            if(sfct_api::remove_entry(entry.dst).s == application::remove_file_ext::remove_file_status::removal_success){
                                m_all_seen_entries.erase(entry);
                            }
                            break;
                        }
                        case std::filesystem::file_type::fifo:{
                            if(sfct_api::remove_entry(entry.dst).s == application::remove_file_ext::remove_file_status::removal_success){
                                m_all_seen_entries.erase(entry);
                            }
                            break;
                        }
                        case std::filesystem::file_type::socket:{
                            if(sfct_api::remove_entry(entry.dst).s == application::remove_file_ext::remove_file_status::removal_success){
                                m_all_seen_entries.erase(entry);
                            }
                            break;
                        }
                        case std::filesystem::file_type::unknown:
                            // do nothing
                            break;
                        default:
                            // do nothing
                            break;
                    }
                    break;
                }
                case file_queue_status::rename_old:{
                    
                    // this is dependant on the DirectorySignal class and the windows api to process
                    // rename notfications with the old name first and then the new name 
                    // as the next notification. Anything different and the rename operations
                    // will fail.
                    try{
                        // if this fails not much can be done
                        // other than skip it but I think the whole program
                        // is doomed if a copy operation throws an exception
                        m_rename_old = entry.dst;
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        std::cerr << "Filesystem error: " << e.what() << "\n";
                    }
                    catch(const std::runtime_error& e){
                        std::cerr << "Runtime error: " << e.what() << "\n";
                    }
                    catch(const std::bad_alloc& e){
                        std::cerr << "Allocation error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Standard exception: " << e.what() << "\n";
                    } 
                    catch (...) {
                        std::cerr << "Unknown exception caught \n";
                    }
                    break;
                }
                case file_queue_status::rename_new:{
                    
                    // a rename operation changes the entry path which causes a newly renamed added file within the same buffer zone
                    // (the time before processing begins when entries are added to the queue buffer) to skip the file_added entry. 
                    // This causes the problem of the newly added file entry getting processed before the rename operation entry and at this
                    // point the file has a different name so its considered non-existent on the file system and is skipped. 
                    // To mitigate this we check if the entry.dst exists if it does not exist we create a file_added entry and recursively process it. 
                    // Then after that, the rename operation can be called and successfully rename the file. 

                    if(!sfct_api::exists(entry.dst)){
                        
                        file_queue_info _file_info;
                        
                        try{
                            // if an exception is thrown by these operations
                            // nothing can be done, except process the empty entry
                            _file_info = entry;
                            _file_info.fqs = file_queue_status::file_added;
                        }
                        catch (const std::filesystem::filesystem_error& e) {
                            std::cerr << "Filesystem error: " << e.what() << "\n";
                        }
                        catch(const std::runtime_error& e){
                            std::cerr << "Runtime error: " << e.what() << "\n";
                        }
                        catch(const std::bad_alloc& e){
                            std::cerr << "Allocation error: " << e.what() << "\n";
                        }
                        catch (const std::exception& e) {
                            std::cerr << "Standard exception: " << e.what() << "\n";
                        } 
                        catch (...) {
                            std::cerr << "Unknown exception caught \n";
                        }
                        
                        process_entry(_file_info);
                    }

                    sfct_api::rename_entry(m_rename_old,entry.dst);
                    break;
                }
                case file_queue_status::none:
                    // do nothing
                    break;
                default:
                    // do nothing
                    break;
            }
        }

        /// @brief wrapper for adding to the m_still_wait_data queue
        /// handles any eceptions throw within the function
        /// @param entry data for copying, renaming, or removing files
        void add_to_still_wait_data(const file_queue_info& entry) noexcept{
            // this could potentially throw but I think only under exceptional
            // circumstances like system is running out of memory and cannot allocate
            // if it does throw we just skip the entry and the function ends
            try{
                m_still_wait_data.emplace(entry);
            }
            catch(const std::runtime_error& e){
                std::cerr << "Runtime error: " << e.what() << "\n";
            }
            catch(const std::bad_alloc& e){
                std::cerr << "Allocation error: " << e.what() << "\n";
            }
            catch (const std::exception& e) {
                std::cerr << "Standard exception: " << e.what() << "\n";
            } 
            catch (...) {
                std::cerr << "Unknown exception caught \n";
            }
        }
    };
}
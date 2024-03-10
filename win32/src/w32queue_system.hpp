#pragma once
#include "w32cpplib.hpp"
#include "w32obj.hpp"
#include "w32sfct_api.hpp"
#include "w32tm.hpp"

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


        // used to notify the waiting thread
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
            try{
                bool exit__{false};
                for(auto entry{m_new_main_directory_entries.begin()};entry != m_new_main_directory_entries.end() && !exit__;entry++){
                    if(sfct_api::exists(entry->src)){
                        for(const auto& _entry:std::filesystem::recursive_directory_iterator(entry->src)){
                            auto dst_path = sfct_api::create_file_relative_path(_entry.path(),entry->dst,entry->src,false);
                            if(dst_path.has_value()){
                                file_queue_info _file_info;
                                _file_info.co = entry->co;
                                _file_info.commands = entry->commands;
                                _file_info.dst = dst_path.value();
                                _file_info.fqs = file_queue_status::file_added;
                                auto gfs_dst = sfct_api::get_file_status(dst_path.value());
                                auto gfs_src = sfct_api::get_file_status(_entry.path());
                                _file_info.main_dst = entry->main_dst;
                                _file_info.main_src = entry->main_src;
                                _file_info.src = _entry.path();

                                if(gfs_dst.has_value()){
                                    _file_info.fs_dst = gfs_dst.value();
                                }

                                if(gfs_src.has_value()){
                                    _file_info.fs_src = gfs_src.value();
                                }

                                // true if not in the set
                                // false if in the set
                                if(m_all_seen_entries.insert(_file_info).second){
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
            } catch (...) {
                std::cerr << "Unknown exception caught \n";
            }
        }

        // for renaming a path
        std::filesystem::path m_rename_old;
        
        /// @brief 
        /// @param entry file_queue_info object
        void process_entry(const file_queue_info& entry) noexcept{
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::directory:{
                            if(sfct_api::exists(entry.src)){
                                sfct_api::create_directory_paths(entry.dst);
                                
                                try{
                                    if(entry.src.parent_path() == entry.main_src && 
                                        sfct_api::recursive_flag_check(entry.commands) && 
                                        m_all_seen_main_directory_entries.insert(entry).second){
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
                                } catch (...) {
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::block:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::character:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::fifo:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::socket:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::block:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::character:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::fifo:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::socket:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                
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
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";
                                }
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
                            sfct_api::remove_entry(entry.dst);
                            m_all_seen_entries.erase(entry);
                            break;
                        }
                        case std::filesystem::file_type::directory:{
                            if(sfct_api::recursive_flag_check(entry.commands) && sfct_api::exists(entry.dst)){
                                // if an exception is thrown(any exception) m_all_seen_entries gets a reset
                                // this is okay since 
                                try{
                                    for(const auto& rdi_entry_:std::filesystem::recursive_directory_iterator(entry.dst)){
                                        // we need to recreate the src path to produce the correct hash in order to erase it from
                                        // m_all_seen_entries
                                        auto recreated_src = sfct_api::create_file_relative_path(rdi_entry_.path(),entry.src,entry.dst,false);
                                        if(recreated_src.has_value()){
                                            file_queue_info _file_info;
                                            _file_info.src = recreated_src.value();
                                            _file_info.dst = rdi_entry_.path();
                                            m_all_seen_entries.erase(_file_info);
                                        }
                                    }
                                }
                                catch (const std::filesystem::filesystem_error& e) {
                                    std::cerr << "Filesystem error: " << e.what() << "\n";

                                    m_all_seen_entries.clear();
                                }
                                catch(const std::runtime_error& e){
                                    std::cerr << "Runtime error: " << e.what() << "\n";

                                    m_all_seen_entries.clear();
                                }
                                catch(const std::bad_alloc& e){
                                    std::cerr << "Allocation error: " << e.what() << "\n";

                                    m_all_seen_entries.clear();
                                }
                                catch (const std::exception& e) {
                                    std::cerr << "Standard exception: " << e.what() << "\n";

                                    m_all_seen_entries.clear();
                                } catch (...) {
                                    std::cerr << "Unknown exception caught \n";

                                    m_all_seen_entries.clear();
                                }
                            }
                            else{
                                m_all_seen_entries.erase(entry);
                            }


                            sfct_api::remove_all(entry.dst);
                            break;
                        }
                        case std::filesystem::file_type::symlink:{
                            sfct_api::remove_entry(entry.dst);
                            m_all_seen_entries.erase(entry);
                            break;
                        }
                        case std::filesystem::file_type::block:{
                            sfct_api::remove_entry(entry.dst);
                            m_all_seen_entries.erase(entry);
                            break;
                        }
                        case std::filesystem::file_type::character:{
                            sfct_api::remove_entry(entry.dst);
                            m_all_seen_entries.erase(entry);
                            break;
                        }
                        case std::filesystem::file_type::fifo:{
                            sfct_api::remove_entry(entry.dst);
                            m_all_seen_entries.erase(entry);
                            break;
                        }
                        case std::filesystem::file_type::socket:{
                            sfct_api::remove_entry(entry.dst);
                            m_all_seen_entries.erase(entry);
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
                    
                    try{
                        // if this fails not much can be done
                        // other than skip it
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
                    } catch (...) {
                        std::cerr << "Unknown exception caught \n";
                    }
                    break;
                }
                case file_queue_status::rename_new:{
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
                        } catch (...) {
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
    };
}
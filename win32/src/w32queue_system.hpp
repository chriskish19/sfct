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
            while(m_running){

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
        // default constructor
        queue_system() = default;

        // default destructor
        ~queue_system()= default;
        
        // delete the Copy constructor
        queue_system(const queue_system&) = delete;

        // delete Copy assignment operator
        queue_system& operator=(const queue_system&) = delete;

        // delete Move constructor
        queue_system(queue_system&&) = delete;

        // delete Move assignment operator
        queue_system& operator=(queue_system&&) = delete;



        void process() noexcept{
            while(m_running){
            
                try{
                    if(m_ready_to_process.load()){
                        while(!m_queue.empty()){
                            file_queue_info entry = m_queue.front();
                            process_entry(entry);
                            m_queue.pop();
                        }
                        m_ready_to_process = false;
                        m_main_thread_cv.notify_one();
                    }
                    else{

                        if(!m_still_wait_data.empty()){
                            m_wait_data.swap(m_still_wait_data);

                            while(!m_wait_data.empty()){
                                file_queue_info entry = m_wait_data.front();
                                process_entry(entry);
                                m_wait_data.pop();
                            }
                        }
                        
                        check();

                        {   
                            std::unique_lock<std::mutex> local_lock(m_local_thread_guard);
                            m_local_thread_cv.wait(local_lock, [this] {return m_ready_to_process.load();});
                        }
                        
                        std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);
                        m_queue.swap(m_queue_buffer);
                    }
                }
                catch (const std::filesystem::filesystem_error& e) {
                    // Handle filesystem related errors
                    std::cerr << "Filesystem error: " << e.what() << "\n";
                }
                catch(const std::runtime_error& e){
                    // the error message
                    std::cerr << "Runtime error :" << e.what() << "\n";
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

        void add_to_queue(const file_queue_info& entry) noexcept{
            try{
                std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);
                m_queue_buffer.emplace(entry);
            }
            catch(const std::runtime_error& e){
                // the error message
                std::cerr << "Runtime error :" << e.what() << "\n";
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

        void exit() noexcept{
            try{
                // this code is unlikely to throw an exception only under exceptional circumstances
                // but for robustness its wrapped in a try catch block
                // wait to process remaining buffer
                std::unique_lock<std::mutex> local_lock(m_main_thread_guard);
                m_main_thread_cv.wait(local_lock, [this] {return !m_ready_to_process.load(); });

                m_running = false;
            }
            catch(const std::runtime_error& e){
                // the error message
                std::cerr << "Runtime error :" << e.what() << "\n";

                // exit anyway
                m_running = false;
            }
            catch(const std::bad_alloc& e){
                // the error message
                std::cerr << "Allocation error: " << e.what() << "\n";

                // exit anyway
                m_running = false;
            }
            catch (const std::exception& e) {
                // Catch other standard exceptions
                std::cerr << "Standard exception: " << e.what() << "\n";

                // exit anyway
                m_running = false;
            } catch (...) {
                // Catch any other exceptions
                std::cerr << "Unknown exception caught \n";

                // exit anyway
                m_running = false;
            }
        }


        // used to notify the waiting thread
        std::condition_variable m_local_thread_cv;

        std::atomic<bool> m_ready_to_process{false};

        std::atomic<bool> m_running{true};
    private:
        // data ready to be processed
        std::queue<file_queue_info> m_queue; 

        std::queue<file_queue_info> m_queue_buffer;

        // data to be proccessed later
        std::queue<file_queue_info> m_wait_data;

        // data that still has to be processed later
        std::queue<file_queue_info> m_still_wait_data;

        std::mutex m_queue_buffer_mtx;

        std::mutex m_local_thread_guard;
		
        std::mutex m_main_thread_guard;
		std::condition_variable m_main_thread_cv;

        std::vector<file_queue_info> m_new_main_directory_entries;
        std::unordered_set<file_queue_info> m_all_seen_entries,m_all_seen_main_directory_entries;

        // we only check once
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
                // Handle filesystem related errors
                std::cerr << "Filesystem error: " << e.what() << "\n";
            }
            catch(const std::runtime_error& e){
                // the error message
                std::cerr << "Runtime error :" << e.what() << "\n";
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

        // for renaming a path
        std::filesystem::path m_rename_old;
        
        void process_entry(const file_queue_info& entry){
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
                                m_still_wait_data.emplace(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::directory:{
                            if(sfct_api::exists(entry.src)){
                                sfct_api::create_directory_paths(entry.dst);
                                if(entry.src.parent_path() == entry.main_src && sfct_api::recursive_flag_check(entry.commands) && m_all_seen_main_directory_entries.insert(entry).second){
                                    m_new_main_directory_entries.push_back(entry);
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::symlink:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                m_still_wait_data.emplace(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::block:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                m_still_wait_data.emplace(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::character:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                m_still_wait_data.emplace(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::fifo:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                m_still_wait_data.emplace(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::socket:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                m_still_wait_data.emplace(entry);
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
                                m_still_wait_data.emplace(entry);
                            }
                            break;
                        }
                        case std::filesystem::file_type::directory:
                            
                            break;
                        case std::filesystem::file_type::symlink:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                m_still_wait_data.emplace(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::block:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                m_still_wait_data.emplace(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::character:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                m_still_wait_data.emplace(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::fifo:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                m_still_wait_data.emplace(entry);
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::socket:{
                            if(sfct_api::entry_check(entry.src)){
                                sfct_api::copy_entry(entry.src,entry.dst,entry.co);
                            }
                            else{
                                m_still_wait_data.emplace(entry);
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
                            m_all_seen_entries.clear();
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
                    m_rename_old = entry.dst;
                    break;
                }
                case file_queue_status::rename_new:{
                    if(!sfct_api::exists(entry.dst)){
                        file_queue_info _file_info = entry;
                        _file_info.fqs = file_queue_status::file_added;
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
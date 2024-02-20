#pragma once
#include <queue>
#include "obj.hpp"
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <concepts>
#include <type_traits>
#include "sfct_api.hpp"
#include <filesystem>

namespace application{
    template<typename data_t>
    class queue_system{
    public:
        void process(){
            while(m_running){
                if(m_ready_to_process.load()){
                    while(!m_queue.empty()){
                        data_t entry = m_queue.front();
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
                            data_t entry = m_wait_data.front();
                            process_entry(entry);
                            m_wait_data.pop();
                        }
                    }
                    

                    {   
                        std::unique_lock<std::mutex> local_lock(m_local_thread_guard);
                        m_local_thread_cv.wait(local_lock, [this] {return m_ready_to_process.load();});
                    }
                    
                    std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);
                    m_queue.swap(m_queue_buffer);
                }
            }
        }

        void add_to_queue(const data_t& entry){
            std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);
            m_queue_buffer.emplace(entry);
        }

        void exit(){
            // process remaining buffer
            std::unique_lock<std::mutex> local_lock(m_main_thread_guard);
            m_main_thread_cv.wait(local_lock, [this] {return !m_ready_to_process.load(); });

            m_running = false;
        }

        // used to notify the waiting thread
        std::condition_variable m_local_thread_cv;

        std::atomic<bool> m_ready_to_process{false};

        std::atomic<bool> m_running{true};
    private:
        // data ready to be processed
        std::queue<data_t> m_queue; 

        std::queue<data_t> m_queue_buffer;

        // data to be proccessed later
        std::queue<data_t> m_wait_data;

        // data that still has to be processed later
        std::queue<data_t> m_still_wait_data;

        std::mutex m_queue_buffer_mtx;

        std::mutex m_local_thread_guard;
		
        std::mutex m_main_thread_guard;
		std::condition_variable m_main_thread_cv;


        void process_entry(const data_t& entry){
            // add more for other types of data_t 
        }
    };




    template<>
    class queue_system<file_queue_info>{
        public:
        void process(){
            while(m_running){
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
        }

        void add_to_queue(const file_queue_info& entry){
            std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);
            m_queue_buffer.emplace(entry);
        }

        void exit(){
            // process remaining buffer
            std::unique_lock<std::mutex> local_lock(m_main_thread_guard);
            m_main_thread_cv.wait(local_lock, [this] {return !m_ready_to_process.load(); });

            m_running = false;
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
        void check(){
            bool exit__{false};
            for(auto entry{m_new_main_directory_entries.begin()};entry != m_new_main_directory_entries.end() && !exit__;entry++){
                if(std::filesystem::exists(entry->src)){
                    for(const auto& _entry:std::filesystem::recursive_directory_iterator(entry->src)){
                        auto dst_path = sfct_api::create_file_relative_path(_entry.path(),entry->dst,entry->src,false);
                        if(dst_path.has_value()){
                            file_queue_info _file_info;
                            _file_info.co = entry->co;
                            _file_info.commands = entry->commands;
                            _file_info.dst = dst_path.value();
                            _file_info.fqs = file_queue_status::file_added;
                            _file_info.fs_dst = std::filesystem::status(dst_path.value());
                            _file_info.fs_src = std::filesystem::status(_entry.path());
                            _file_info.main_dst = entry->main_dst;
                            _file_info.main_src = entry->main_src;
                            _file_info.src = _entry.path();

                            // true if not in the set
                            // false if in the set
                            if(m_all_seen_entries.insert(_file_info).second){
                                process_entry(_file_info);
                            }
                            else{
                                // lets not check everything
                                // if one entry exists most likley they all do
                                // but may change this in the future for more robust checking
                                exit__ = true;
                            }
                        }
                    }
                }
                
                
            }

            m_new_main_directory_entries.clear();
        }

    public:
        void process_entry(const file_queue_info& entry){
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
                            if(std::filesystem::exists(entry.src)){
                                sfct_api::create_directory_paths(entry.dst);
                                if(entry.src.parent_path() == entry.main_src && sfct_api::recursive_flag_check(entry.commands) && m_all_seen_main_directory_entries.insert(entry).second){
                                    m_new_main_directory_entries.push_back(entry);
                                }
                            }
                            
                            break;
                        }
                        case std::filesystem::file_type::symlink:
                            sfct_api::copy_symlink(entry.src,entry.dst,entry.co);
                            break;
                        case std::filesystem::file_type::block:
                            
                            break;
                        case std::filesystem::file_type::character:
                            
                            break;
                        case std::filesystem::file_type::fifo:
                            
                            break;
                        case std::filesystem::file_type::socket:
                            
                            break;
                        case std::filesystem::file_type::unknown:

                            break;
                        default:
                            
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
                        case std::filesystem::file_type::symlink:
                            sfct_api::copy_symlink(entry.src,entry.dst,entry.co);
                            break;
                        case std::filesystem::file_type::block:
                            
                            break;
                        case std::filesystem::file_type::character:
                            
                            break;
                        case std::filesystem::file_type::fifo:
                            
                            break;
                        case std::filesystem::file_type::socket:
                            
                            break;
                        case std::filesystem::file_type::unknown:

                            break;
                        default:
                            
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
                        case std::filesystem::file_type::symlink:
                            sfct_api::remove_entry(entry.dst);
                            break;
                        case std::filesystem::file_type::block:
                            
                            break;
                        case std::filesystem::file_type::character:
                            
                            break;
                        case std::filesystem::file_type::fifo:
                            
                            break;
                        case std::filesystem::file_type::socket:
                            
                            break;
                        case std::filesystem::file_type::unknown:

                            break;
                        default:
                            
                            break;
                    }
                    break;
                }
                case file_queue_status::none:
                    break;
                default:
                    break;
            }
        }
    };
}
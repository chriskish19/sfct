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

namespace application{
    template<typename data_t>
    class queue_system{
    public:

        queue_system(){

        }


        void process(){
            while(m_running){
                if(m_ready_to_process){
                    while(!m_queue.empty()){
                        data_t entry = m_queue.front();
                        process_entry(entry);
                        m_queue.pop();
                    }
                    m_ready_to_process = false;
                    m_main_thread_cv.notify_one();
                }
                else{
                    m_local_thread_lock = std::unique_lock<std::mutex>(m_local_thread_guard);
                    m_local_thread_cv.wait(m_local_thread_lock, [this] {return m_go;});
                    
                    std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);
                    std::queue<data_t> empty_queue;
                    m_ready_to_process = true;
                    m_queue.swap(m_queue_buffer);
                    m_queue_buffer.swap(empty_queue);
                }
            }
        }

        void add_to_queue(const data_t& entry){
            std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);
            m_queue_buffer.emplace(entry);
        }

        void exit(){
            // process remaining buffer
            m_main_thread_lock = std::unique_lock<std::mutex>(m_main_thread_guard);
            m_main_thread_cv.wait(m_main_thread_lock, [this] {return !m_ready_to_process; });

            m_running = false;
        }

        // used to notify the waiting thread
        std::condition_variable m_local_thread_cv;
    private:
        // data ready to be processed
        std::queue<data_t> m_queue; 

        std::queue<data_t> m_queue_buffer;

        std::mutex m_queue_buffer_mtx;

        std::atomic<bool> m_ready_to_process{false},m_running{true},m_go{true};


        std::chrono::steady_clock::time_point m_start,m_end;
        std::chrono::duration<double_t> m_duration;

        std::mutex m_local_thread_guard;
		std::unique_lock<std::mutex> m_local_thread_lock;
        
        std::mutex m_main_thread_guard;
		std::unique_lock<std::mutex> m_main_thread_lock;
		std::condition_variable m_main_thread_cv;


        void process_entry(const data_t& entry){
            if constexpr (std::is_same_v<data_t, file_queue_info>) {
                process_file_queue_info(entry);
            }

            // add more for other types of data_t 
        }

        void process_file_queue_info(const file_queue_info& entry){
            switch(entry.fqs){
                case file_queue_status::file_added:{
                    switch(entry.fs_src.type()){
                        case std::filesystem::file_type::none:
                            // skip for now
                            break;
                        case std::filesystem::file_type::not_found:
                            // skip for now
                            break;
                        case std::filesystem::file_type::regular:
                            sfct_api::copy_file_create_path(entry.src,entry.dst,entry.co);
                            break;
                        case std::filesystem::file_type::directory:
                            sfct_api::create_directory_paths(entry.dst);
                            break;
                        case std::filesystem::file_type::symlink:
                            
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
                        case std::filesystem::file_type::regular:
                            sfct_api::copy_file_create_path(entry.src,entry.dst,entry.co);
                            break;
                        case std::filesystem::file_type::directory:
                            
                            break;
                        case std::filesystem::file_type::symlink:
                            
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
                        case std::filesystem::file_type::regular:
                            sfct_api::remove_file(entry.dst);
                            break;
                        case std::filesystem::file_type::directory:
                            sfct_api::remove_all(entry.dst);
                            break;
                        case std::filesystem::file_type::symlink:
                            
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
            }
        }
    };
}
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
            if(m_ready_to_process){
                while(!m_queue.empty()){
                    data_t entry = m_queue.front();
                    process_entry(entry);
                    m_queue.pop();
                }
                m_ready_to_process = false;
            }
            else{
                std::this_thread::sleep_for(std::chrono::seconds(30));
                std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);
                m_end = std::chrono::high_resolution_clock::now();
                m_duration = m_end - m_start;
                if(duration.count() > 30.0){
                    std::queue<data_t> empty_queue;
                    m_ready_to_process = true;
                    m_queue.swap(m_queue_buffer);
                    m_queue_buffer.swap(empty_queue);
                }
            }
            
        }

        void add_to_queue(data_t entry){
            std::lock_guard<std::mutex> local_lock(m_queue_buffer_mtx);
            m_queue_buffer.emplace(entry);
            m_start = std::chrono::high_resolution_clock::now();
        }
    private:
        // data ready to be processed
        std::queue<data_t> m_queue; 

        std::queue<data_t> m_queue_buffer;

        std::mutex m_queue_buffer_mtx;

        std::atomic<bool> m_ready_to_process{false};


        std::chrono::steady_clock::time_point m_start,m_end;
        std::chrono::duration<double_t> m_duration;

        
        void process_entry(const data_t& entry){
            if constexpr (std::is_same_v<data_t, file_queue_info>) {
                process_file_queue_info(entry);
            }

            // add more for other types of data_t 
        }

        void process_file_queue_info(const file_queue_info& entry){
            switch(entry.fqs){
                case file_queue_status::file_added:{
                    sfct_api::copy_file_create_path(entry.src,entry.dst,entry.co);
                    break;
                }
                case file_queue_status::file_updated:{
                    
                    break;
                }
                case file_queue_status::file_removed:{
                    
                    break;
                }
            }
        }
    };
}
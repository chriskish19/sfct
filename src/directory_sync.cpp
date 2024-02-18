#include "directory_sync.hpp"

void application::directory_sync(const std::vector<application::copyto> &dirs, std::atomic<bool> *procceed, std::condition_variable *local_thread_cv, std::atomic<bool> *running)
{
    
    queue_system<file_queue_info> qs;

    std::thread q_sys_thread([&qs](){
        exceptions(
            &application::queue_system<application::file_queue_info>::process, 
            &qs
        );
        });
    
    while(running->load()){
        std::mutex local_mtx;
        std::unique_lock<std::mutex> local_lock(local_mtx);
        local_thread_cv->wait(local_lock, [procceed] {return procceed->load();});

        for(const auto& dir:dirs){

            auto missing = sfct_api::are_directories_synced(dir.source,dir.destination,sfct_api::recursive_flag_check(dir.commands));
            if(missing.has_value()){
                

                for(const auto& pair:*missing.value()){
                    file_queue_info _file_info;
                    _file_info.co = dir.co;
                    _file_info.dst = pair.first;
                    _file_info.fqs = file_queue_status::file_added;
                    _file_info.fs_dst = std::filesystem::status(pair.first);
                    _file_info.src = pair.second;
                    _file_info.fs_src = std::filesystem::status(pair.second);

                    qs.add_to_queue(_file_info);
                }

            }

        }

        qs.m_ready_to_process = true;
        qs.m_local_thread_cv.notify_one();

        procceed->store(false);
    }

    qs.exit();
    if(q_sys_thread.joinable()){
        q_sys_thread.detach();
    }
}